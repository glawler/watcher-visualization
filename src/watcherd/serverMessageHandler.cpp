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

bool ServerMessageHandler::handleMessageArrive(const MessagePtr message, MessagePtr &response)
{
    TRACE_ENTER();

    MessageHandler::handleMessageArrive(message, response); 

    // Server currently handles a message just by acking it.
    response=MessageStatusPtr(new MessageStatus(MessageStatus::status_ack));
    LOG_DEBUG("Created ACK message to respond:" << *response); 
    TRACE_EXIT_RET(response);
    return response;
}

