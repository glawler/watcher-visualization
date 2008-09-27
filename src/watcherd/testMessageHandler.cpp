#include "testMessageHandler.h"
#include "messageFactory.h"
#include "messageStatus.h"

using namespace watcher;

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

bool TestMessageHandler::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    
    LOG_DEBUG("Producing reply for message: " << *request);
    reply=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Produced reply: " << *reply);

    TRACE_EXIT_RET("true"); 
    return true;
}

bool TestMessageHandler::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(TEST_MESSAGE_TYPE); 
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("true"); 
    return true;
}

