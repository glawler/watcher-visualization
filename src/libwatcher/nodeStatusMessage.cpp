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
 * @file nodeStatusMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-03-23
 */
#include "nodeStatusMessage.h"
#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event; 

INIT_LOGGER(NodeStatusMessage, "Message.NodeStatusMessage");

NodeStatusMessage::NodeStatusMessage(const statusEvent &event_) : 
	Message(NODE_STATUS_MESSAGE_TYPE, NODE_STATUS_MESSAGE_VERSION), 
	event(event_),
	layer(PHYSICAL_LAYER)
{
	TRACE_ENTER();
	TRACE_EXIT(); 
}

NodeStatusMessage::NodeStatusMessage(const NodeStatusMessage &rhs) : 
	Message(rhs.type, rhs.version),
	event(rhs.event),
	layer(rhs.layer)
{
	TRACE_ENTER();
	TRACE_EXIT(); 
}

NodeStatusMessage::~NodeStatusMessage()
{
	TRACE_ENTER();
	TRACE_EXIT();
}

bool NodeStatusMessage::operator==(const NodeStatusMessage &other) const
{
	TRACE_ENTER();
	bool retVal = 
		Message::operator==(other) && 
		event==other.event && 
		layer==other.layer; 
	TRACE_EXIT_RET(retVal);
	return retVal;
}

NodeStatusMessage &NodeStatusMessage::operator=(const NodeStatusMessage &other)
{
	TRACE_ENTER();
	Message::operator=(other);
	event=other.event;
	layer=other.layer; 
	TRACE_EXIT();
	return *this;
}

// virtual 
std::ostream &NodeStatusMessage::toStream(std::ostream &out) const
{
	TRACE_ENTER();
	Message::toStream(out); 
	out << "event: " << statusEventToString(event) << " layer: " << layer; 
	TRACE_EXIT();
	return out;
}

std::ostream &NodeStatusMessage::operator<<(std::ostream &out) const
{
	TRACE_ENTER();
	TRACE_EXIT();
	return toStream(out); 
}
// static
string NodeStatusMessage::statusEventToString(const NodeStatusMessage::statusEvent &e)
{
	switch(e)
	{
		case connect: return "connect"; break;
		case disconnect: return "disconnect"; break;
	}
	return ""; 
}

ostream &operator<<(ostream &out, const NodeStatusMessage &mess)
{
	mess.operator<<(out);
	return out;
}


YAML::Emitter &NodeStatusMessage::serialize(YAML::Emitter &e) const {
	e << YAML::Flow << YAML::BeginMap;
	Message::serialize(e); 
	e << YAML::Key << "event" << YAML::Value << static_cast<unsigned short>(event);
	e << YAML::Key << "layer" << YAML::Value << layer;
	e << YAML::EndMap; 
	return e; 
}
YAML::Node &NodeStatusMessage::serialize(YAML::Node &node) {
	// Do not serialize base data GTL - Message::serialize(node); 
	node["event"] >> (unsigned short &)event;
	node["layer"] >> layer;
	return node;
}
