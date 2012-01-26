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

/**
 * @file messageStreamFilterMessage.cpp
 * @author Geoff Lawler <geoff.lawer@cobham.com>
 * @date 2009-07-15
 */
#include <boost/foreach.hpp>

#include "messageStreamFilterMessage.h"
#include "logger.h"

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {
        INIT_LOGGER(MessageStreamFilterMessage, "Message.MessageStreamFilterMessage");

        MessageStreamFilterMessage::MessageStreamFilterMessage() : 
            Message(MESSAGE_STREAM_FILTER_MESSAGE_TYPE, MESSAGE_STREAM_FILTER_MESSAGE_VERSION),
            applyFilter(true),
            enableAllFiltering(true)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        MessageStreamFilterMessage::MessageStreamFilterMessage(const MessageStreamFilter &filter) : 
            Message(MESSAGE_STREAM_FILTER_MESSAGE_TYPE, MESSAGE_STREAM_FILTER_MESSAGE_VERSION),
            applyFilter(true),
            enableAllFiltering(true),
            theFilter(filter)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        MessageStreamFilterMessage::MessageStreamFilterMessage(const MessageStreamFilterMessage &other) : 
            Message(other),
            applyFilter(other.applyFilter),
            enableAllFiltering(other.enableAllFiltering), 
            theFilter(other.theFilter) 
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool MessageStreamFilterMessage::operator==(const MessageStreamFilterMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                applyFilter==other.applyFilter &&
                enableAllFiltering==other.enableAllFiltering && 
                theFilter==other.theFilter;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        MessageStreamFilterMessage &MessageStreamFilterMessage::operator=(const MessageStreamFilterMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            applyFilter=other.applyFilter;
            enableAllFiltering=other.enableAllFiltering;
            theFilter=other.theFilter;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &MessageStreamFilterMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << "(" << (applyFilter?"apply":"remove") << " filter) ";
            out << " filtering: " << (enableAllFiltering?"on":"off"); 
            out << " filter: " << theFilter;

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const MessageStreamFilterMessage &mess)
        {
            mess.operator<<(out);
            return out;
        }

		YAML::Emitter &MessageStreamFilterMessage::serialize(YAML::Emitter &e) const {
			e << YAML::Flow << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "applyFilter" << YAML::Value << applyFilter;
			e << YAML::Key << "enableAllFiltering" << YAML::Value << enableAllFiltering;
			e << YAML::Key << "theFilter" << YAML::Value; 
				e << YAML::Flow << YAML::BeginMap; 
				e << YAML::Key << "layers" << YAML::Value << theFilter.layers;
				e << YAML::Key << "messageTypes" << YAML::Value << theFilter.messageTypes;
				e << YAML::Key << "opAND" << YAML::Value << theFilter.opAND; 
				// region is currently data free, so don't bother serializing
				e << YAML::EndMap; 
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &MessageStreamFilterMessage::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			node["applyFilter"] >> applyFilter;
			node["enableAllFiltering"] >> enableAllFiltering;
			const YAML::Node &filter=node["theFilter"]; 
			filter["layers"] >> theFilter.layers; 
			filter["messageTypes"] >> theFilter.messageTypes; 
			filter["opAND"] >> theFilter.opAND; 
			// region not encoded. 
			return node;
		}
    }
}

