#include "messageStatusHandler.h"
#include "messageFactory.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(MessageStatusHandler, "MessageStatusHandler");

MessageStatusHandler::MessageStatusHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageStatusHandler::~MessageStatusHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::ConnectionCommand MessageStatusHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Not producing reply for status message: " << *request);
    TRACE_EXIT_RET("closeConnection");
    return closeConnection;
}

MessageHandler::ConnectionCommand MessageStatusHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(MESSAGE_STATUS_TYPE);
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

