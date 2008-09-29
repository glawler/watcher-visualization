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

MessageHandler::ConnectionCommand EdgeMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(EDGE_MESSAGE_TYPE); 

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

