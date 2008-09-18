#ifndef WATCHER_CLIENT_HPP
#define WATCHER_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "logger.h"

#include "clientConnection.hpp"
#include "message.h"

namespace watcher 
{
    //
    // The top-level class of a watcherd client. 
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
            explicit Client(const std::string& server, const std::string& service="watcherd");

            bool sendMessage(const boost::shared_ptr<Message> message);

        protected:
        private:

            DECLARE_LOGGER();

            boost::asio::io_service ioService;

            std::string server;
            std::string service;

            clientConnectionPtr clientConnection;

    };
} // namespace watcher

#endif // WATCHER_CLIENT_HPP