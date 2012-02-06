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

/** @file listStreamsMessage.cpp
 */
#include "listStreamsMessage.h"
#include "logger.h"
#include <boost/foreach.hpp>

namespace watcher {
namespace event {

INIT_LOGGER(ListStreamsMessage, "Message.ListStreamsMessage");

EventStreamInfo::EventStreamInfo() : uid(-1)
{
}

EventStreamInfo::EventStreamInfo(uint32_t uid_, const std::string& desc_) : uid(uid_), description(desc_)
{
}

ListStreamsMessage::ListStreamsMessage() : Message(LIST_STREAMS_MESSAGE_TYPE, LIST_STREAMS_MESSAGE_VERSION)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

std::ostream& operator<< (std::ostream& os, const EventStreamInfo& p)
{
    return os << "[EventStreamInfo uid=" << p.uid << " description=" << p.description << "]";
}

std::ostream& operator<< (std::ostream& os, const ListStreamsMessage& p)
{
    os << "[ListStreamsMessage count=" << p.evstreams.size();
    BOOST_FOREACH(const EventStreamInfoPtr& streamInfo, p.evstreams) {
	os << "\n\t" << *streamInfo;
    }
    return os << "]";
}

// virtual
std::ostream& ListStreamsMessage::toStream(std::ostream& os) const
{
    return Message::toStream(os) << *this;
}


YAML::Emitter &ListStreamsMessage::serialize(YAML::Emitter &e) const {
	e << YAML::Flow << YAML::BeginMap;
	Message::serialize(e); 
	e << YAML::Key << "events" << YAML::Value; 
		e << YAML::Flow << YAML::BeginSeq; 
		BOOST_FOREACH(const EventStreamInfoPtr ev, evstreams) {
			e << YAML::Flow << YAML::BeginMap;
			e << YAML::Key << "uid" << YAML::Value << ev->uid;
			e << YAML::Key << "description" << YAML::Value << ev->description;
			e << YAML::EndMap; 
		}
		e << YAML::EndSeq; 
	e << YAML::EndMap; 
	return e; 
}
YAML::Node &ListStreamsMessage::serialize(YAML::Node &node) {
	// Do not serialize base data GTL - Message::serialize(node); 
	const YAML::Node &events=node["events"];  // a Sequence
	for (unsigned i=0;i<events.size();i++) {
		EventStreamInfoPtr ev(new EventStreamInfo); 
		events[i]["uid"] >> ev->uid;
		events[i]["description"] >> ev->description;
		evstreams.push_back(ev); 
	}
	return node;
}

} // namespace

} // namespace

