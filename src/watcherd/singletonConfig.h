#ifndef SINGLETON_CONFIG_H
#define SINGLETON_CONFIG_H

#include "logger.h"
#include "libconfig.h++"
#include "pthread.h"

namespace watcher
{
    class SingletonConfig
    {
        public:
            static libconfig::Config &instance();

            static void lock();
            static void unlock();

        private:
            DECLARE_LOGGER();

            SingletonConfig();
            ~SingletonConfig();

            static pthread_mutex_t accessMutex;
    };
}

#endif // SINGLETON_CONFIG_H
