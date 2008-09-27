#include "edgeMessageHandler.h"
#include "messageFactory.h"
#include "messageStatus.h"

using namespace watcher;

INIT_LOGGER(EdgeMessageHandler, "EdgeMessageHandler");

EdgeMessageHandler::EdgeMessageHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

EdgeMessageHandler::~EdgeMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool EdgeMessageHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Producing reply for message: " << *request);
    reply=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Produced reply: " << *reply);

    TRACE_EXIT_RET("true"); 
    return true;
}

bool EdgeMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(EDGE_MESSAGE_TYPE); 

    TRACE_EXIT_RET("true"); 
    return true;
}

