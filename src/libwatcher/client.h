/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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
/**
 * @file client.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_CLIENT_HPP
#define WATCHER_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "declareLogger.h"

#include "libwatcher/message_fwd.h"

namespace watcher 
{
    class ClientConnection;
    typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr; 

    class MessageHandler;
    typedef boost::shared_ptr<MessageHandler> MessageHandlerPtr; 

    /// Currently it's just a thin wrapper around the clientConnection class. 
    class Client : private boost::noncopyable
    {
        public:
            /**
             * Construct the Client to connect to the specified hostname and service.
             * to send messages. 
	     * If service is not specified, server may be of the form "host:service", and
	     * otherwise the default service name "watcherd" is used.
	     * If hostname is empty (server is "" or ":service"), "localhost" is used.
             * @param[in] server the host to connect to
             * @param[in] service the service/port on the server
             */
            explicit Client(
                    const std::string& server, 
                    const std::string& service="");

            virtual ~Client();

            /**
             * Send a messsage to the server.
             * 
             * @param message the message to be sent. 
             * @return boolean true on success, false otherwise
             */
            bool sendMessage(const event::MessagePtr message);

            /**
             * Send messsages to the server.
             * 
             * @param messages the messages to be sent. 
             * @return boolean true on success, false otherwise
             */
            bool sendMessages(const std::vector<event::MessagePtr> &messages);

            /**
             * setMessageHandler() Set a messageHandler if you want direct access to the 
             * responses sent via sendMessage().
             * @param messageHandler - an instance of a message handler class. 
             */
            void addMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * Does what it says on the tin.
             */
            void removeMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * wait until all work is done in the client.
             */
            void wait();

            /**
             * Perform a synchronous connection attempt to the server.
             * @retval true connection succeeded
             * @retval false connection failed
             */
            bool connect(bool async=false);

            /**
             * @return true if connected, false otherwise.
             */
            bool connected() const;

            /**
             * Close the connection.
             */
            void close(); 

        protected:
        private:

            DECLARE_LOGGER();

            boost::asio::io_service ioService;

            std::string server;
            std::string service;

            ClientConnectionPtr clientConnection;

            boost::thread *workThread;
            boost::asio::io_service::work *work;
    };

    typedef boost::shared_ptr<Client> ClientPtr;

} // namespace watcher

#endif // WATCHER_CLIENT_HPP

// vim:sw=4
