#include "messageFactory.h"

#include "message.h"
#include "testMessage.h"
#include "gpsMessage.h"
#include "messageStatus.h"
#include "labelMessage.h"
#include "edgeMessage.h"
#include "colorMessage.h"
#include "dataRequestMessage.h"

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
        case LABEL_MESSAGE_TYPE:
            return boost::shared_ptr<LabelMessage>(new LabelMessage);
            break;
        case EDGE_MESSAGE_TYPE:
            return boost::shared_ptr<EdgeMessage>(new EdgeMessage);
            break;
        case COLOR_MESSAGE_TYPE:
            return boost::shared_ptr<ColorMessage>(new ColorMessage);
            break;
        case DATA_REQUEST_MESSAGE_TYPE:
            return boost::shared_ptr<DataRequestMessage>(new DataRequestMessage);
            break;

            
            // GTL - do not put a default or the compiler
            // won't tell us when we've missed a case.
    }
    return boost::shared_ptr<Message>(); // == NULL
}

