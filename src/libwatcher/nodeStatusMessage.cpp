/** 
 * @file
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-23
 */
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>

#include "nodeStatusMessage.h"
#include "watcherGlobalFunctions.h" // for NodeIdentifier serialization...

using namespace std;
using namespace watcher;
using namespace watcher::event; 

INIT_LOGGER(NodeStatusMessage, "Message.NodeStatusMessage");

NodeStatusMessage::NodeStatusMessage(const statusEvent &event_, const NodeIdentifier &nodeId_) : 
    Message(NODE_STATUS_MESSAGE_TYPE, NODE_STATUS_MESSAGE_VERSION), 
    event(event_),
    nodeId(nodeId_)
{
    TRACE_ENTER();
    TRACE_EXIT(); 
}

NodeStatusMessage::NodeStatusMessage(const NodeStatusMessage &rhs) : 
    Message(rhs.type, rhs.version),
    event(rhs.event),
    nodeId(rhs.nodeId)
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
        nodeId==other.nodeId;
    TRACE_EXIT_RET(retVal);
    return retVal;
}

NodeStatusMessage &NodeStatusMessage::operator=(const NodeStatusMessage &other)
{
    TRACE_ENTER();
    Message::operator=(other);
    event=other.event;
    nodeId=other.nodeId;
    TRACE_EXIT();
    return *this;
}

// virtual 
std::ostream &NodeStatusMessage::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    Message::toStream(out); 
    out << "event: " << statusEventToString(event) << " ";
    out << "nodeId: " << nodeId.to_string(); 
    TRACE_EXIT();
    return out;
}

std::ostream &NodeStatusMessage::operator<<(std::ostream &out) const
{
    TRACE_ENTER();
    TRACE_EXIT();
    return toStream(out); 
}


template <typename Archive>
void NodeStatusMessage::serialize(Archive& ar, const unsigned int /* version */)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & event;
    ar & nodeId;
    TRACE_EXIT();
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
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}
