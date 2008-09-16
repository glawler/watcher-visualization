#include "message.h"

#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/export.hpp>

using namespace std;
using namespace watcher;

INIT_LOGGER(Message, "Message");

BOOST_CLASS_EXPORT_GUID(Message, "Message");

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

// virtual 
std::ostream &Message::toStream(std::ostream &out) const
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

void Message::serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & version;
    ar & type;
    TRACE_EXIT();
}
void Message::serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & version;
    ar & type;
    TRACE_EXIT();
}
