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

#ifndef INIT_CONFIG_H
#define INIT_CONFIG_H

#include <string>
#include "libconfig.h++"

namespace watcher
{
    /**
     * Initialize the Config passed in. 
     *
     * Looks for char arguemnt or string argument on the command line
     * and uses the value of the config file found to initialize
     * the passed in Config object. e.g. "-c foobar.cfg", finds the file
     * foobar.cfg and uses it to initialize the Config object passed in.
     *
     * If the passed in char or string is not found on the command line, 
     * a configuration file of the form 'programname.cfg', is searched for
     * and used if found. (Where "programname" is the basename of argv[0].)
     *
     * Once the filename is found, if there is an error parsing the data, 
     * exit(EXIT_FAILURE) is called as it is assumed that bad/unreadable input 
     * to a program is a FATAL error. 
     *
     * @param[in,out] config the config object to initialize.
     * @param[in] argc number of args in argv
     * @param[in] argv strings on the command line
     * @param[out] configFilename The name of the configuration file, if found.
     * @param[in] commandLineShort the short form of the command line arg that contains the 
     *  configuration filename. (ex: "-c foobar.cfg", the value is 'c')
     * @param[in] commandLineLong The long form of the command line arguement that 
     *  points to the configuration file arguement. (ex: "--configFile foobar.cfg", the value is "configFile")
     *
     * @retval false if the argument or default file name was not found
     * @retval true otherwise
     */
    bool initConfig(
            libconfig::Config &config, 
            int argc, 
            char **argv, 
            std::string &configFilename,
            const char commandLineShort='c',
            const char *commandLineLong="configFile");
}

#endif // INIT_CONFIG_H
