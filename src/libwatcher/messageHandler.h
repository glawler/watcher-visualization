/**
 * @file messageHandler.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef MESSSAGE_HADNERLS_H
#define MESSSAGE_HADNERLS_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "libwatcher/message.h"
#include "libwatcher/connection_fwd.h"

namespace watcher 
{
    /** Base class for hierarchy of classes that implement callback handlers when specific
     * events arrive.  */
    class MessageHandler : public boost::noncopyable
    {
        public:
            MessageHandler();

            virtual ~MessageHandler(); 

            /**
             * Notification of message arrival. Default does nothing, but log the message. 
             * Classes which derive from this class should handle the message arrival and if
             * they want to respond to the message, make 'response' non-null on return.
             * @param[in] conn the connection from which the message was received
             * @param[in] message the newly arrived message. 
             * @retval false, keep connection open,
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

             * @retval true otherwise.
             */
            virtual bool handleMessageArrive(ConnectionPtr conn, const event::MessagePtr &message);

            /**
             * Notification of messages arrival. Default does nothing, but log the messages. 
             * Classes which derive from this class should handle the message arrival and if
             * they want to respond to the message, make 'response' non-null on return.
             * @param[in] conn the connection from which the message was received
             * @param[in] messages the newly arrived messages. 
             * @retval false, keep connection open,
             * @retval true otherwise.
             */
            virtual bool handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr> &messages);

            /**
             * Notification that a message has been successfully sent.
             *
             * @param[in] message that was sent
             * @return boolean. If true, shutdown the write half of the socket, flase otherwise
             */
            virtual bool handleMessageSent(const event::MessagePtr &message); 

            /**
             * Notification that messages have been successfully sent.
             *
             * @param[in] messages the messages that were sent
             * @return If true, shutdown the write half of the socket, flase otherwise
             */
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr;

} // namespace 

#endif // MESSSAGE_HADNERLS_H
