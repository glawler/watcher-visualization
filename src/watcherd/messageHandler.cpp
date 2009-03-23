#include <boost/cast.hpp>

#include "messageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(MessageHandler, "MessageHandler");

MessageHandler::MessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::~MessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::ConnectionCommand MessageHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Producing reply for message: " << *request);
    reply=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Produced reply: " << *reply);

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

MessageHandler::ConnectionCommand MessageHandler::handleReply(const MessagePtr &request, const MessagePtr &reply)
{
    TRACE_ENTER();

    LOG_INFO("Recv'd :" << "\n\t" << *reply << endl << "In reply to: " << "\n\t" << *request);

    if (reply->type != MESSAGE_STATUS_TYPE)
    {
        LOG_WARN("Got a non-message status message in reply.");
        TRACE_EXIT_RET("closeConnection");
        return closeConnection;
    }

    MessageStatusPtr mess=boost::dynamic_pointer_cast<MessageStatus>(reply);
    
    if(mess->status!=MessageStatus::status_ack && mess->status!=MessageStatus::status_ok) 
    {
        LOG_WARN("Recv'd non ack reply to request: " << MessageStatus::statusToString(mess->status)); 
    }
    else
    {
        LOG_INFO("Recv'd ack to request, all is well."); 
    }

    TRACE_EXIT_RET("stayConnected");
    return stayConnected;
}

void MessageHandler::handleMessageArrive(const MessagePtr message)
{
    TRACE_ENTER();
    LOG_INFO("Recv'd message: " << message); 
    TRACE_EXIT();
}

