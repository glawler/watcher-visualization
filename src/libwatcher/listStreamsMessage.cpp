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
#include "watcherSerialize.h"
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

template <typename Archive> void ListStreamsMessage::serialize(Archive& ar, const unsigned int /* version */)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & evstreams;
    TRACE_EXIT();
}

template <typename Archive> void EventStreamInfo::serialize(Archive& ar, const unsigned int /* version */)
{
    //TRACE_ENTER();
    ar & uid;
    ar & description;
    //TRACE_EXIT();
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

} // namespace

} // namespace

BOOST_CLASS_EXPORT(watcher::event::EventStreamInfo)
BOOST_CLASS_EXPORT(watcher::event::ListStreamsMessage)
