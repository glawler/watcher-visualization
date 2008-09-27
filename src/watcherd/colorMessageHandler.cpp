#include "colorMessageHandler.h"
#include "messageFactory.h"
#include "messageStatus.h"

using namespace watcher;

INIT_LOGGER(ColorMessageHandler, "ColorMessageHandler");

ColorMessageHandler::ColorMessageHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

ColorMessageHandler::~ColorMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool ColorMessageHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Producing reply for message: " << *request);
    reply=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Produced reply: " << *reply);

    TRACE_EXIT_RET("true"); 
    return true;
}

bool ColorMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(COLOR_MESSAGE_TYPE); 

    TRACE_EXIT_RET("true"); 
    return true;
}
