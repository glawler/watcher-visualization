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

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "request_handler.hpp"

#include "dataMarshaller.hpp"
#include "message.h"

namespace watcher 
{
    /// Represents a single connection from a client.
    class serverConnection : 
        public boost::enable_shared_from_this<serverConnection>, 
        private boost::noncopyable
    {
        public:
            /// Construct a connection with the given io_service.
            explicit serverConnection(boost::asio::io_service& io_service, boost::shared_ptr<request_handler> handler);

            /// Get the socket associated with the connection.
            boost::asio::ip::tcp::socket& socket();

            /// Start the first asynchronous operation for the serverConnection.
            void start();

        private:
            DECLARE_LOGGER();

            /// Handle completion of a read operation.
            void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred);

            /// Handle completion of a write operation.
            void handle_write(const boost::system::error_code& e);

            /// Strand to ensure the connection's handlers are not called concurrently.
            boost::asio::io_service::strand strand_;

            /// Socket for the connection.
            boost::asio::ip::tcp::socket socket_;

            /// The handler used to process the incoming request.
            boost::shared_ptr<request_handler> request_handler_;

            /// Buffer for incoming data.
            boost::array<char, 8192> buffer_;

            /// Buffer for outgoing data
            std::vector<boost::asio::const_buffer> outboundDataBuffers;

            /// The incoming message.
            boost::shared_ptr<Message> request_;

            /// The reply to be sent back to the client.
            boost::shared_ptr<Message> reply_;

            // Use the utiliity class for arbitrary marshal/unmarhasl
            DataMarshaller dataMarshaller;
    };

    typedef boost::shared_ptr<serverConnection> server_connection_ptr;

} // namespace http

#endif // WATCHER_SERVER_CONNECTION
