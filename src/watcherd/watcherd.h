#ifndef WATCHERD_H
#define WATCHERD_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include "logger.h"

namespace watcher
{
    class Watcherd : boost::noncopyable
    {
        public:


        protected:


        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<Watcherd> WatcherdPtr;
    WatcherdPtr getWatcherd(); 
}

#endif // WATCHERD_H
