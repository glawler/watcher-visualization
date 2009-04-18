#include <boost/cast.hpp>

#include "watcherdAPIMessageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(WatcherdAPIMessageHandler, "MessageHandler.WatcherdAPIMessageHandler");

WatcherdAPIMessageHandler::WatcherdAPIMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherdAPIMessageHandler::~WatcherdAPIMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool WatcherdAPIMessageHandler::handleMessageArrive(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log message arrival
    MessageHandler::handleMessageArrive(message); 

    TRACE_EXIT_RET("true"); 
    return true;
}

// virtual 
bool WatcherdAPIMessageHandler::handleMessagesArrive(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageArrive(*i);

    // Watcherd API currently always wants a response - so always return true
    TRACE_EXIT_RET("true");
    return true;
    TRACE_EXIT(); 
}

bool WatcherdAPIMessageHandler::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log the message. 
    MessageHandler::handleMessageArrive(message); 

    TRACE_EXIT_RET("true");
    return true;
}

// virtual 
bool WatcherdAPIMessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageSent(*i);

    // Watcherd API currently always wants a response - so always return true
    TRACE_EXIT_RET("true");
    return true;
}


