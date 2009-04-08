#include <boost/cast.hpp>

#include "clientMessageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(ClientMessageHandler, "MessageHandler.ClientMessageHandler");

ClientMessageHandler::ClientMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

ClientMessageHandler::~ClientMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool ClientMessageHandler::handleMessageArrive(const MessagePtr message, MessagePtr &response)
{
    TRACE_ENTER();

    MessageHandler::handleMessageArrive(message, response); 

    bool retVal;
    if(message->type!=MESSAGE_STATUS_TYPE)
    {
        LOG_ERROR("Server responded with non status message"); 
        retVal=false;
    }
    else
    {
        MessageStatusPtr mess=boost::dynamic_pointer_cast<MessageStatus>(message);
        if(!mess)
        {
            LOG_ERROR("Unable to convert message into a status message - soemthing is very wrong."); 
            retVal=false;
        }
        else if(mess->status!=MessageStatus::status_ack || mess->status!=MessageStatus::status_ok)
        {
            LOG_WARN("Server didn't ack or give us an OK for the message we sent them. Status: " << mess->statusToString(mess->status));
            retVal=false;
        }
        retVal=true;
    }
    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

// virtual 
bool ClientMessageHandler::handleMessagesArrive(const std::vector<event::MessagePtr> &messages, event::MessagePtr &response)
{
    TRACE_ENTER();

    MessageHandler::handleMessagesArrive(messages, response); 

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(!handleMessageArrive(*i, response))  // may be overwriting responses from earlier loop iterations...
        {
            TRACE_EXIT_RET("false"); 
            return false;
        }
    }

    TRACE_EXIT_RET("true");
    return true;
    TRACE_EXIT(); 
}
