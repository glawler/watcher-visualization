#include <map>

#include "message.h"

#include "testMessage.h"
#include "messageStatus.h"
#include "gpsMessage.h"
#include "labelMessage.h"
#include "edgeMessage.h"
#include "colorMessage.h"
#include "connectivityMessage.h"
#include "seekWatcherMessage.h"
#include "startWatcherMessage.h"
#include "stopWatcherMessage.h"
#include "speedWatcherMessage.h"
#include "nodeStatusMessage.h"

namespace watcher {
    namespace event {
        MessagePtr Message::create(MessageType t)
        {
            // TODO maybe this should be a map instead of a swich statement? - melkins
            switch (t) {
                case MESSAGE_STATUS_TYPE:
                    return MessageStatusPtr ( new MessageStatus() );
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
                case CONNECTIVITY_MESSAGE_TYPE:
                    return ConnectivityMessagePtr(new ConnectivityMessage);
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
                case NODE_STATUS_MESSAGE_TYPE:
                    return NodeStatusMessagePtr(new NodeStatusMessage);
                    break;
            }
            throw std::runtime_error("invalid message type in message");
        }
    }
}
