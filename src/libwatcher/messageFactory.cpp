/* Copyright 2012 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 * @file messageFactory.cpp
 * @author Geoff Lawler <geoff.lawler@sparta.com> 
 * @date 2009-07-15
 */
#include "messageTypesAndVersions.h"
#include "message.h"
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
#include "playbackTimeRange.h"
#include "messageStreamFilterMessage.h"
#include "subscribeStreamMessage.h"
#include "streamDescriptionMessage.h" 
#include "listStreamsMessage.h"
#include "dataPointMessage.h"
#include "nodePropertiesMessage.h"

namespace watcher {
    namespace event {
		// Factory method for messages. 
        MessagePtr createMessage(MessageType t)
        {
            // TODO maybe this should be a map instead of a swich statement? - melkins
			// How would that work? With the switch() the compiler can tell us if we've
			// missed any TYPEs. (At least g++ does.) - GTL 
            switch (t) {
				case UNKNOWN_MESSAGE_TYPE:
					return MessagePtr(); 
					break;
                case MESSAGE_STATUS_TYPE:
                    return MessageStatusPtr(new MessageStatus);
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
                case NODE_STATUS_MESSAGE_TYPE:
                    return NodeStatusMessagePtr(new NodeStatusMessage);
                    break;
                case DATA_POINT_MESSAGE_TYPE:
                    return DataPointMessagePtr(new DataPointMessage);
                    break;
                case NODE_PROPERTIES_MESSAGE_TYPE:
                    return NodePropertiesMessagePtr(new NodePropertiesMessage);
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
                case PLAYBACK_TIME_RANGE_MESSAGE_TYPE:
                    return PlaybackTimeRangeMessagePtr(new PlaybackTimeRangeMessage);
                    break;
                case MESSAGE_STREAM_FILTER_MESSAGE_TYPE:
                    return MessageStreamFilterMessagePtr(new MessageStreamFilterMessage);
                    break;
                case SUBSCRIBE_STREAM_MESSAGE_TYPE:
                    return SubscribeStreamMessagePtr(new SubscribeStreamMessage);
                    break;
                case STREAM_DESCRIPTION_MESSAGE_TYPE:
                    return StreamDescriptionMessagePtr(new StreamDescriptionMessage);
                    break;
                case LIST_STREAMS_MESSAGE_TYPE:
                    return ListStreamsMessagePtr(new ListStreamsMessage);
                    break;
				case USER_DEFINED_MESSAGE_TYPE:
					return MessagePtr(); 
					break;
            }
            throw std::runtime_error("invalid message type in message");
        }
    }
}
