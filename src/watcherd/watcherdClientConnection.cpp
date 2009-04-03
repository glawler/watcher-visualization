/** 
 * @file watcherClientConnection.cpp
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-25
 */

#include "watcherdClientConnection.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherdClientConnection, "ClientConnection.WatcherdClientConnection");


WatcherdClientConnection::WatcherdClientConnection(
        WatcherdClientMessageHandlerPtr messageHandler_,
        boost::asio::io_service& io_service, 
        const std::string &server_, 
        const std::string &service_) : 
    ClientConnection(io_service, server_, service_),
    messageArrivalHandler(messageHandler_)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherdClientConnection::~WatcherdClientConnection()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual 
bool WatcherdClientConnection::messageArrive(const event::Message &message)
{
    TRACE_ENTER();
    bool retVal=false;

    LOG_INFO("Got message from server in WatcherdClientConnection: " << message); 

    if(messageArrivalHandler)
    {
        LOG_DEBUG("Sending new message to message arrival handler"); 
        MessagePtr messPtr(new Message(message)); 
        messageArrivalHandler->messageArrived(messPtr); 
    }
    else
    {
        LOG_DEBUG("No message arrival handler - doing nothing with new message"); 
    }

    TRACE_EXIT_RET( (retVal==true?"true":"false") );
    return retVal;
}

