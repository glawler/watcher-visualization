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

/** @file streamDescriptionMessage.cpp
 */
#include "streamDescriptionMessage.h"
#include "logger.h"

namespace watcher {
	namespace event {
		INIT_LOGGER(StreamDescriptionMessage, "Message.StreamDescriptionMessage");

		StreamDescriptionMessage::StreamDescriptionMessage() :
			Message(STREAM_DESCRIPTION_MESSAGE_TYPE, STREAM_DESCRIPTION_MESSAGE_VERSION)
		{
			TRACE_ENTER();
			TRACE_EXIT();
		}

		StreamDescriptionMessage::StreamDescriptionMessage(const std::string& desc_) :
			Message(STREAM_DESCRIPTION_MESSAGE_TYPE, STREAM_DESCRIPTION_MESSAGE_VERSION),
			desc(desc_)
		{
			TRACE_ENTER();
			TRACE_EXIT();
		}

		std::ostream& operator<< (std::ostream& os, const StreamDescriptionMessagePtr& p)
		{
			return os << "[StreamDescriptionMessage desc=" << p->desc << "]\n";
		}

		YAML::Emitter &StreamDescriptionMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "desc" << YAML::Value << desc;
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &StreamDescriptionMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			node["desc"] >> desc;
			return node;
		}
	}
}

