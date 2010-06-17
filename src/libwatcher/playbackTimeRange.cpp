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

/** @file
 * @date 2009-06-24
 */
#include "watcherSerialize.h"
#include "playbackTimeRange.h"
#include "logger.h"

namespace watcher {

namespace event {

INIT_LOGGER(PlaybackTimeRangeMessage, "Message.PlaybackTimeRangeMessage");

PlaybackTimeRangeMessage::PlaybackTimeRangeMessage(Timestamp tmin, Timestamp tmax)
	: Message(PLAYBACK_TIME_RANGE_MESSAGE_TYPE, PLAYBACK_TIME_RANGE_MESSAGE_VERSION),
	min_(tmin), max_(tmax)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

std::ostream& operator<< (std::ostream& os, const PlaybackTimeRangeMessage& m)
{
    return os << "PlaybackTimeRangeMessage(" << m.min_ << ',' << m.max_ << ')';
}

bool operator== (const PlaybackTimeRangeMessage& lhs , const PlaybackTimeRangeMessage& rhs)
{
    return lhs.min_ == rhs.min_ && lhs.max_ == rhs.max_;
};

template <typename Archive>
void PlaybackTimeRangeMessage::serialize(Archive & ar, const unsigned int /* version */)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & min_;
    ar & max_;
    TRACE_EXIT();
}

} // namespace

} // namespace

BOOST_CLASS_EXPORT(watcher::event::PlaybackTimeRangeMessage);
