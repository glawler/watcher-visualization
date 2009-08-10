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

#include "feederAPIMessageHandler.h"
#include <libwatcher/messageStatus.h>

using namespace std; 
using namespace watcher;
using namespace watcher::event;
using namespace boost;

INIT_LOGGER(FeederAPIMessageHandler, "MessageHandler.FeederAPIMessageHandler");

FeederAPIMessageHandler::FeederAPIMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

FeederAPIMessageHandler::~FeederAPIMessageHandler()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool FeederAPIMessageHandler::handleMessageArrive(ConnectionPtr conn, const MessagePtr &message)
{
    TRACE_ENTER();
    bool retVal=MessageHandler::handleMessageArrive(conn, message); 
    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

// virtual 
bool FeederAPIMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    LOG_WARN("Feeder API got a response - this generally should not happen."); 

    // GTL - maybe assert here or do something more aggressive.

    // Log the message
    MessageHandler::handleMessagesArrive(conn, messages);

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageArrive(conn, *i);

    // Feeder API currently never wants a response - so always return false
    TRACE_EXIT_RET("false");
    return false;
}

bool FeederAPIMessageHandler::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    // Log the message. 
    MessageHandler::handleMessageSent(message); 

    TRACE_EXIT_RET("false");
    return false;
}

// virtual 
bool FeederAPIMessageHandler::handleMessagesSent(const std::vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    for(vector<MessagePtr>::const_iterator i=messages.begin(); i!=messages.end(); ++i)
        handleMessageSent(*i);

    // Feeder API currently never wants a response - so always return false
    TRACE_EXIT_RET("false");
    return false;
}
