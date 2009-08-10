/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

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

