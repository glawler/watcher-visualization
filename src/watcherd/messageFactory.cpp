#include "messageFactory.h"

#include <libwatcher/message.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/colorMessage.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/messageStatus.h>
#include <libwatcher/dataRequestMessage.h>
//#include "testMessage.h"

using namespace watcher;
using namespace watcher::event;

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
            /*
        case TEST_MESSAGE_TYPE:
            return boost::shared_ptr<TestMessage>(new TestMessage);
            break; */
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

