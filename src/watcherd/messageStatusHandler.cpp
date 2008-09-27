#include "messageStatusHandler.h"
#include "messageFactory.h"

using namespace watcher;

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

bool MessageStatusHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Producing reply for message: " << *request);
    TRACE_EXIT_RET("false"); 
    return false;
}

bool MessageStatusHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(MESSAGE_STATUS_TYPE);
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("false"); 
    return false;
}

