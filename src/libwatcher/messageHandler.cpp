#include <boost/cast.hpp>

#include "messageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(MessageHandler, "MessageHandler");

MessageHandler::MessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

MessageHandler::~MessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}


bool MessageHandler::handleMessageArrive(ConnectionPtr, const MessagePtr &message)
{
    TRACE_ENTER();
    LOG_INFO("Recv'd message: " << *message); 
    TRACE_EXIT_RET(false);
    return false;
}

bool MessageHandler::handleMessagesArrive(ConnectionPtr conn, const vector<MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(handleMessageArrive(conn, *i))
        {
            TRACE_EXIT_RET("true"); 
            return true;
        }
    }

    TRACE_EXIT_RET("false");
    return false;
}

// virtual 
bool MessageHandler::handleMessageSent(const event::MessagePtr &message)
{
    TRACE_ENTER();
    LOG_INFO("Sent message: " << *message); 
    TRACE_EXIT_RET(false);
    return false;
}

// virtual 
bool MessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(handleMessageSent(*i))
        {
            TRACE_EXIT_RET("true"); 
            return true;
        }
    }

    TRACE_EXIT_RET("false");
    return false;
}

