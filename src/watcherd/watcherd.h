#ifndef WATCHERD_H
#define WATCHERD_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "libwatcher/dataRequestMessage.h"

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

        protected:

        private:

            DECLARE_LOGGER();

            ServerPtr serverConnection;
            boost::thread connectionThread;
            libconfig::Config &config;

            ServerMessageHandlerPtr serverMessageHandlerPtr;

            // List of clients subscribed to messages.
            typedef std::list<event::DataRequestMessagePtr> MessageRequesters;
            MessageRequesters messageRequesters;
    };

    typedef boost::shared_ptr<Watcherd> WatcherdPtr;

}

#endif // WATCHERD_H
