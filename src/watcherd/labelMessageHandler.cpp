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

MessageHandler::ConnectionCommand LabelMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(LABEL_MESSAGE_TYPE); 

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

