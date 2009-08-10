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

#ifndef GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H
#define GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H

#include "libwatcher/messageHandler.h"

namespace watcher 
{
    /// Base class for hierarchy of classes which implement handlers for events originating from GUI or feeder clients.
    class ServerMessageHandler : public MessageHandler
    {
        public:
            ServerMessageHandler();
            virtual ~ServerMessageHandler(); 

            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);

            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages);

            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<ServerMessageHandler> ServerMessageHandlerPtr;

} // namespace 

#endif //  GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H
