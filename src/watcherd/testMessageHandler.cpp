#include "testMessageHandler.h"
#include "messageFactory.h"
#include "libwatcher/messageStatus.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(TestMessageHandler, "TestMessageHandler");

TestMessageHandler::TestMessageHandler() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

TestMessageHandler::~TestMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::ConnectionCommand TestMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(TEST_MESSAGE_TYPE); 
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("writeMessage"); 
    return writeMessage;
}

