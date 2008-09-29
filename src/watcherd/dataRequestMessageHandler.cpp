#include "dataRequestMessageHandler.h"
#include "messageFactory.h"
#include "messageStatus.h"

using namespace watcher;

INIT_LOGGER(DataRequestMessageHandler, "DataRequestMessageHandler");

DataRequestMessageHandler::DataRequestMessageHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

DataRequestMessageHandler::~DataRequestMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::ConnectionCommand DataRequestMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(DATA_REQUEST_MESSAGE_TYPE); 
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

