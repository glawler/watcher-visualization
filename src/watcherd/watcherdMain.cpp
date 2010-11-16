/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <getopt.h>
#include <boost/filesystem.hpp>

#include "server.h"
#include "logger.h"
#include "libconfig.h++"
#include "initConfig.h"
#include "singletonConfig.h"
#include "watcherd.h"

using namespace std;
using namespace watcher;
using namespace libconfig; 

option Options[] = {
    { "help", 0, NULL, 'h' },
    { "config", 1, NULL, 'c' },
    { "database", 1, NULL, 'd' },
    { "read-only", 1, NULL, 'r' },
    { "logLevel", 1, NULL, 'l' },
    { "logProperties", 1, NULL, 'L' },
    { 0, 0, NULL, 0 }
};

void usage(const char *progName, bool exitp)
{
    cout << "Usage: " << basename(progName) << " [-c config filename]" << endl;
    cout << "Args: " << endl; 
    cout << "\t-h,--help\t\tshow this messsage and exit." << endl; 
    cout << "\t-d,--database database\t\t use this event database when running watcherd" << endl; 
    cout << "\t-c,--config configfile\t\tIf not given a filename of the form \""<< basename(progName) << ".cfg\" is assumed." << endl;
    cout << "\t-r, --read-only\t\t do not write events to the database." << endl;
    cout << "\t-l,--logLevel level\t\tSet the default log level. Must be one of\n";
    cout << "\t                     \t\t\toff, fatal, error, warn, info, debug, or trace.\n";
    cout << "\t-L,--logProperties filename\t\tThe log.properties filename\n";
    cout << "If a configuration file is not found on startup, a default one will be created, used, and saved on program exit." << endl;

    if (exitp)
        exit(EXIT_FAILURE); 
}

DECLARE_GLOBAL_LOGGER("watcherdMain");

int main(int argc, char* argv[])
{
    TRACE_ENTER();

    int i;
    bool readOnly = false;
    std::string dbPath, logLevel, logPropsFilename;
    while ((i = getopt_long(argc, argv, "hc:d:rl:L:", Options, NULL)) != -1) {
        switch (i) {
            case 'c':
                //handled below
                break;
            case 'r':
                readOnly = true;
                break;
            case 'd':
                dbPath=string(optarg);
                break;
            case 'l':
                logLevel=optarg;
                if (logLevel!="off" && logLevel!="fatal" && logLevel!="error" && logLevel!="warn" && 
                        logLevel!="info" && logLevel!="debug" && logLevel!="trace") { 
                    cout << endl << "logLevel must be one of off, fatal, error, warn, info, debug, or trace." << endl;
                    usage(basename(argv[0]), true); 
                    exit(EXIT_FAILURE);
                }
                break;
            case 'L':
                logPropsFilename=optarg;
                break;
            default:
                usage(argv[0], true); 
        }
    }

    string configFilename;
    Config &config=SingletonConfig::instance();
    SingletonConfig::lock();
    if (false==initConfig(config, argc, argv, configFilename))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-c|--configFile] configfile" << endl;
        return 1;
    }
    SingletonConfig::unlock();

    string logConf("log.properties");
    if (!logPropsFilename.empty())
        logConf=logPropsFilename;
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf 
             << " and adding it to the configuration file." << endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString)=logConf;
    }

    if (!boost::filesystem::exists(logConf)) {
        cerr << "Log properties file not found - logging disabled." << endl;
        Logger::getRootLogger()->setLevel(Level::getOff());
    }
    else {
        LOAD_LOG_PROPS(logConf);
        LOG_INFO("Logger initialized from file \"" << logConf << "\"");
    }
    if (!logLevel.empty()) {
        cout << "Setting default log level to " << logLevel << endl;
        Logger::getRootLogger()->setLevel(Level::toLevel(logLevel)); 
    }

    string address("localhost");
    string port("8095");
    size_t numThreads=8;

    if (!config.lookupValue("server", address))
    {
        LOG_INFO("'server' not found in the configuration file, using default: " << address 
                << " and adding this to the configuration file.");
        config.getRoot().add("server", libconfig::Setting::TypeString) = address;
    }

    if (!config.lookupValue("port", port))
    {
        LOG_INFO("'port' not found in the configuration file, using default: " << port  
                << " and adding this to the configuration file.");
        config.getRoot().add("port", libconfig::Setting::TypeString)=port;
    }

    if (!config.lookupValue("serverThreadNum", numThreads))
    {
        LOG_INFO("'serverThreadNum' not found in the configuration file, using default: " << numThreads 
                << " and adding this to the configuration file.");
           config.getRoot().add("serverThreadNum", libconfig::Setting::TypeInt)=static_cast<int>(numThreads);
    }

    std::string tmpDBPath("event.db"); 
    if (!config.lookupValue("databasePath", tmpDBPath))
    {
        LOG_INFO("'databasePath' not found in the configuration file, using default: " << dbPath
                << " and adding this to the configuration file.");
           config.getRoot().add("databasePath", libconfig::Setting::TypeString)=tmpDBPath;
    }
    if (dbPath.size())  
        config.getRoot()["databasePath"]=dbPath;

    WatcherdPtr theWatcherDaemon(new Watcherd(readOnly));
    try
    {
        theWatcherDaemon->run(address, port, (int)numThreads);
    }
    catch (std::exception &e)
    {
        LOG_FATAL("Caught exception in main(): " << e.what());
        std::cerr << "exception: " << e.what() << "\n";
    }

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    SingletonConfig::saveConfig(); 

    return 0;
}
