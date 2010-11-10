/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @file messageTypesAndVersions.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef MESSAGETYPES_AND_VERSIONS_H
#define MESSAGETYPES_AND_VERSIONS_H

#include <ostream>
#include "message_fwd.h"

namespace watcher {
    namespace event {
        //
        // An enum which contains all possible messages. 
        //
        typedef enum 
        {
            // feeder API messages
            UNKNOWN_MESSAGE_TYPE      = 0x00000000,
            MESSAGE_STATUS_TYPE       = 0x00000001,
            TEST_MESSAGE_TYPE         = 0x00000002,
            GPS_MESSAGE_TYPE          = 0x00000003,
            LABEL_MESSAGE_TYPE        = 0x00000004,
            EDGE_MESSAGE_TYPE         = 0x00000005,
            COLOR_MESSAGE_TYPE        = 0x00000006,
            CONNECTIVITY_MESSAGE_TYPE = 0x00000007,
            NODE_STATUS_MESSAGE_TYPE  = 0x00000008,
            DATA_POINT_MESSAGE_TYPE   = 0x00000009,
            NODE_PROPERTIES_MESSAGE_TYPE = 0x0000000a,

            // watcherdAPI messages
            SEEK_MESSAGE_TYPE         = 0x0000ff00, // DO NOT REORDER THIS WITHOUT CHANGING isFeederEvent
            START_MESSAGE_TYPE        = 0x0000ff01,
            STOP_MESSAGE_TYPE         = 0x0000ff02,
            SPEED_MESSAGE_TYPE        = 0x0000ff03,
            PLAYBACK_TIME_RANGE_MESSAGE_TYPE = 0x0000ff04,
            MESSAGE_STREAM_FILTER_MESSAGE_TYPE = 0x0000ff05,
            SUBSCRIBE_STREAM_MESSAGE_TYPE = 0x0000ff06,
            STREAM_DESCRIPTION_MESSAGE_TYPE = 0x0000ff07,
            LIST_STREAMS_MESSAGE_TYPE = 0x0000ff08,

            USER_DEFINED_MESSAGE_TYPE = 0xffff0000
        } MessageType;

        std::ostream& operator<< (std::ostream &out, const MessageType &type);

        //
        // version numbers are on a per message format basis
        // I don't know that these will ever really change.
        // 
        const unsigned int BASE_MESSAGE_VERSION         = 1;
        const unsigned int MESSAGE_STATUS_VERSION       = 1;
        const unsigned int MESSAGE_TEST_VERSION         = 1;
        const unsigned int GPS_MESSAGE_VERSION          = 1;
        const unsigned int LABEL_MESSAGE_VERSION        = 1;
        const unsigned int EDGE_MESSAGE_VERSION         = 1;
        const unsigned int COLOR_MESSAGE_VERSION        = 1;
        const unsigned int CONNECTIVITY_MESSAGE_VERSION = 1;
        const unsigned int DATA_POINT_MESSAGE_VERSION   = 1;
        const unsigned int NODE_PROPERTIES_MESSAGE_VERSION = 1;

        const unsigned int SEEK_MESSAGE_VERSION         = 1;
        const unsigned int START_MESSAGE_VERSION        = 1;
        const unsigned int STOP_MESSAGE_VERSION         = 1;
        const unsigned int SPEED_MESSAGE_VERSION        = 1;
        const unsigned int NODE_STATUS_MESSAGE_VERSION  = 1;
        const unsigned int PLAYBACK_TIME_RANGE_MESSAGE_VERSION = 1;
        const unsigned int MESSAGE_STREAM_FILTER_MESSAGE_VERSION = 1;
	const unsigned int SUBSCRIBE_STREAM_MESSAGE_VERSION = 1;
	const unsigned int STREAM_DESCRIPTION_MESSAGE_VERSION = 1;
	const unsigned int LIST_STREAMS_MESSAGE_VERSION = 1;

        /**
         * GUI bits in the watcher have a concept of a layer which can be turned on or off.
         *
         * These are default layers, but the name if a layer is not contrained by these strings
         * and anyone that uses them should be able to handle any string value as a valid layer. 
         * The layers are dyanmic: a feeder can specify any layer it wants during runtime and the 
         * GUIs should be able to handle it. 
         *
         * The static const strings below are all the layers from the legacy watcher. 
         */
        typedef std::string GUILayer;
        static const GUILayer UNDEFINED_LAYER = "Undefined";
        static const GUILayer PHYSICAL_LAYER = "Physical";
        static const GUILayer HIERARCHY_LAYER = "Hierarchy"; 
        static const GUILayer BANDWIDTH_LAYER = "Bandwidth"; 
        static const GUILayer ROUTING_LAYER = "Routing"; 
        static const GUILayer ONE_HOP_ROUTING_LAYER = "One_Hop_Routing"; 
        static const GUILayer ANTENNARADIUS_LAYER = "Antenna_Radius"; 
        static const GUILayer SANITY_CHECK_LAYER = "Sanity_Check"; 
        static const GUILayer ANOMPATHS_LAYER = "Anomolus_Paths"; 
        static const GUILayer CORROLATION_LAYER = "Correlation"; 
        static const GUILayer ALERT_LAYER = "Alerts"; 
        static const GUILayer CORROLATION_3HOP_LAYER = "Three_Hop_Correlation"; 
        static const GUILayer ROUTING2_LAYER = "Wormhole Routing"; 
        static const GUILayer ROUTING2_ONE_HOP_LAYER = "Wormhole_One_Hop_Routing"; 
        static const GUILayer FLOATING_GRAPH_LAYER = "Floating_Graph"; 
        static const GUILayer NORMAL_PATHS_LAYER = "Normal_Paths"; 

        /** Determine if the specified message type is a feeder API event.
         * @retval true if the message type is a feeder api event
         * @retval false otherwise
         */
        inline bool isFeederEvent(MessageType t) { return t < SEEK_MESSAGE_TYPE; }

        /** Determine if the message references a layer or not.
         * @retval true if it does and the layer is put in 'layer'
         * @retval false if it does not.
         */
        bool hasLayer(MessagePtr m, GUILayer &layer);
    }
}

#endif // MESSAGETYPES_AND_VERSIONS_H
