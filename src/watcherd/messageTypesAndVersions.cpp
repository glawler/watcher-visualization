#include "messageTypesAndVersions.h"
#include "logger.h"

using namespace std;
using namespace watcher;

ostream &watcher::operator<<(ostream &out, const MessageType &type)
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

            // don't put default case so the (smart) compiler can
            // tell us if we've not put a MessageType here....
    }
    out << "\"";

    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const GUILayer &layer)
{
    TRACE_ENTER();

    out << "\"";
    switch (layer)
    {

        case HIERARCHY_LAYER:
            out << static_cast<int>(HIERARCHY_LAYER) << " (hierachy layer)";
            break;
        case ROUTING_LAYER:
            out << static_cast<int>(ROUTING_LAYER) << " (routing layer)";
            break;
        case NODE_LAYER:
            out << static_cast<int>(NODE_LAYER) << " (node  layer)";
            break;
        case FLOATING_LAYER:
            out << static_cast<int>(FLOATING_LAYER) << " (floating layer)";
            break;

            // GTL - don't put default.
    }
    out << "\"";

    TRACE_EXIT();
    return out;
}
