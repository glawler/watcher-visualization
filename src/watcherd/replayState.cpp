#include "replayState.h"

#include <queue>
#include <boost/bind.hpp>

#include "libwatcher/message.h"
#include "serverConnection.h"
#include "Assert.h"

using namespace util;
using namespace watcher;
using namespace watcher::event;

//< default value for number of events to prefetch from the database
const unsigned int DEFAULT_BUFFER_SIZE = 10U; /* db rows */
const unsigned int DEFAULT_STEP = 250U /* ms */;

/** Internal structure used for implementing the class.  Used to avoid
 * dependencies for the user of the class.  These would normally be private
 * members of ReplayState.
 */
struct ReplayState::impl {
    boost::weak_ptr<ServerConnection> conn;
    std::queue<MessagePtr> events;
    boost::asio::deadline_timer timer;
    Timestamp ts; // the current effective time
    float speed; //< playback speed
    unsigned int bufsiz; //< number of database rows to prefetch
    Timestamp step;

    impl(ServerConnectionPtr& ptr) :
        conn(ptr), timer(ptr->io_service()), ts(0), bufsiz(DEFAULT_BUFFER_SIZE),
        step(DEFAULT_STEP) {}
};

ReplayState::ReplayState(ServerConnectionPtr& ptr, float playback_speed) :
    impl_(new impl(ptr))
{
    speed(playback_speed);
}

ReplayState& ReplayState::speed(float f)
{
    Assert<Bad_speed>(f != 0);
    impl_->speed = f;
    return *this;
}

ReplayState& ReplayState::buffer_size(unsigned int n)
{
    Assert<Bad_buffer_size>(n != 0);
    impl_->bufsiz = n;
    return *this;
}

ReplayState& ReplayState::time_step(unsigned int n)
{
    Assert<Bad_step>(n != 0);
    impl_->step = n;
    return *this;
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
    if (impl_->events.empty()) {
        /* queue is empty, pre-fetch more items from the DB */
        //FIXME need to implement this!

        /* When starting to replay, assume that time T=0 is the time of the
         * first event in the stream. */
        if (impl_->ts == 0)
            impl_->ts = impl_->events.front()->timestamp;
    }

    if (! impl_->events.empty()) {
        /* time until next event */
        Timestamp delta = impl_->events.front()->timestamp - impl_->ts;

        // update our notion of the current time after the timer expires
        impl_->ts += delta;

        /* Adjust for playback speed.  Note that when playing events in reverse, both speed
         * delta will be negative, which will turn delta into a positive value for the
         * async_wait() call, which is exactly what is required.  */
        delta /= impl_->speed;

        impl_->timer.expires_from_now(boost::posix_time::millisec(delta));
        impl_->timer.async_wait(boost::bind(&ReplayState::timer_handler, shared_from_this(), boost::asio::placeholders::error));
    }
    /*
     * FIXME what should happen when the end of the event stream is reached?
     * Currently the object is destroyed when this function exits because no
     * other shared pointers will exist.
     */
}

/** Replay events to a GUI client when a timer expires.
 *
 * The run() member function is reponsible for prefetching events from the
 * database and storing them in the class object.  When a timer expires, run
 * through the locally stored events and send those that occurred within the
 * last time slice.  The task is then rescheduled when the next most recent
 * event needs to be transmitted.
 */
void ReplayState::timer_handler(const boost::system::error_code&)
{
    std::vector<MessagePtr> msgs;

    while (! impl_->events.empty()) {
        MessagePtr m = impl_->events.front();
        /* Replay all events in the current time step.  Use the absolute value
         * of the difference in order for forward and reverse replay to work
         * properly. */
        if (abs(m->timestamp - impl_->ts) >= impl_->step)
            break;
        msgs.push_back(m);
        impl_->events.pop();
    }

    ServerConnectionPtr srv = impl_->conn.lock();
    if (srv) { /* connection is still alive */
        srv->sendMessage(msgs);
        run(); // reschedule this task
    }
}
