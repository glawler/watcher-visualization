#include "messageFactory.h"

#include "libwatcher/message.h"
#include "libwatcher/labelMessage.h"
#include "libwatcher/edgeMessage.h"
#include "libwatcher/colorMessage.h"
#include "libwatcher/gpsMessage.h"
#include "libwatcher/messageStatus.h"
#include "libwatcher/dataRequestMessage.h"
#include "libwatcher/testMessage.h"
#include "libwatcher/seekWatcherMessage.h"
#include "libwatcher/startWatcherMessage.h"
#include "libwatcher/stopWatcherMessage.h"
#include "libwatcher/speedWatcherMessage.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(MessageFactory, "MessageFactory");

// static
boost::shared_ptr<Message> MessageFactory::makeMessage(const MessageType &type)
{
    switch(type)
    {
        case MESSAGE_STATUS_TYPE:
            return MessageStatusPtr(new MessageStatus);
            break;
        case TEST_MESSAGE_TYPE:
            return TestMessagePtr(new TestMessage);
            break; 
        case GPS_MESSAGE_TYPE:
            return GPSMessagePtr(new GPSMessage);
            break;
        case LABEL_MESSAGE_TYPE:
            return LabelMessagePtr(new LabelMessage);
            break;
        case EDGE_MESSAGE_TYPE:
            return EdgeMessagePtr(new EdgeMessage);
            break;
        case COLOR_MESSAGE_TYPE:
            return ColorMessagePtr(new ColorMessage);
            break;
        case DATA_REQUEST_MESSAGE_TYPE:
            return DataRequestMessagePtr(new DataRequestMessage);
            break;
        case SEEK_MESSAGE_TYPE:
            return SeekMessagePtr(new SeekMessage);
            break;
        case START_MESSAGE_TYPE:
            return StartMessagePtr(new StartMessage);
            break;
        case STOP_MESSAGE_TYPE:
            return StopMessagePtr(new StopMessage);
            break;
        case SPEED_MESSAGE_TYPE:
            return SpeedMessagePtr(new SpeedMessage);
            break;

            // let used defined and unknown fall through - a NULL will be returned.
        case UNKNOWN_MESSAGE_TYPE: 
        case USER_DEFINED_MESSAGE_TYPE:
            break;
            
            // GTL - do not put a default or the compiler
            // won't tell us when we've missed a case.
    }
    return MessagePtr(); // == NULL
}

