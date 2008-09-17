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
#include "server.hpp"

#if !defined(_WIN32)

#include <pthread.h>
#include <signal.h>

#include "logger.h"
#include "libconfig.h++"
#include "initConfig.h"
#include "singletonConfig.h"

using namespace std;
using namespace watcher;
using namespace libconfig; 

int main(int argc, char* argv[])
{
    TRACE_ENTER();

    string configFilename;
    Config &config=singletonConfig::instance();
    singletonConfig::lock();
    if (false==initConfig(config, argc, argv, configFilename))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-c|--configFile] configfile" << endl;
        return 1;
    }
    singletonConfig::unlock();

    string logConf("log.properties");
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf 
             << " and adding it to the configuration file." << endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString)=logConf;
    }

    PropertyConfigurator::configureAndWatch(logConf);

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    try
    {
        string address("glory");
        string port("8095");
        std::size_t numThreads = 4;

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
                    << " and adding this to the configuration file.")
            config.getRoot().add("serverThreadNum", libconfig::Setting::TypeInt)=static_cast<int>(numThreads);
        }

        // Block all signals for background thread.
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigset_t old_mask;
        pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

        // Run server in background thread.
        // boost::shared_ptr<request_handler> requestHandler = boost::shared_ptr<request_handler>(new request_handler);
        watcher::Server s(
                address, 
                port, 
                // requestHandler, 
                numThreads);
        boost::thread t(boost::bind(&watcher::Server::run, &s));

        // Restore previous signals.
        pthread_sigmask(SIG_SETMASK, &old_mask, 0);

        // Wait for signal indicating time to shut down.
        sigset_t wait_mask;
        sigemptyset(&wait_mask);
        sigaddset(&wait_mask, SIGINT);
        sigaddset(&wait_mask, SIGQUIT);
        sigaddset(&wait_mask, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
        int sig = 0;
        sigwait(&wait_mask, &sig);

        // Stop the server.
        s.stop();
        t.join();
    }
    catch (std::exception& e)
    {
        LOG_FATAL("Exception: " << e.what());
        std::cerr << "exception: " << e.what() << "\n";
    }

    singletonConfig::lock();
    config.writeFile(configFilename.c_str());

    return 0;
}


#endif // !defined(_WIN32)
