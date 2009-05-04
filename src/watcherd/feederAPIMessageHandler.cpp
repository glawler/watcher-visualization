#include <boost/cast.hpp>

#include "feederAPIMessageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(FeederAPIMessageHandler, "MessageHandler.FeederAPIMessageHandler");

FeederAPIMessageHandler::FeederAPIMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

FeederAPIMessageHandler::~FeederAPIMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool FeederAPIMessageHandler::handleMessageArrive(ConnectionPtr conn, const MessagePtr &message)
{
    TRACE_ENTER();
    bool retVal=MessageHandler::handleMessageArrive(conn, message); 
    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

// virtual 
bool FeederAPIMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    LOG_WARN("Feeder API got a response - this generally should not happen."); 

    // GTL - maybe assert here or do something more aggressive.

    // Log the message
    MessageHandler::handleMessagesArrive(conn, messages);

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageArrive(conn, *i);

    // Feeder API currently never wants a response - so always return false
    TRACE_EXIT_RET("false");
    return false;
}

bool FeederAPIMessageHandler::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log the message. 
    MessageHandler::handleMessageSent(message); 

    TRACE_EXIT_RET("false");
    return false;
}

// virtual 
bool FeederAPIMessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageSent(*i);

    // Feeder API currently never wants a response - so always return false
    TRACE_EXIT_RET("false");
    return false;
}
