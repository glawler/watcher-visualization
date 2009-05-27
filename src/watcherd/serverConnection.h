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

#include "dataMarshaller.h"
#include "connection.h"
#include "watcherd_fwd.h"
#include "serverConnectionFwd.h"
#include "replayState.h"

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

        protected:
            // 
            //
        private:
            void start();

            DECLARE_LOGGER();

            /// Handle completion of a read operation.
            void handle_read_header(
                    const boost::system::error_code& e, 
                    std::size_t bytes_transferred);

            void handle_read_payload(
                    const boost::system::error_code& e, 
                    std::size_t bytes_transferred, 
                    unsigned short messageNum); 

            /// Handle completion of a write operation.
            void handle_write(const boost::system::error_code& e, event::MessagePtr reply);

            Watcherd& watcher;
            boost::asio::io_service& io_service_;

            /// Strand to ensure the connection's handlers are not called concurrently.
            boost::asio::io_service::strand strand_;
            /// Strand for write operations
            boost::asio::io_service::strand write_strand_;

            /// Buffer for incoming data.
            typedef boost::array<char, 8192> IncomingBuffer;
            IncomingBuffer incomingBuffer;

            /// What type of connection is this?
            enum connection_type { unknown, feeder, gui };
            connection_type conn_type;

            /// state variables for Live and Replay tracking
            boost::shared_ptr<ReplayState> replay;
            bool isPlaying_;
            bool isLive_;
    };

} // namespace http

#endif // WATCHER_SERVER_CONNECTION
