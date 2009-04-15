#include <boost/cast.hpp>

#include "libwatcher/messageStatus.h"
#include "serverMessageHandler.h"

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(ServerMessageHandler, "MessageHandler.ServerMessageHandler");

ServerMessageHandler::ServerMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

ServerMessageHandler::~ServerMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool ServerMessageHandler::handleMessageArrive(const MessagePtr &message)
{
    TRACE_ENTER();

    bool retVal=MessageHandler::handleMessageArrive(message);

    // If it's a feeder API message, close connection, otherwise keep open
    // to push out a message stream
    switch(message->type)
    {
        // feeder API messages - close connection
        case UNKNOWN_MESSAGE_TYPE:
        case MESSAGE_STATUS_TYPE:
        case TEST_MESSAGE_TYPE:
        case GPS_MESSAGE_TYPE:
        case LABEL_MESSAGE_TYPE:
        case EDGE_MESSAGE_TYPE:
        case COLOR_MESSAGE_TYPE:
        case DATA_REQUEST_MESSAGE_TYPE:
            retVal=false;
            break;

        // watcherdAPI messages - keep connection open
        case SEEK_MESSAGE_TYPE:
        case START_MESSAGE_TYPE:
        case STOP_MESSAGE_TYPE:
        case SPEED_MESSAGE_TYPE:
        case NODE_STATUS_MESSAGE_TYPE:
            retVal=true;
            break;

        // No idea.
        case USER_DEFINED_MESSAGE_TYPE:
            retVal=false;
            break;
    }

    TRACE_EXIT_RET((retVal?"true":"false"));
    return retVal;
}

bool ServerMessageHandler::handleMessagesArrive(const vector<MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(!handleMessageArrive(*i))
        {
            // This means if any message is a feeder api message, we cut the connection.
            // This may or may not be correct.
            TRACE_EXIT_RET("false"); 
            return false;
        }
    }

    TRACE_EXIT_RET("true");
    return true;
}

bool ServerMessageHandler::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log the message. 
    MessageHandler::handleMessageArrive(message); 

    TRACE_EXIT_RET("true");
    return true;
}

// virtual 
bool ServerMessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageSent(*i);

    // Server currently never wants a response, but does want the connection to remain open.
    TRACE_EXIT_RET("true");
    return true;
}


