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

#include <list>

#include <boost/foreach.hpp>

#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/playbackTimeRange.h>
#include <libwatcher/streamDescriptionMessage.h>
#include <libwatcher/streamDescriptionMessage.h>
#include <libwatcher/startWatcherMessage.h>
#include <libwatcher/stopWatcherMessage.h>

#include "sharedStream.h"
#include "replayState.h"
#include "watcherd.h"
#include "database.h"

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

class SharedStreamImpl {
    public:
	Watcherd& watcher_;
	uint32_t uid_;
	boost::shared_ptr<ReplayState> replay_;
	std::string description_;

	boost::mutex lock_;
	std::list<ServerConnectionPtr> clients_; // clients subscribed to this stream

	SharedStreamImpl(Watcherd& wd) : watcher_(wd), uid_(getNextUID()) {}
};

using namespace watcher::event;

uint32_t SharedStream::getUID() const
{
    return impl_->uid_;
}

SharedStream::SharedStream(Watcherd& wd) : isPlaying_(false), isLive_(true), impl_(new SharedStreamImpl(wd))
{
    TRACE_ENTER();
    TRACE_EXIT();
}

SharedStream::~SharedStream()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void SharedStream::seek(const SeekMessagePtr& p)
{
    TRACE_ENTER();
    if (p->offset == SeekMessage::eof) {
	if (isLive_) {
	    // nothing to do
	} else if (impl_->replay_->speed() >= 0) {
	    // switch to live stream
	    isLive_ = true;
	    impl_->replay_->pause();
	    if (isPlaying_)
		impl_->watcher_.subscribe(shared_from_this());
	} else {
	    impl_->replay_->seek( SeekMessage::eof );
	}
    } else {
	impl_->replay_->seek(p->offset);

	// when switching from live to replay while playing, kick off the replay strand
	if (isLive_ && isPlaying_)
	    impl_->replay_->run();
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
	    impl_->watcher_.subscribe(shared_from_this());
	else
	    impl_->replay_->run();
    }
    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    sendMessage(StartMessagePtr(new StartMessage())); // relay to all subscribers
    TRACE_EXIT();
}

void SharedStream::stop()
{
    TRACE_ENTER();
    LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    if (isPlaying_) {
	LOG_DEBUG("stopping playback");
	if (isLive_) {
	    impl_->watcher_.unsubscribe(shared_from_this());
	    /* save current Timestamp so we can resume from the database */
	    isLive_ = false;
	    impl_->replay_->seek( getCurrentTime() );
	} else
	    impl_->replay_->pause();
	isPlaying_ = false;
    } else
	LOG_DEBUG("stop message received, but playback is stopped");
    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    sendMessage(StopMessagePtr(new StopMessage())); // relay to all subscribers
    TRACE_EXIT();
}

void SharedStream::speed(const SpeedMessagePtr& p)
{
    TRACE_ENTER();
    LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    bool resend = true;

    if (p->speed == 0) {
	/* special case, speed==0 means StopMessage.  This is to avoid
	 * a Bad_arg exception from ReplayState. */
	LOG_INFO("got speed==0, emulating StopMessage");
	stop();
    } else if (isLive_ && p->speed >= 1.0f) {
	// ignore, can't predict the future
	resend = false;
    } else {
	impl_->replay_->speed(p->speed);
	if (isLive_) {
	    isLive_ = false;
	    /* when transitioning from live playback, automatically
	     * seek to the end of the database.
	     */
	    impl_->replay_->seek( SeekMessage::eof );
	    if (isPlaying_)
		impl_->replay_->run();
	}
    }

    /* Resend this message to all clients watching this stream. */
    if (resend)
	sendMessage(p);

    LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
    TRACE_EXIT();
}

/** Returns a PlaybackTimeRangeMessage event to the sender with the timestamps of
 * the first and last event in the database.
 */
void SharedStream::range(ServerConnectionPtr conn)
{
    TRACE_ENTER();

    TimeRange r(event_range());
    PlaybackTimeRangeMessagePtr p(new PlaybackTimeRangeMessage(r.first, r.second));
    conn->sendMessage(p);

    TRACE_EXIT();
}

/** send a message to all clients subscribed to this shared stream. */
void SharedStream::sendMessage(MessagePtr m)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(impl_->lock_);

    int count = 0;
    BOOST_FOREACH(ServerConnectionPtr conn, impl_->clients_) {
	conn->sendMessage(m);
	++count;
    }

    LOG_DEBUG("sent message to " << count << " clients for stream uid " << impl_->uid_);

    TRACE_EXIT();
}

void SharedStream::sendMessage(const std::vector<MessagePtr>& msgs)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(impl_->lock_);

    int count = 0;
    BOOST_FOREACH(ServerConnectionPtr conn, impl_->clients_) {
	conn->sendMessage(msgs);
	++count;
    }
    LOG_DEBUG("sent messages to " << count << " clients for stream uid " << impl_->uid_);

    TRACE_EXIT();
}

void SharedStream::subscribe(ServerConnectionPtr p)
{
    TRACE_ENTER();

    boost::mutex::scoped_lock lck(impl_->lock_);

    // shared_from_this() doesn't work in a ctor, so delay creation of the ReplayState until a client subscribes
    // FIXME: this assumes that all clients are using the same io_service.
    if (! impl_->replay_.get())
	impl_->replay_.reset(new ReplayState(p->io_service(), shared_from_this()));

    impl_->clients_.push_front(p);

    // send the current state to the new subscribe
    {
	SpeedMessagePtr msg(new SpeedMessage(impl_->replay_->speed()));
	p->sendMessage(msg);
    }
    {
	StreamDescriptionMessagePtr msg(new StreamDescriptionMessage(impl_->description_));
	p->sendMessage(msg);
    }

    TRACE_EXIT();
}

void SharedStream::unsubscribe(ServerConnectionPtr p)
{
    TRACE_ENTER();
    boost::mutex::scoped_lock lck(impl_->lock_);
    impl_->clients_.remove(p);

    if (impl_->clients_.empty()) {
	LOG_INFO("no more waiting clients for for stream uid=" << impl_->uid_);
	impl_->watcher_.removeStream(shared_from_this());
    }
    TRACE_EXIT();
}

void SharedStream::setDescription(StreamDescriptionMessagePtr p)
{
    TRACE_ENTER();
    impl_->description_ = p->desc;
    sendMessage(p); // resend to all clients
    TRACE_EXIT();
}

std::string SharedStream::getDescription() const
{
    return impl_->description_;
}

} // namespace
