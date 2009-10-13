/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "replayState.h"

#include <deque>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "libwatcher/message.h"
#include "serverConnection.h"
#include "Assert.h"
#include "database.h"
#include "watcherd.h"
#include "logger.h"

using namespace util;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(ReplayState, "ReplayState"); 

//< default value for number of events to prefetch from the database
const unsigned int DEFAULT_BUFFER_SIZE = 20U; /* db rows */
const unsigned int DEFAULT_STEP = 250U /* ms */;

/** Internal structure used for implementing the class.  Used to avoid
 * dependencies for the user of the class.  These would normally be private
 * members of ReplayState.
 */
struct ReplayState::impl {
    boost::weak_ptr<ServerConnection> conn;
    std::deque<MessagePtr> events;
    boost::asio::deadline_timer timer;
    Timestamp ts; // the current effective time
    Timestamp last_event; // timestamp of last event retrieved from db
    float speed; //< playback speed
    unsigned int bufsiz; //< number of database rows to prefetch
    Timestamp step;
    enum run_state { paused, running } state;
    timeval wall_time; //< used to correct for clock skew
    Timestamp delta;

    /*
     * Lock used for event queue.  This is required due to the seek() member
     * function, which can be called from a different thread.
     */
    boost::mutex lock;

    impl(ServerConnectionPtr& ptr) :
        conn(ptr), timer(ptr->io_service()), ts(0), last_event(0), bufsiz(DEFAULT_BUFFER_SIZE),
        step(DEFAULT_STEP), state(paused), delta(0)
    {
        TRACE_ENTER();
        wall_time.tv_sec = 0;
        wall_time.tv_usec = 0;
        TRACE_EXIT();
    }
};

ReplayState::ReplayState(ServerConnectionPtr ptr, Timestamp t, float playback_speed) :
    impl_(new impl(ptr))
{
    TRACE_ENTER();
    Assert<Bad_arg>(t >= 0);
    impl_->ts = t;
    impl_->last_event = t;

    speed(playback_speed);
    TRACE_EXIT();
}

Timestamp ReplayState::tell() const
{
    TRACE_ENTER();
    TRACE_EXIT_RET(impl_->ts);
    return impl_->ts;
}

ReplayState& ReplayState::pause()
{
    TRACE_ENTER();
    LOG_DEBUG("cancelling timer");
    impl_->timer.cancel();
    impl_->state = impl::paused;
    TRACE_EXIT();
    return *this;
}

ReplayState& ReplayState::seek(Timestamp t)
{
    TRACE_ENTER();

    impl::run_state oldstate;
    {
        boost::mutex::scoped_lock L(impl_->lock);
        oldstate = impl_->state;
        pause();
        impl_->events.clear();
        impl_->ts = t;
        if (t == -1)
            impl_->last_event = std::numeric_limits<Timestamp>::max();
        else
            impl_->last_event = t;
    }
    if (oldstate == impl::running)
        run();
    TRACE_EXIT();
    return *this;
}

ReplayState& ReplayState::speed(float f)
{
    TRACE_ENTER();
    Assert<Bad_arg>(f != 0);
    /* If speed changes direction, need to clear the event list.
     * Check for sign change by noting that positive*negative==negative
     */
    if (impl_->speed * f < 0) {
        impl::run_state oldstate;
        {
            boost::mutex::scoped_lock L(impl_->lock);

            oldstate = impl_->state;
            pause();

            impl_->events.clear();

            /*
             * Avoid setting .last_event when SpeedMessage is received
             * prior to the first StartMessage.
             */
            if (impl_->ts != 0 && impl_->ts != -1)
                impl_->last_event = impl_->ts;

            impl_->speed = f;
        }
        if (oldstate == impl::running)
            run();
    } else
        impl_->speed = f;
    LOG_DEBUG("ts=" << impl_->ts << " last_event=" << impl_->last_event);
    TRACE_EXIT();
    return *this;
}

ReplayState& ReplayState::buffer_size(unsigned int n)
{
    TRACE_ENTER();
    Assert<Bad_arg>(n != 0);
    impl_->bufsiz = n;
    TRACE_EXIT();
    return *this;
}

ReplayState& ReplayState::time_step(unsigned int n)
{
    TRACE_ENTER();
    Assert<Bad_arg>(n != 0);
    impl_->step = n;
    TRACE_EXIT();
    return *this;
}

namespace {
    /* function object for accepting events output from Database::getEvents() */
    struct event_output {
        std::deque<MessagePtr>& q;
        event_output(std::deque<MessagePtr>& qq) : q(qq) {}
        void operator() (MessagePtr m) { q.push_back(m); }
    };
}

/** Schedule an asynchronous task to replay events from the database to a GUI
 * client.  If the local cache of upcoming events is empty, prefetch a block of
 * events from the database.
 *
 * The code is written such that it will work when playing events forward or in
 * reverse.
 */
