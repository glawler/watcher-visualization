#include "labelMessageHandler.h"
#include "messageFactory.h"
#include "messageStatus.h"

using namespace watcher;

INIT_LOGGER(LabelMessageHandler, "LabelMessageHandler");

LabelMessageHandler::LabelMessageHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

LabelMessageHandler::~LabelMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool LabelMessageHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Producing reply for message: " << *request);
    reply=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Produced reply: " << *reply);

    TRACE_EXIT_RET("true"); 
    return true;
}

bool LabelMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(LABEL_MESSAGE_TYPE); 

    TRACE_EXIT_RET("true"); 
    return true;
}

