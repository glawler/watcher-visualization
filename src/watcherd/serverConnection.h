/* Copyright 2009,2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef WATCHER_SERVER_CONNECTION
#define WATCHER_SERVER_CONNECTION

#include <list>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "libwatcher/message.h"
#include "libwatcher/connection.h"
#include "libwatcher/dataMarshaller.h"
#include "libwatcher/messageStreamFilter.h"

#include "watcherd_fwd.h"
#include "serverConnectionFwd.h"
#include "sharedStreamFwd.h"

namespace watcher 
{
    /// Represents a single connection from a client.
    class ServerConnection : 
        public Connection,
        public boost::enable_shared_from_this<ServerConnection> 
    {
        public:
            /// Construct a connection with the given io_service.
            explicit ServerConnection(Watcherd&, boost::asio::io_service& io_service);
            ~ServerConnection();

            /// Start the first asynchronous operation for the ServerConnection.
            void run();

            /// Send a message(s) to this client
            void sendMessage(event::MessagePtr);
            void sendMessage(const std::vector<event::MessagePtr>&);

            /// get the io_service associated with this connection
            boost::asio::io_service& io_service() { return io_service_; }

            /** Get the Watcherd instance associated with this connection. */
            Watcherd& watcherd() { return watcher; }

        private:

            DECLARE_LOGGER();

            /// Handle completion of a read operation.
            void handle_read_header(
                    const boost::system::error_code& e, 
                    size_t bytes_transferred);

            void handle_read_payload(
                    const boost::system::error_code& e, 
                    size_t bytes_transferred, 
                    unsigned short messageNum); 

            /// Handle completion of a write operation.
            void handle_write(const boost::system::error_code& e, event::MessagePtr reply);

            void read_error(const boost::system::error_code &e);

            bool dispatch_gui_event(event::MessagePtr &);
            void filter(event::MessagePtr& m);

            Watcherd& watcher;
            boost::asio::io_service& io_service_;

            /// Strand to ensure the connection's handlers are not called concurrently.
            boost::asio::io_service::strand strand_;
            /// Strand for write operations
            boost::asio::io_service::strand write_strand_;

            /// Buffer for incoming data.
            typedef boost::array<char, 32768> IncomingBuffer;
            IncomingBuffer incomingBuffer;

            /// What type of connection is this?
            enum connection_type { unknown, feeder, gui };
            connection_type conn_type;

            /// If needed, a network address to map incoming message IDs with.
            boost::asio::ip::address_v4 dataNetwork;

            typedef std::list<MessageStreamFilter> MessageStreamFilterList;
            MessageStreamFilterList messageStreamFilters;
            bool messageStreamFilterEnabled; 

	    SharedStreamPtr stream;
	    void seek(event::MessagePtr& m);
	    void start(event::MessagePtr&);
	    void stop(event::MessagePtr&);
	    void speed(event::MessagePtr& m);
	    void range(event::MessagePtr& m);
	    void subscribeToStream(MessagePtr&);
	    void description(MessagePtr&);
	    void listStreams(MessagePtr&);
    };

} // namespace

#endif // WATCHER_SERVER_CONNECTION
