/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

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

bool ServerMessageHandler::handleMessageArrive(ConnectionPtr conn, const MessagePtr &message)
{
    TRACE_ENTER();

    MessageHandler::handleMessageArrive(conn, message);

    // Server always keeps the connection open - client closes it when it is through with it.
    TRACE_EXIT_RET("false");
    return false;
}

bool ServerMessageHandler::handleMessagesArrive(ConnectionPtr conn, const vector<MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(handleMessageArrive(conn, *i))
        {
            // This means if any message is a feeder api message, we cut the connection.
            // This may or may not be correct.
            TRACE_EXIT_RET("true"); 
            return true;
        }
    }

    TRACE_EXIT_RET("false");
    return false;
}

bool ServerMessageHandler::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log the message. 
    MessageHandler::handleMessageSent(message); 

    TRACE_EXIT_RET("false");
    return false;
}

// virtual 
bool ServerMessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageSent(*i);

    // Server currently never wants a response, but does want the connection to remain open.
    TRACE_EXIT_RET("false");
    return false;
}


