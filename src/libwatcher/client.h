#ifndef WATCHER_CLIENT_HPP
#define WATCHER_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "logger.h"

#include "clientConnection.h"
#include "libwatcher/message.h"

namespace watcher 
{
    //
    // Currently it's just a thin wrapper around the clientConnection class. 
    //
    class Client : private boost::noncopyable
    {
        public:
            /**
             * Construct the Client to connect to the specified TCP address and service.
             * to send messages. Default service is "watcherd" - a watcherd running somewhere.
             * @param[in] server the host to connect to
             * @param[in] service the service/port on the server
             * @param[in] reconnect when true, attempt to reconnect to server when connection is lost
             */
            explicit Client(
                    const std::string& server, 
                    const std::string& service="watcherd",
                    bool reconnect = false);

            virtual ~Client();

            /**
             * Send a messsage to the server.
             * 
             * @param - message - the message to be sent. 
             * @return - boolean - true on success, false otherwise
             */
            bool sendMessage(const event::MessagePtr message);
            bool sendMessages(const std::vector<event::MessagePtr> &messages);

            /**
             * setMessageHandler() Set a messageHandler if you want direct access to the 
             * responses sent via sendMessage().
             * @param messageHandler - an instance of a message handler class. 
             */
            void addMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * wait until all work is done in the client.
             */
            void wait();

            /**
             * Perform a synchronous connection attempt to the server.
             * @retval true connection succeeded
             * @retval false connection failed
             */
            bool connect();

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
