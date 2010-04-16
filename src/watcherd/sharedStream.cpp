/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

#include "sharedStream.h"
#include "replayState.h"
#include "watcherd.h"
#include "database.h"

#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/playbackTimeRange.h>

#include <boost/weak_ptr.hpp>
#include <boost/foreach.hpp>

#include <list>

namespace {

uint32_t getNextUID()
{
    static uint32_t UID = 0;
    static boost::mutex uidlock;

    boost::mutex::scoped_lock l(uidlock);
    ++UID;
    return UID;
}

} // namespace

namespace watcher {

INIT_LOGGER(SharedStream, "SharedStream");

using namespace watcher::event;

void SharedStream::init(boost::asio::io_service& ios)
{
    isPlaying_=false;
    isLive_=true;
    uid=getNextUID();
    replay.reset(new ReplayState(ios, shared_from_this()));
}

SharedStream::SharedStream(Watcherd& wd, boost::asio::io_service& ios_) : watcher(wd)
{
    TRACE_ENTER();
    init(ios_);
    TRACE_EXIT();
}

SharedStream::SharedStream(Watcherd& wd, boost::asio::io_service& ios_, ServerConnectionPtr ptr) : watcher(wd)
{
    TRACE_ENTER();
    init(ios_);
    subscribe(ptr);
    TRACE_EXIT();
}

void SharedStream::seek(const SeekMessagePtr& p)
{
    TRACE_ENTER();
    if (p->offset == SeekMessage::eof) {
	if (isLive_) {
	    // nothing to do
	} else if (replay->speed() >= 0) {
	    // switch to live stream
	    isLive_ = true;
	    replay->pause();
	    if (isPlaying_)
		watcher.subscribe(shared_from_this());
	} else {
	    replay->seek( SeekMessage::eof );
	}
    } else {
	replay->seek(p->offset);

	// when switching from live to replay while playing, kick off the replay strand
	if (isLive_ && isPlaying_)
	    replay->run();
	isLive_ = false;
    }
    TRACE_EXIT();
}

void SharedStream::start()
{
    TRACE_ENTER();
    LOG_DEBUG("in: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    if (!isPlaying_) {
	isPlaying_ = true;
	if (isLive_)
	    watcher.subscribe(shared_from_this());
	else
	    replay->run();
    }
    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

void SharedStream::stop()
{
    TRACE_ENTER();
    LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    if (isPlaying_) {
	LOG_DEBUG("stopping playback");
	if (isLive_) {
	    watcher.unsubscribe(shared_from_this());
	    /* save current Timestamp so we can resume from the database */
	    isLive_ = false;
	    replay->seek( getCurrentTime() );
	} else
	    replay->pause();
	isPlaying_ = false;
    } else
	LOG_DEBUG("stop message received, but playback is stopped");
    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

void SharedStream::speed(const SpeedMessagePtr& p)
{
    TRACE_ENTER();
    LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);

    if (p->speed == 0) {
	/* special case, speed==0 means StopMessage.  This is to avoid
	 * a Bad_arg exception from ReplayState. */
	LOG_INFO("got speed==0, emulating StopMessage");
	stop();
    } else if (isLive_ && p->speed >= 1.0f) {
	// ignore, can't predict the future
    } else {
	replay->speed(p->speed);
	if (isLive_) {
	    isLive_ = false;
	    /* when transitioning from live playback, automatically
	     * seek to the end of the database.
	     */
	    replay->seek( SeekMessage::eof );
	    if (isPlaying_)
		replay->run();
	}
    }

    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

/** Returns a PlaybackTimeRangeMessage event to the sender with the timestamps of
 * the first and last event in the database.
 */
void SharedStream::range(PlaybackTimeRangeMessagePtr p)
{
    TRACE_ENTER();

    TimeRange r(event_range());
    p->min_ = r.first;
    p->max_ = r.second;
    sendMessage(p);

    TRACE_EXIT();
}

/** send a message to all clients subscribed to this shared stream. */
void SharedStream::sendMessage(MessagePtr m)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(lock);

    /* weak_ptr doesn't have an operator== so we can't keep a dead list. */
    std::list<ServerConnectionWeakPtr> alive;

    BOOST_FOREACH(ServerConnectionWeakPtr ptr, clients) {
	ServerConnectionPtr conn = ptr.lock();
	if (conn) {
	    conn->sendMessage(m);
	    alive.push_front(ptr);
	}
    }

    if (alive.empty()) {
	LOG_INFO("no more waiting clients");
	watcher.unsubscribe(shared_from_this());
    } else
	clients = alive;

    TRACE_EXIT();
}

void SharedStream::sendMessage(const std::vector<MessagePtr>& msgs)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(lock);

    /* weak_ptr doesn't have an operator== so we can't keep a dead list. */
    std::list<ServerConnectionWeakPtr> alive;

    BOOST_FOREACH(ServerConnectionWeakPtr ptr, clients) {
	ServerConnectionPtr conn = ptr.lock();
	if (conn) {
	    conn->sendMessage(msgs);
	    alive.push_front(ptr);
	}
    }

    if (alive.empty()) {
	LOG_INFO("no more waiting clients");
	watcher.unsubscribe(shared_from_this());
    } else
	clients = alive;

    TRACE_EXIT();
}

void SharedStream::subscribe(ServerConnectionPtr p)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(lock);
    clients.push_front(p);
    TRACE_EXIT();
}

/* list.remove() requires operator== which weak_ptr does not have */
template <typename T>
void weak_ptr_remove(std::list<boost::weak_ptr<T> > l, boost::weak_ptr<T> v)
{
    for (typename std::list<boost::weak_ptr<T> >::iterator it = l.begin(); it != l.end();)
    {
	if ( !(*it < v) && !(v < *it) )
	    it = l.erase(it);
	else
	    ++it;
    }
}
void SharedStream::unsubscribe(ServerConnectionPtr p)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(lock);
    weak_ptr_remove(clients, ServerConnectionWeakPtr(p));
    if (clients.empty()) {
	LOG_INFO("no more waiting clients");
	watcher.unsubscribe(shared_from_this());
    }
    TRACE_EXIT();
}

} // namespace
