#include "messageTypesAndVersions.h"

using namespace std;
using namespace watcher;

ostream &watcher::operator<<(ostream &out, const MessageType &type)
{
    out << "\"";
    switch(type)
    {
        case UNKNOWN_MESSAGE_TYPE: 
            out << "Unknown (" << static_cast<int>(UNKNOWN_MESSAGE_TYPE) << ")"; 
            break;
        case MESSAGE_STATUS_TYPE: 
            out << "Status (" << static_cast<int>(MESSAGE_STATUS_TYPE) << ")";
            break;
        case TEST_MESSAGE_TYPE: 
            out << "Test (" << static_cast<int>(TEST_MESSAGE_TYPE) << ")";
            break;
        default: 
            out << "Error - undefined message type"; 
    }
    out << "\"";
    return out;
}
