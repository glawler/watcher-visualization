#ifndef SINGLETON_CONFIG_H
#define SINGLETON_CONFIG_H

#include <string>

#include "logger.h"
#include "libconfig.h++"
#include "pthread.h"

namespace watcher
{
    class singletonConfig
    {
        public:
            static singletonConfig &instance();

            void lock();
            void unlock();

            libconfig::Config &getConfig();

            void setConfigFile(const std::string &filename);    // Set the filename...
            void saveConfig();                                  // ...and write the config out to the file.

        private:
            DECLARE_LOGGER();

            singletonConfig();
            ~singletonConfig();
            libconfig::Config cfg;
            std::string filename; 
            static pthread_mutex_t accessMutex;
    };
}

#endif // SINGLETON_CONFIG_H
