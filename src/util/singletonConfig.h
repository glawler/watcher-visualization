#ifndef SINGLETON_CONFIG_H
#define SINGLETON_CONFIG_H

#include <string>

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

            /** Set the filename to save this configuration to */
            static void setConfigFile(const std::string &filename_); 

            /** Save the configuration to the file */
            static void saveConfig();

        private:
            DECLARE_LOGGER();

            SingletonConfig();
            ~SingletonConfig();

            static pthread_mutex_t accessMutex;
            static std::string filename; 
    };
}

#endif // SINGLETON_CONFIG_H
