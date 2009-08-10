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

#include "client.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(Client, "Client");

Client::Client(
        const std::string& server_, 
        const std::string& service_,
        bool reconnect_) :
    ioService(),
    server(server_),
    service(service_),
    clientConnection(new ClientConnection(ioService, server, service, reconnect_))
{
    TRACE_ENTER();
    
    // Yo trabajo sin reposo.
    work=new boost::asio::io_service::work(ioService);

    workThread=new boost::thread(boost::bind(&boost::asio::io_service::run, &ioService));
    TRACE_EXIT();
}

Client::~Client()
{
    TRACE_ENTER();
    if(work)
    {
        delete work;
        work=NULL; 
        workThread->join();
    }
    TRACE_EXIT();
}

bool Client::sendMessage(const MessagePtr request)
{
    TRACE_ENTER(); 

    bool retVal=clientConnection->sendMessage(request);

    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}

bool Client::sendMessages(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    bool retVal=clientConnection->sendMessages(messages);

    TRACE_EXIT_RET((retVal ? "true" : "false"));
    return retVal;
}

void Client::addMessageHandler(MessageHandlerPtr messageHandler)
{
    TRACE_ENTER();
    clientConnection->addMessageHandler(messageHandler); 
    TRACE_EXIT();
}

void Client::wait()
{
    TRACE_ENTER();

    // remove artifical work and wait for real work, if any, to complete.
    delete work; 
    work=NULL; 
    ioService.run(); 
    workThread->join();

    TRACE_EXIT(); 
}

bool Client::connect()
{
    TRACE_ENTER();
    bool rv = clientConnection->connect();
    TRACE_EXIT_RET_BOOL(rv);
    return rv;
}
