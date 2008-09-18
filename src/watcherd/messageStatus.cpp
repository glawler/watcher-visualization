#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include "messageStatus.h"
#include "messageTypesAndVersions.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(MessageStatus, "Message.Status");
BOOST_CLASS_EXPORT_GUID(MessageStatus, "MessageStatus"); 

MessageStatus::MessageStatus(const Status stat) : 
    Message(MESSAGE_STATUS_TYPE, MESSAGE_STATUS_VERSION),
    status(stat)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageStatus::MessageStatus(const MessageStatus &other)
{
    TRACE_ENTER();
    (*this)=other;
    TRACE_EXIT();
}

MessageStatus::~MessageStatus()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool MessageStatus::operator==(const MessageStatus &other) const
{
    TRACE_ENTER();
    
    bool retVal = 
        Message::operator==(other) && 
        status==other.status;

    TRACE_EXIT_RET(retVal);
    return retVal;
}

MessageStatus &MessageStatus::operator=(const MessageStatus &other)
{
    TRACE_ENTER();
    Message::operator=(other);
    status=other.status;
    TRACE_EXIT();
    return *this;
}

// static
const char *MessageStatus::statusToString(const Status &status)
{
    TRACE_ENTER();
    const char *retVal;
    switch(status)
    {
        case status_ok: retVal="ok"; break;
        case status_error: retVal="error"; break;
        case status_ack: retVal="ack"; break;
        case status_nack: retVal="nack"; break;
        case status_disconnected: retVal="disconnected"; break;
    }
    TRACE_EXIT_RET(retVal);
    return retVal;
}

// virtual
std::ostream &MessageStatus::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    Message::toStream(out); 
    out << " status : " << statusToString(status);
    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const MessageStatus &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}

void MessageStatus::serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & status;
    TRACE_EXIT();
}
void MessageStatus::serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & status;
    TRACE_EXIT();
}

