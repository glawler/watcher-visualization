//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_SERVER_HPP
#define HTTP_SERVER3_SERVER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include "serverConnection.h"
#include "logger.h"

namespace watcher 
{
    /// The top-level class of a watcher Server.
    class Server : private boost::noncopyable
    {
        public:
            /// Construct the Server to listen on the specified TCP address and port, and
            /// serve up files from the given directory.
            explicit Server(
                    const std::string& address, 
                    const std::string& port,
                    // boost::shared_ptr<request_handler> &handler,
                    std::size_t thread_pool_size);

            /// Run the Server's io_service loop.
            void run();

            /// Stop the Server.
            void stop();

        private:
            DECLARE_LOGGER();

            /// Handle completion of an asynchronous accept operation.
            void handle_accept(const boost::system::error_code& e);

            /// The number of threads that will call io_service::run().
            std::size_t thread_pool_size_;

            /// The io_service used to perform asynchronous operations.
            boost::asio::io_service io_service_;

            /// Acceptor used to listen for incoming connections.
            boost::asio::ip::tcp::acceptor acceptor_;

            /// The next connection to be accepted.
            ServerConnectionPtr new_connection_;
    };

    typedef boost::shared_ptr<Server> ServerPtr;

} // namespace watcher

#endif // HTTP_SERVER3_SERVER_HPP
