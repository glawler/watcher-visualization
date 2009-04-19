#ifndef WATCHERD_H
#define WATCHERD_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "libwatcher/dataRequestMessage.h"
#include "libwatcher/message_fwd.h"

#include "watcherd_fwd.h"
#include "serverMessageHandler.h"
#include "server.h"
#include "libconfig.h++"
#include "logger.h"

namespace watcher
{
    class Watcherd : 
        public boost::enable_shared_from_this<Watcherd>  
    //     public boost::noncopyable, 
    {
        public:

            Watcherd();
            virtual ~Watcherd(); 

            void run(const std::string &address, const std::string &port, const int &threadNum);

            /** Subscribe a ServerConnection to the event stream. */
            void subscribe(ServerConnectionPtr);

            /** Unsubscribe a ServerConnection to the event stream. */
            void unsubscribe(ServerConnectionPtr);

            /** Post an event to all listening clients */
            void sendMessage(event::MessagePtr);
            void sendMessage(const std::vector<event::MessagePtr>&);
        protected:

        private:

            DECLARE_LOGGER();

            ServerPtr serverConnection;
            boost::thread connectionThread;
            libconfig::Config &config;

            ServerMessageHandlerPtr serverMessageHandlerPtr;

            // List of clients subscribed to messages.
            typedef std::list<ServerConnectionPtr> MessageRequestors;
            MessageRequestors messageRequestors;
            pthread_rwlock_t messageRequestorsLock;
    };
}

#endif // WATCHERD_H
