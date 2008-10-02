#ifndef WATCHER_SERVER_H
#define WATCHER_SERVER_H

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
            // Construct the Server to listen on the specified TCP address and port.
            explicit Server(
                    const std::string& address, 
                    const std::string& port,
                    std::size_t thread_pool_size,
                    MessageHandlerPtr messageHandler);

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

            MessageHandlerPtr messageHandler;
    };

    typedef boost::shared_ptr<Server> ServerPtr;

} // namespace watcher

#endif // WATCHER_SERVER_H
