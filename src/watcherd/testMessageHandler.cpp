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

bool TestMessageHandler::produceReply(const boost::shared_ptr<Message> &request, boost::shared_ptr<Message> &reply)
{
    TRACE_ENTER();
    
    reply=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Produced reply: " << *reply);

    TRACE_EXIT_RET("true"); 
    return true;
}

bool TestMessageHandler::produceRequest(boost::shared_ptr<Message> &request)
{
    TRACE_ENTER();
    
    request=MessageFactory::makeMessage(TEST_MESSAGE_TYPE); 
    LOG_DEBUG("Produced request: " << *request);

    TRACE_EXIT_RET("true"); 
    return true;
}

