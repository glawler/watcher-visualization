#include "testMessageHandler.h"
#include "messageFactory.h"

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

bool TestMessageHandler::produceReply(const boost::shared_ptr<Message> &request, boost::shared_ptr<Message> &reply)
{
    TRACE_ENTER();
    LOG_DEBUG("Producing reply for message: " << *request);
    TRACE_EXIT_RET("false"); 
    return false;
}

bool TestMessageHandler::produceRequest(boost::shared_ptr<Message> &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(TEST_MESSAGE_TYPE);
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("false"); 
    return false;
}

