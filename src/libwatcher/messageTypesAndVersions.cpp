#include "messageTypesAndVersions.h"
#include "logger.h"

using namespace std;

namespace watcher {
    namespace event {
        ostream& operator<<(ostream &out, const MessageType &type)
        {
            TRACE_ENTER();

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
                case LABEL_MESSAGE_TYPE:
                    out << static_cast<int>(LABEL_MESSAGE_TYPE) << " (label)";
                    break;
                case EDGE_MESSAGE_TYPE:
                    out << static_cast<int>(EDGE_MESSAGE_TYPE) << " (edge)";
                    break;
                case COLOR_MESSAGE_TYPE:
                    out << static_cast<int>(COLOR_MESSAGE_TYPE) << " (color)";
                    break;
                case DATA_REQUEST_MESSAGE_TYPE:
                    out << static_cast<int>(DATA_REQUEST_MESSAGE_TYPE) << " (data request)";
                    break;
                case CONNECTIVITY_MESSAGE_TYPE:
                    out << static_cast<int>(CONNECTIVITY_MESSAGE_TYPE) << " (connectivity)";
                    break;

                case SEEK_MESSAGE_TYPE:
                    out << static_cast<int>(SEEK_MESSAGE_TYPE) << " (seek)";
                    break;
                case START_MESSAGE_TYPE:
                    out << static_cast<int>(START_MESSAGE_TYPE) << " (start)";
                    break;
                case STOP_MESSAGE_TYPE:
                    out << static_cast<int>(STOP_MESSAGE_TYPE) << " (stop)";
                    break;
                case SPEED_MESSAGE_TYPE:
                    out << static_cast<int>(SPEED_MESSAGE_TYPE) << " (speed)";
                    break;
                case NODE_STATUS_MESSAGE_TYPE:
                    out << static_cast<int>(NODE_STATUS_MESSAGE_TYPE) << " (node status)";
                    break;


                case USER_DEFINED_MESSAGE_TYPE: 
                    out << static_cast<int>(USER_DEFINED_MESSAGE_TYPE) << " (user defined)";
                    break;

                    // don't put default case so the (smart) compiler can
                    // tell us if we've not put a MessageType here....
            }
            out << "\"";

            TRACE_EXIT();
            return out;
        }
    }
}
