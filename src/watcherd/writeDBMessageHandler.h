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

/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef writedb_message_handler_h
#define writedb_message_handler_h

#include <string>

#include "libwatcher/messageHandler.h"

namespace watcher
{
    /** Class implementing the interface to the event database.
     * @author Michael.Elkins@cobham.com
     * @date 2009-05-04
     */
    class WriteDBMessageHandler : public MessageHandler
    {
        public:
            bool handleMessageArrive(ConnectionPtr, const event::MessagePtr&);
            bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr>&);

        private:
            DECLARE_LOGGER();
    };

} //namespace

#endif /* writedb_message_handler_h */
