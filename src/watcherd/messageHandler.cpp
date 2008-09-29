#include <boost/cast.hpp>

#include "messageHandler.h"
#include "messageStatus.h"

using namespace watcher;
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

    LOG_INFO("Recv'd " << *reply << " as reply to " << *request);

    if (reply->type != MESSAGE_STATUS_TYPE)
    {
        LOG_WARN("Got a non-message status message in reply.");
        TRACE_EXIT_RET("closeConnection");
        return closeConnection;
    }

    MessageStatusPtr mess=MessageStatusPtr(boost::polymorphic_downcast<MessageStatus*>(reply.get()));
    
    if(mess->status!=MessageStatus::status_ack || 
       mess->status!=MessageStatus::status_ok) 
    {
        LOG_WARN("Recv'd non ack reply to request");
    }
    else
    {
        LOG_INFO("Recv'd aak to request, all is well."); 
    }

    TRACE_EXIT_RET("closeConnection");
    return closeConnection;
}

