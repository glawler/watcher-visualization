#include "messageFactory.h"

#include "message.h"
#include "testMessage.h"
#include "gpsMessage.h"
#include "messageStatus.h"

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
        case MESSAGE_STATUS_TYPE:
            return boost::shared_ptr<MessageStatus>(new MessageStatus);
            break;
        case TEST_MESSAGE_TYPE:
            return boost::shared_ptr<TestMessage>(new TestMessage);
            break;
        case GPS_MESSAGE_TYPE:
            return boost::shared_ptr<GPSMessage>(new GPSMessage);
            break;
    }
    return boost::shared_ptr<Message>(); // == NULL
}

