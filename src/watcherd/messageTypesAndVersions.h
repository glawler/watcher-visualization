#ifndef MESSAGETYPES_AND_VERSIONS_H
#define MESSAGETYPES_AND_VERSIONS_H

#include <iostream>

namespace watcher
{
    typedef enum 
    {
        UNKNOWN_MESSAGE_TYPE      = 0,
        MESSAGE_STATUS_TYPE       = 1,
        TEST_MESSAGE_TYPE         = 2
    } MessageType;

    const unsigned int BASE_MESSAGE_VERSION      = 1;
    const unsigned int MESSAGE_STATUS_VERSION    = 1;
    const unsigned int MESSAGE_TEST_VERSION      = 1;

    std::ostream &operator<<(std::ostream &out, const MessageType &type);
}

#endif // MESSAGETYPES_AND_VERSIONS_H
