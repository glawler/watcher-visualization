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

/** @file seekWatcherMessage.cpp
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#include "seekWatcherMessage.h"
#include "logger.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(SeekMessage, "Message.SeekMessage");

        Timestamp const SeekMessage::epoch;
        Timestamp const SeekMessage::eof;

        /**
         * Seek to a particular point in time in the event stream.
         * @param offset_ time at which to seek to
         * @param w how to interpret the time offset
         */
        SeekMessage::SeekMessage(Timestamp offset_, whence w)
            : Message(SEEK_MESSAGE_TYPE, SEEK_MESSAGE_VERSION), offset(offset_), rel(w)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        std::ostream& SeekMessage::toStream (std::ostream& o) const
        {
	    Message::toStream(o);
            return o << " offset=" << offset << ", rel=" << rel;
        }

        std::ostream& operator<< (std::ostream& o, const SeekMessage& m)
        {
	    return m.toStream(o);
        }

		YAML::Emitter &SeekMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "offset" << YAML::Value << offset;
			e << YAML::Key << "rel" << YAML::Value << static_cast<unsigned short>(rel); 
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &SeekMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			node["offset"] >> offset;
			node["rel"] >> (unsigned short&)rel;
			return node;
		}
    }
}

