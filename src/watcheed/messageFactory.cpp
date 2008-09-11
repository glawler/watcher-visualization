#include "messageFactory.h"

#include "message.h"
#include "testMessage.h"

using namespace watcher;

INIT_LOGGER(MessageFactory, "MessageFactory");

// static
boost::shared_ptr<Message> MessageFactory::makeMessage(const MessageType &type)
{
    switch(type)
    {
        case UNKNOWN_MESSAGE_TYPE: 
            return boost::shared_ptr<Message>(); 
            break;
        case MESSAGE_HEADER:
            return boost::shared_ptr<Message>(new Message(MESSAGE_HEADER, 0));
            break;
        case TEST_MESSAGE_TYPE:
            return boost::shared_ptr<TestMessage>(new TestMessage);
            break;
    }
    return boost::shared_ptr<Message>(); // == NULL
}

