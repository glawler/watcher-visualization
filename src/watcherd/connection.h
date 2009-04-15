#ifndef WATHER_CONNECTION_H
#define WATHER_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <list>

#include "messageHandler.h"

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

        protected:

            typedef std::list<MessageHandlerPtr> MessageHandlerList;
            MessageHandlerList messageHandlers; 

            ConnectionSocket theSocket;

        private:

            DECLARE_LOGGER(); 
    };

}

#endif // WATHER_CONNECTION_H
