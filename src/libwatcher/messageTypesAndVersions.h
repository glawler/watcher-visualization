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
            UNKNOWN_MESSAGE_TYPE      = 0x0,
            MESSAGE_STATUS_TYPE       = 0x1,
            TEST_MESSAGE_TYPE         = 0x2,
            GPS_MESSAGE_TYPE          = 0x3,
            LABEL_MESSAGE_TYPE        = 0x4,
            EDGE_MESSAGE_TYPE         = 0x5,
            COLOR_MESSAGE_TYPE        = 0x6,
            DATA_REQUEST_MESSAGE_TYPE = 0x7,

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
        const unsigned int DATA_REQUEST_MESSAGE_VERSION = 1;

        //
        // GUI bits in the watcher have a concept of a layer which can be turned on or off.
        //
        typedef enum
        {
            HIERARCHY_LAYER         = 1L << 0, 
            ROUTING_LAYER           = 1L << 1,
            NODE_LAYER              = 1L << 2,
            FLOATING_LAYER          = 1L << 3
        } GUILayer;

        std::ostream&operator<< (std::ostream &out, const GUILayer &layer);
    }
}

#endif // MESSAGETYPES_AND_VERSIONS_H
