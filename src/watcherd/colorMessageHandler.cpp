#include "colorMessageHandler.h"
#include "messageFactory.h"
#include <libwatcher/messageStatus.h>
#include <boost/cast.hpp>

using namespace watcher;
using namespace watcher::event;

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

MessageHandler::ConnectionCommand ColorMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(COLOR_MESSAGE_TYPE); 

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