void ReplayState::run()
{
    TRACE_ENTER();
    boost::mutex::scoped_lock L(impl_->lock);

    if (impl_->events.empty()) {
        // queue is empty, pre-fetch more items from the DB

        boost::function<void(MessagePtr)> cb(event_output(impl_->events));
        LOG_DEBUG("fetching events " << (impl_->speed > 0 ? "> " : "< ") << impl_->last_event);
        get_db_handle().getEvents(cb,
                                  impl_->last_event,
                                  (impl_->speed >= 0) ? Database::forward : Database::reverse,
                                  impl_->bufsiz);

        if (!impl_->events.empty()) {
            LOG_DEBUG("got " << impl_->events.size() << " events from the db query");
            /* When starting to replay, assume that time T=0 is the time of the
             * first event in the stream.
             * T= -1 is EOF.
             * Convert to timestamp of first item in the returned events.
             *
             * When playing in reverse, the first item in the list is the last event in the database.
             */
            if (impl_->ts == 0 || impl_->ts == -1)
                impl_->ts = impl_->events.front()->timestamp;

            // save timestamp of last event retrieved to avoid duplication
            impl_->last_event = impl_->events.back()->timestamp;
        }
    }

    if (! impl_->events.empty()) {
        /*
         * Calculate for the skew introduced by the time required to process the events.  Skew is calculated
         * as the difference between the actual time taken and the expected time.  This gets 
         * subtracted from the wait for the next event to catch up.
         */
        timeval tv;
        gettimeofday(&tv, 0);
        Timestamp skew = (tv.tv_sec - impl_->wall_time.tv_sec) * 1000 + (tv.tv_usec - impl_->wall_time.tv_usec) / 1000;
        LOG_DEBUG("skew=" << skew << " delta=" << impl_->delta);
        skew -= impl_->delta;
        LOG_DEBUG("calculated skew of " << skew << " ms");
        if (skew < 0) {
            LOG_DEBUG("something strange happened, skew < delta ??");
            skew = 0;
        }
        memcpy(&impl_->wall_time, &tv, sizeof(tv));

        // time until next event
        impl_->delta = impl_->events.front()->timestamp - impl_->ts;

        // update our notion of the current time after the timer expires
        impl_->ts = impl_->events.front()->timestamp;

        /* Adjust for playback speed.  Note that when playing events in reverse, both speed
         * delta will be negative, which will turn delta into a positive value for the
         * async_wait() call, which is exactly what is required.  */
        impl_->delta = (Timestamp)(impl_->delta/impl_->speed);

        /* Correct for skew */
        impl_->delta -= skew;
        if (impl_->delta < 0)
            impl_->delta = 0;

        LOG_DEBUG("Next event in " << impl_->delta << " ms");
        impl_->timer.expires_from_now(boost::posix_time::millisec(impl_->delta));
        impl_->timer.async_wait(boost::bind(&ReplayState::timer_handler, shared_from_this(), boost::asio::placeholders::error));
        impl_->state = impl::running;
    } else {
        /*
         * FIXME what should happen when the end of the event stream is reached?
         * One option would be to convert to live stream at this point.
         */
        LOG_DEBUG("reached end of database, pausing playback");
        impl_->state = impl::paused;
    }
    TRACE_EXIT();
}

/** Replay events to a GUI client when a timer expires.
 *
 * The run() member function is reponsible for prefetching events from the
 * database and storing them in the class object.  When a timer expires, run
 * through the locally stored events and send those that occurred within the
 * last time slice.  The task is then rescheduled when the next most recent
 * event needs to be transmitted.
 */
void ReplayState::timer_handler(const boost::system::error_code& ec)
{
    TRACE_ENTER();
    if (ec == boost::asio::error::operation_aborted)
        LOG_DEBUG("timer was cancelled");
    else if (impl_->state == impl::paused) {
        LOG_DEBUG("timer expired but state is paused!");
    } else {
        std::vector<MessagePtr> msgs;
        {
            boost::mutex::scoped_lock L(impl_->lock);

            while (! impl_->events.empty()) {
                MessagePtr m = impl_->events.front();
                /* Replay all events in the current time step.  Use the absolute value
                 * of the difference in order for forward and reverse replay to work
                 * properly. */
                if (abs(m->timestamp - impl_->ts) >= impl_->step)
                    break;
                msgs.push_back(m);
                impl_->events.pop_front();
            }
        }

        ServerConnectionPtr srv = impl_->conn.lock();
        if (srv) { /* connection is still alive */
            srv->sendMessage(msgs);
            run(); // reschedule this task
        }
    }
    TRACE_EXIT();
}

/* This is required to be defined, otherwise a the default dtor will cause a
 * compiler error due to use of scoped_ptr with an incomplete type.
 */
ReplayState::~ReplayState()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

float ReplayState::speed() const
{
    return impl_->speed;
}
