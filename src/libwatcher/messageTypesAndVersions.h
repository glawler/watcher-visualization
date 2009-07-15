/**
 * @file messageTypesAndVersions.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef MESSAGETYPES_AND_VERSIONS_H
#define MESSAGETYPES_AND_VERSIONS_H

#include <ostream>

namespace watcher {
    namespace event {
        //
        // An enum which contains all possible messages. 
        //
        typedef enum 
        {
            // feeder API messages
            UNKNOWN_MESSAGE_TYPE      = 0x0,
            MESSAGE_STATUS_TYPE       = 0x1,
            TEST_MESSAGE_TYPE         = 0x2,
            GPS_MESSAGE_TYPE          = 0x3,
            LABEL_MESSAGE_TYPE        = 0x4,
            EDGE_MESSAGE_TYPE         = 0x5,
            COLOR_MESSAGE_TYPE        = 0x6,
            CONNECTIVITY_MESSAGE_TYPE = 0x7,
            NODE_STATUS_MESSAGE_TYPE  = 0x8,
            DATA_POINT_MESSAGE_TYPE   = 0x9,

            // watcherdAPI messages
            SEEK_MESSAGE_TYPE         = 0xA, // DO NOT REORDER THIS WITHOUT CHANGING isFeederEvent
            START_MESSAGE_TYPE        = 0xB,
            STOP_MESSAGE_TYPE         = 0xC,
            SPEED_MESSAGE_TYPE        = 0xD,
            PLAYBACK_TIME_RANGE_MESSAGE_TYPE = 0xE,

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

        const unsigned int SEEK_MESSAGE_VERSION         = 1;
        const unsigned int START_MESSAGE_VERSION        = 1;
        const unsigned int STOP_MESSAGE_VERSION         = 1;
        const unsigned int SPEED_MESSAGE_VERSION        = 1;
        const unsigned int NODE_STATUS_MESSAGE_VERSION  = 1;
        const unsigned int PLAYBACK_TIME_RANGE_MESSAGE_VERSION = 1;

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
    }
}

#endif // MESSAGETYPES_AND_VERSIONS_H
