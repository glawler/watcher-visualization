#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <vector>

#include "connection.h" // Must come before boost/serialization headers.
#include "message.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "logger.h"

namespace watcher 
{
    class client
    {
        public:
            /// Constructor starts the asynchronous connect operation.
            client(const std::string& host, const std::string& service);

        protpected:

            /// Handle completion of a connect operation.
            void handleConnect(
                    const boost::system::error_code& e, 
                    boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

        private:
            /// Give this instance it's own logger.
            DECLARE_LOGGER();

            /// Private io_service instance
            boost::asio::io_service ioService;

            /// The connection to the server.
            connection_ptr connection;

            /// The data received from the server.
            // std::vector<TestMessage> messages;
            // std::vector<boost::shared_ptr<Message> >messages;
            boost::shared_ptr<Message> message;
    };

    std::ostream &operator<<(std::ostream &out, const Watchee &watchee);

} // namespace watcher


