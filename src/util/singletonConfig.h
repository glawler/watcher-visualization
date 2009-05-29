#ifndef SINGLETON_CONFIG_H
#define SINGLETON_CONFIG_H

#include "logger.h"
#include "libconfig.h++"
#include "pthread.h"

namespace watcher
{
    /**
     * @class SingletonConfig
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * A class that wraps a libconfig Config object.
     */
    class SingletonConfig
    {
        public:
            
            /** The underlaying Config object is directly accessible */
            static libconfig::Config &instance();

            /** Lock the configuration when writing */
            static void lock();

            /** Unlock the config when finished writing */
            static void unlock();

        private:
            DECLARE_LOGGER();

            SingletonConfig();
            ~SingletonConfig();

            static pthread_mutex_t accessMutex;
    };
}

#endif // SINGLETON_CONFIG_H
