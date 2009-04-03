#ifndef WATCHER_CLIENT_HPP
#define WATCHER_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "logger.h"

#include "watcherdClientConnection.h"
#include <libwatcher/message.h>

namespace watcher 
{
    //
    // Currently it's just a thin wrapper around the clientConnection class. 
    // In the future, I may make this class a little easier to use by: 
    // - put it in a library 
    // - remove boost dependencies, etc
    // - add a got-a-response callback, etc. 
    //
    class Client : private boost::noncopyable
    {
        public:
            /// Construct the Client to connect to the specified TCP address and service.
            // to send messages. Default service is "watcherd" - a watcherd running somewhere.
            explicit Client(
                    const std::string& server, 
                    WatcherdClientMessageHandlerPtr messageHandler=WatcherdClientMessageHandlerPtr(),
                    const std::string& service="watcherd");

            ~Client() {} 

            bool sendMessage(const event::MessagePtr message);

        protected:
        private:

            DECLARE_LOGGER();

            boost::asio::io_service ioService;

            std::string server;
            std::string service;

            WatcherdClientConnectionPtr watcherdClientConnection;
    };

    typedef boost::shared_ptr<Client> ClientPtr;

} // namespace watcher

#endif // WATCHER_CLIENT_HPP
