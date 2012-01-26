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

/** @file speedWatcherMessage.cpp
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#include "speedWatcherMessage.h"
#include "logger.h"

namespace watcher {
    namespace event {
        /**
         * Set the playback speed of the event stream.
         * A negative value indicates reverse direction.
         */
        SpeedMessage::SpeedMessage(float speed_)
            : Message(SPEED_MESSAGE_TYPE, SPEED_MESSAGE_VERSION), speed(speed_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
       
	std::ostream& SpeedMessage::toStream(std::ostream& os) const
	{
	    Message::toStream(os);
            return os << " speed=" << speed;
	}

        std::ostream& operator<< (std::ostream& o, const SpeedMessage& rhs)
        {
	    return rhs.toStream(o);
        }


		YAML::Emitter &SpeedMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "speed" << YAML::Value << speed;
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &SpeedMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			node["speed"] >> speed;
			return node;
		}

        INIT_LOGGER(SpeedMessage, "Message.SpeedMessage");
    }
}

