#include "messageTypesAndVersions.h"

using namespace std;
using namespace watcher;

ostream &watcher::operator<<(ostream &out, const MessageType &type)
{
    out << "\"";
    switch(type)
    {
        case UNKNOWN_MESSAGE_TYPE: 
            out << static_cast<int>(UNKNOWN_MESSAGE_TYPE) << " (unknown)"; 
            break;
        case MESSAGE_STATUS_TYPE: 
            out << static_cast<int>(MESSAGE_STATUS_TYPE) << " (status)";
            break;
        case TEST_MESSAGE_TYPE: 
            out << static_cast<int>(TEST_MESSAGE_TYPE) << " (test)"; 
            break;
        case GPS_MESSAGE_TYPE: 
            out << static_cast<int>(GPS_MESSAGE_TYPE) << " (gps)";
            break;
        default: 
            out << "Error - undefined message type"; 
    }
    out << "\"";
    return out;
}