#include "logger.h"
#include "connection.h"

using namespace watcher;
using namespace boost;
using namespace boost::asio;

INIT_LOGGER(Connection, "Connection"); 

Connection::Connection(boost::asio::io_service &io_service) :
    theSocket(io_service)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

//virtual 
Connection::~Connection()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

Connection::ConnectionSocket &Connection::getSocket()
{
    TRACE_ENTER();
    TRACE_EXIT();
    return theSocket;
}

void Connection::addMessageHandler(MessageHandlerPtr messageHandler)
{
    TRACE_ENTER();

    LOG_DEBUG("Adding message handler to connection."); 
    messageHandlers.push_back(messageHandler); 

    TRACE_EXIT();
}

void Connection::removeMessageHandler(MessageHandlerPtr messageHandler)
{
    TRACE_ENTER();

    LOG_DEBUG("Removing messageHandler from connection."); 
    MessageHandlerList::iterator removeMe;

    removeMe=find(messageHandlers.end(), messageHandlers.begin(), messageHandler);

    if (removeMe!=messageHandlers.end())
        messageHandlers.erase(removeMe);

    TRACE_EXIT();
}

