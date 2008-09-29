#include "gpsMessageHandler.h"
#include "messageFactory.h"
#include "messageStatus.h"

using namespace watcher;

INIT_LOGGER(GPSMessageHandler, "GPSMessageHandler");

GPSMessageHandler::GPSMessageHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

GPSMessageHandler::~GPSMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::ConnectionCommand GPSMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(GPS_MESSAGE_TYPE); 

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

