#ifndef WATCHERD_H
#define WATCHERD_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>

#include "server.h"
#include "libconfig.h++"
#include "logger.h"

namespace watcher
{
    class Watcherd : boost::noncopyable
    {
        public:

            Watcherd();
            ~Watcherd(); 

            void run(const std::string &address, const std::string &port, const int &threadNum);
            void stop(); 

        protected:

        private:

            DECLARE_LOGGER();

            ServerPtr serverConnection;
            boost::thread connectionThread;
            libconfig::Config &config;
    };

}

#endif // WATCHERD_H
