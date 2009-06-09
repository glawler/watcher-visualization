#include <boost/cast.hpp>

#include "watcherdAPIMessageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

namespace watcher {
    INIT_LOGGER(WatcherdAPIMessageHandler, "MessageHandler.WatcherdAPIMessageHandler");
}

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

bool WatcherdAPIMessageHandler::handleMessageArrive(ConnectionPtr conn, const MessagePtr &message)
{
    TRACE_ENTER();

    // Log message arrival
    bool rv = MessageHandler::handleMessageArrive(conn, message); 

    TRACE_EXIT_RET_BOOL(rv);
    return true;
}

// virtual 
bool WatcherdAPIMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    bool rv = false;
    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        rv |= handleMessageArrive(conn, *i);

    TRACE_EXIT_RET_BOOL(rv);
    return rv;
}

bool WatcherdAPIMessageHandler::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log the message. 
    bool rv = MessageHandler::handleMessageSent(message); 

    TRACE_EXIT_RET_BOOL(rv);
    return rv;
}

// virtual 
bool WatcherdAPIMessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    bool rv = false;
    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        rv |= handleMessageSent(*i);

    TRACE_EXIT_RET_BOOL(rv);
    return rv;
}
