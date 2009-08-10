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

/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/foreach.hpp>

#include "writeDBMessageHandler.h"
#include "database.h"
#include "libwatcher/connection.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WriteDBMessageHandler, "MessageHandler.WriteDBMessageHandler");

bool WriteDBMessageHandler::handleMessageArrive(ConnectionPtr, const MessagePtr& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    assert(isFeederEvent(msg->type)); // only store feeder events
    store_event(msg); // using the database handle for this thread.

    TRACE_EXIT_RET(ret);
    return ret;
}

bool WriteDBMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<MessagePtr>& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    BOOST_FOREACH(MessagePtr m, msg) {
        ret |= handleMessageArrive(conn, m);
    }

    TRACE_EXIT_RET(ret);
    return ret;
}
