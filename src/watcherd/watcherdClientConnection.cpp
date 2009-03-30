/** 
 * @file watcherClientConnection.cpp
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-25
 */

#include "watcherdClientConnection.h"

using namespace watcher;

INIT_LOGGER(WatcherdClientConnection, "ClientConnection.WatcherdClientConnection");


WatcherdClientConnection::WatcherdClientConnection(
        boost::asio::io_service& io_service, 
        const std::string &server_, 
        const std::string &service_) : 
    ClientConnection(io_service, server_, service_)
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

    TRACE_EXIT_RET( (retVal==true?"true":"false") );
    return retVal;
}

