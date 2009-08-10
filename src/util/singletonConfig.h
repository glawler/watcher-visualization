/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SINGLETON_CONFIG_H
#define SINGLETON_CONFIG_H

#include <string>

#include "logger.h"
#include "libconfig.h++"
#include "pthread.h"

namespace watcher
{
    /**
     * A class that wraps a libconfig Config object.
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
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
