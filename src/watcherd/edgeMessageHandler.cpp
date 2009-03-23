#include "edgeMessageHandler.h"
#include "messageFactory.h"
#include <libwatcher/messageStatus.h>

using namespace watcher;
using namespace watcher::event;

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

