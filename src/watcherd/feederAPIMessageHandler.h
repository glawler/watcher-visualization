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

#ifndef CLIENT_MESSAGE_HANDLER_H
#define CLIENT_MESSAGE_HANDLER_H

#include "messageHandler.h"

namespace watcher 
{
    /// base class for hierarchy of event handlers for message sent via feeder daemons
    class FeederAPIMessageHandler : 
        public MessageHandler
    {
        public:
            FeederAPIMessageHandler();
            virtual ~FeederAPIMessageHandler(); 

            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);
            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages); 

            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<FeederAPIMessageHandler> FeederAPIMessageHandlerPtr;

} // namespace 

#endif //  CLIENT_MESSAGE_HANDLER_H

