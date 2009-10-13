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

#include "logger.h"
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
    MessageHandler::handleMessageArrive(conn, message); 

    TRACE_EXIT_RET_BOOL(false);
    return false;
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
