#ifndef MESSAGETYPES_AND_VERSIONS_H
#define MESSAGETYPES_AND_VERSIONS_H

#include <iostream>

namespace watcher
{
    typedef enum 
    {
        UNKNOWN_MESSAGE_TYPE      = 0,
        MESSAGE_HEADER,
        TEST_MESSAGE_TYPE
    } MessageType;

    const unsigned int BASE_MESSAGE_VERSION      = 1;
    const unsigned int TEST_MESSAGE_VERSION      = 1;

    std::ostream &operator<<(std::ostream &out, const MessageType &type);
}

#endif // MESSAGETYPES_AND_VERSIONS_H
