#include "message.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(Message, "Message");

Message::Message() : 
    version(0), type(UNKNOWN_MESSAGE_TYPE)
{

}

Message::Message(const MessageType &t, const unsigned int v) : 
    version(v), type(t)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

Message::Message(const Message &other)
{
    TRACE_ENTER();
    (*this)=other;
    TRACE_EXIT();
}

Message::~Message()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool Message::operator==(const Message &other) const
{
    TRACE_ENTER();
    bool retVal = version==other.version && type==other.type;
    TRACE_EXIT_RET(retVal);
    return retVal;
}

Message &Message::operator=(const Message &other)
{
    TRACE_ENTER();
    version=other.version;
    type=other.type;
    TRACE_EXIT();
    return *this;
}

ostream &Message::operator<<(ostream &out) const
{
    TRACE_ENTER();
    out << " version: " << version << " type: " << type << " "; 
    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const Message &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}

