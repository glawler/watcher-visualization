/**
 * @file connection.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATHER_CONNECTION_H
#define WATHER_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <list>

#include "messageHandlerFwd.h"
#include "connection_fwd.h"

namespace watcher
{
    /// Base class for common functionality among different types of connections to the watcher daemon.
    class Connection :
        private boost::noncopyable
    {
        public:
            /** Create a new connection.
             * @param io_service the ASIO service for the application
             */
            Connection(boost::asio::io_service& io_service);
            virtual ~Connection();

            /// convenience typedef to refer to the socket type for the connection
            typedef boost::asio::ip::tcp::socket ConnectionSocket;

            /** Retrieve the ASIO socket object associated with this connection.
             * @return reference to socket object
             */
            ConnectionSocket &getSocket();

            /**
             * Set a messageHandler if you want direct access to the 
             * responses sent via sendMessage().
             * @param messageHandler an instance of a message handler class. 
             */
            void addMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * Removes a message handler set via the addMessageHandler member function.
             * @param messageHandler an instance of a message handler class. 
             */
            void removeMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * Returns the address of this connection's peer.
             */
            const boost::asio::ip::tcp::endpoint remoteEndpoint() const { return theSocket.remote_endpoint(); }

        protected:

            typedef std::list<MessageHandlerPtr> MessageHandlerList;
            MessageHandlerList messageHandlers; 

            ConnectionSocket theSocket;

            std::string endpoint_addr_; //< IP address of the connection endpoint
            unsigned short endpoint_port_; //< TCP/IP port of the connection endpoint

        private:

            DECLARE_LOGGER(); 
    };

}

#endif // WATHER_CONNECTION_H
