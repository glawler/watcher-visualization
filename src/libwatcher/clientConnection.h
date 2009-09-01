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

/**
 * @file clientConnection.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
/**
 * @file message.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHERD_CLIENT_CONECTION_HPP
#define WATCHERD_CLIENT_CONECTION_HPP

#include <list>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "dataMarshaller.h"
#include "connection.h"

#include "libwatcher/message.h"

namespace watcher 
{
    /// Represents a single connection from a client.
    class ClientConnection : 
        public Connection,
        public boost::enable_shared_from_this<ClientConnection>
    {
        public:
            /** Connect to the service service on server server using the boost::io_service given.
             * @param io_service io_service for the application
             * @param server DNS name or IP address of server
             * @param service port or service name
             * @param reconnect attempt to reconnect if the connection terminates
             */
            ClientConnection(
                    boost::asio::io_service& io_service, 
                    const std::string &server, 
                    const std::string &service,
                    bool reconnect = false);

            virtual ~ClientConnection(); 

            /**
             * send a packet to the server which contains the message. 
             *
             * @param message The message to send.
             * @return a bool currently ignored. 
             */
            bool sendMessage(const event::MessagePtr message);

            /**
             * send a packet to the server which contains messages. 
             * @param message The messages to send.
             * @return a bool currently ignored. 
             */
            bool sendMessages(const std::vector<event::MessagePtr> &message);

            /**
             * Perform a synchronous connection attempt to the server.
             * @retval true connection succeeded
             * @retval false connection failed
             */
            bool connect();

            /**
             * close the connection to the server.
             */
            void close(); 

        private:

            DECLARE_LOGGER();

            void doClose();
            /** 
             * Connect to the server. This function will not return until connected.
             * Will loop until connected, trying every X seconds.
             */
            void doConnect(); 

            /**
             * Attempt connection to server. Will set 'connected' to true, if successful. Will
             * also return true on success.
             * @return true on success, false otherwise.
             */
            bool tryConnect(); 

            bool connected; 

            boost::asio::io_service &ioService;
            boost::asio::io_service::strand theStrand; // for reading
            boost::asio::io_service::strand writeStrand;

            typedef boost::array<char, 32768> IncomingBuffer;
            IncomingBuffer incomingBuffer;

            void handle_write_message(const boost::system::error_code& e, std::vector<event::MessagePtr> messages);
            void handle_read_header(const boost::system::error_code& e, std::size_t bytes_transferred);
            void handle_read_payload(const boost::system::error_code& e, std::size_t bytes_transferred);
            void run();

            std::string server;
            std::string service;
            bool reconnect;
    };

    typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr;

} // namespace watcher

#endif // WATCHERD_CLIENT_CONECTION_HPP
