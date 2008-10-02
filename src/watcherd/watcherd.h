#ifndef WATCHERD_H
#define WATCHERD_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "messageHandler.h"
#include "dataRequestMessage.h"
#include "server.h"
#include "libconfig.h++"
#include "logger.h"

namespace watcher
{
    class Watcherd : 
        public MessageHandler,
        public boost::enable_shared_from_this<Watcherd>  
    //     public boost::noncopyable, 
    {
        public:

            Watcherd();
            virtual ~Watcherd(); 

            void run(const std::string &address, const std::string &port, const int &threadNum);

            void handleMessageArrive(const MessagePtr message); 
            ConnectionCommand produceReply(const MessagePtr &request, MessagePtr &reply);
            ConnectionCommand handleReply(const MessagePtr &request, const MessagePtr &reply);
            ConnectionCommand produceRequest(MessagePtr &request);

        protected:

        private:

            DECLARE_LOGGER();

            ServerPtr serverConnection;
            boost::thread connectionThread;
            libconfig::Config &config;

            // List of clients subscribed to messages.
            typedef std::list<DataRequestMessagePtr> MessageRequesters;
            MessageRequesters messageRequesters;
    };

}

#endif // WATCHERD_H
