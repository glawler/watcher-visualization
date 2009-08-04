//
// posix_main.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modifed by Geoff Lawler, SPARTA, inc. 2008.
//

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "server.h"

#if !defined(_WIN32)

#include <pthread.h>
#include <signal.h>
#include <getopt.h>

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
    { "read-only", 1, NULL, 'r' },
    { 0, 0, NULL, 0 }
};

void usage(const char *progName, bool exitp)
{
    cout << "Usage: " << basename(progName) << " [-c config filename]" << endl;
    cout << "Args: " << endl; 
    cout << "   -h, show this messsage and exit." << endl; 
    cout << "   -c configfile - If not given a filename of the form \""<< basename(progName) << ".cfg\" is assumed." << endl;
    cout << "   -r, --read-only - do not write events to the database." << endl;
    cout << "If a configuration file is not found on startup, a default one will be created, used, and saved on program exit." << endl;

    if (exitp)
        exit(EXIT_FAILURE); 
}

int main(int argc, char* argv[])
{
    TRACE_ENTER();

    int i;
    bool readOnly = false;
    while ((i = getopt_long(argc, argv, "hc:r", Options, NULL)) != -1) {
        switch (i) {
            case 'c':
                //handled below
                break;
            case 'r':
                readOnly = true;
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
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf 
             << " and adding it to the configuration file." << endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString)=logConf;
    }

    LOAD_LOG_PROPS(logConf);

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    string address("glory");
    string port("8095");
    size_t numThreads;
    std::string dbPath("event.db");

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

    if (!config.lookupValue("databasePath", dbPath))
    {
        LOG_INFO("'databasePath' not found in the configuration file, using default: " << dbPath
                << " and adding this to the configuration file.");
           config.getRoot().add("databasePath", libconfig::Setting::TypeString)=dbPath;
    }

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
    config.writeFile(configFilename.c_str());

    return 0;
}


#endif // !defined(_WIN32)
