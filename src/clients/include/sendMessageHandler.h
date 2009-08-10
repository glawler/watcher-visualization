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

/** @file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-30
 */

#include "libwatcher/messageHandler.h"

namespace watcher {
    /** A handler to use in the case where a single message is sent to the server
     * and the connection should be closed after transmission.
     */
    class SendMessageHandler : public MessageHandler {
        public:
            /** Callback to cause the connection to terminate after sending a single
             * message to the server.
             * @param message the message that was sent
             * @retval true always returns true
             */
            bool handleMessageSent(const event::MessagePtr &message) { return true; }

            static MessageHandlerPtr create();
    };

    inline MessageHandlerPtr SendMessageHandler::create() {
        return MessageHandlerPtr(new SendMessageHandler());
    }
}
