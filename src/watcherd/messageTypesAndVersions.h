#ifndef MESSAGETYPES_AND_VERSIONS_H
#define MESSAGETYPES_AND_VERSIONS_H

#include <iostream>

namespace watcher
{
    typedef enum 
    {
        UNKNOWN_MESSAGE_TYPE      = 0,
        MESSAGE_STATUS_TYPE       = 1,
        TEST_MESSAGE_TYPE         = 2,
        GPS_MESSAGE_TYPE          = 3

    } MessageType;

    // version numbers are on a per message format basis
    // I don't know that these will ever really change.
    const unsigned int BASE_MESSAGE_VERSION      = 1;
    const unsigned int MESSAGE_STATUS_VERSION    = 1;
    const unsigned int MESSAGE_TEST_VERSION      = 1;
    const unsigned int GPS_MESSAGE_VERSION       = 1;

    std::ostream &operator<<(std::ostream &out, const MessageType &type);
}

#endif // MESSAGETYPES_AND_VERSIONS_H
