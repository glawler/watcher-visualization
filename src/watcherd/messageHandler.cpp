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


bool MessageHandler::handleMessageArrive(const MessagePtr &message, MessagePtr &response)
{
    TRACE_ENTER();
    LOG_INFO("Recv'd message: " << *message); 
    TRACE_EXIT_RET(true);
    return true;
}

bool MessageHandler::handleMessagesArrive(const vector<MessagePtr> &messages, MessagePtr &response)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(!handleMessageArrive(*i, response))
        {
            TRACE_EXIT_RET("false"); 
            return false;
        }
    }

    TRACE_EXIT_RET("true");
    return true;
}

