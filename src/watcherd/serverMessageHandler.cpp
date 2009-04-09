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

bool ServerMessageHandler::handleMessageArrive(const MessagePtr &message, MessagePtr &response)
{
    TRACE_ENTER();

    MessageHandler::handleMessageArrive(message, response); 
    response=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));

    TRACE_EXIT_RET("true"); 
    return true;
}

bool ServerMessageHandler::handleMessagesArrive(const vector<MessagePtr> &messages, MessagePtr &response)
{
    TRACE_ENTER();

    MessageHandler::handleMessagesArrive(messages, response); 

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(!handleMessageArrive(*i, response))  // may be overwriting responses from earlier loop iterations...
        {
            response=MessageStatusPtr(new MessageStatus(MessageStatus::status_error));
            TRACE_EXIT_RET("false"); 
            return false;
        }
    }

    TRACE_EXIT_RET("true");
    return true;
}

