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
#include "playbackTimeRange.h"
#include "logger.h"

namespace watcher {

namespace event {

INIT_LOGGER(PlaybackTimeRangeMessage, "Message.PlaybackTimeRangeMessage");

PlaybackTimeRangeMessage::PlaybackTimeRangeMessage(Timestamp tmin, Timestamp tmax, Timestamp tcur)
	: Message(PLAYBACK_TIME_RANGE_MESSAGE_TYPE, PLAYBACK_TIME_RANGE_MESSAGE_VERSION),
	min_(tmin), max_(tmax), cur_(tcur)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

std::ostream& operator<< (std::ostream& os, const PlaybackTimeRangeMessage& m)
{
    return os << "PlaybackTimeRangeMessage(" << m.min_ << ',' << m.max_ << ',' << m.cur_ << ')'; 
}

bool operator== (const PlaybackTimeRangeMessage& lhs , const PlaybackTimeRangeMessage& rhs)
{
    return lhs.min_ == rhs.min_ && lhs.max_ == rhs.max_ && lhs.cur_ == rhs.cur_; 
};


YAML::Emitter &PlaybackTimeRangeMessage::serialize(YAML::Emitter &e) const {
	e << YAML::Flow << YAML::BeginMap;
	Message::serialize(e); 
	e << YAML::Key << "min" << YAML::Value << min_;
	e << YAML::Key << "max" << YAML::Value << max_;
	e << YAML::Key << "cur" << YAML::Value << cur_;
	e << YAML::EndMap; 
	return e; 
}
YAML::Node &PlaybackTimeRangeMessage::serialize(YAML::Node &node) {
	// Do not serialize base data GTL - Message::serialize(node); 
	node["min"] >> min_;
	node["max"] >> max_;
	node["cur"] >> cur_;
	return node;
}

} // namespace

} // namespace

