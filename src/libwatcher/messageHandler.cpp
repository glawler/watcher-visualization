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


bool MessageHandler::handleMessageArrive(ConnectionPtr, const MessagePtr &message)
{
    TRACE_ENTER();
    LOG_INFO("Recv'd message: " << *message); 
    TRACE_EXIT_RET(false);
    return false;
}

bool MessageHandler::handleMessagesArrive(ConnectionPtr conn, const vector<MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(handleMessageArrive(conn, *i))
        {
            TRACE_EXIT_RET("true"); 
            return true;
        }
    }

    TRACE_EXIT_RET("false");
    return false;
}

// virtual 
bool MessageHandler::handleMessageSent(const event::MessagePtr &message)
{
    TRACE_ENTER();
    LOG_INFO("Sent message: " << *message); 
    TRACE_EXIT_RET(false);
    return false;
}

// virtual 
bool MessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
    {
        if(handleMessageSent(*i))
        {
            TRACE_EXIT_RET("true"); 
            return true;
        }
    }

    TRACE_EXIT_RET("false");
    return false;
}

