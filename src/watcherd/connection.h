#ifndef WATHER_CONNECTION_H
#define WATHER_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <list>

#include "messageHandlerFwd.h"
#include "connection_fwd.h"

namespace watcher
{
    class Connection :
        private boost::noncopyable
    {
        public:
            Connection(boost::asio::io_service& io_service);
            virtual ~Connection();

            typedef boost::asio::ip::tcp::socket ConnectionSocket;
            ConnectionSocket &getSocket();

            /**
             * addMessageHandler() Set a messageHandler if you want direct access to the 
             * responses sent via sendMessage().
             * @param messageHandler - an instance of a message handler class. 
             */
            void addMessageHandler(MessageHandlerPtr messageHandler); 
            void removeMessageHandler(MessageHandlerPtr messageHandler); 

            const std::string& getPeerAddr() const { return endpoint_addr_; }
            unsigned short getPeerPort() const { return endpoint_port_; }

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
