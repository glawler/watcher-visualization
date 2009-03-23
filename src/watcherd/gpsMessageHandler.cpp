#include "gpsMessageHandler.h"
#include "messageFactory.h"
#include <libwatcher/messageStatus.h>

using namespace watcher;
using namespace watcher::event;

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

