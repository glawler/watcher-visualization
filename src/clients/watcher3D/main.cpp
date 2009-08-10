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
#include <cstdlib>     // for EXIT_SUCCESS/FAILURE
#include <unistd.h>
#include <getopt.h>

// Watcher includes
#include "logger.h"
#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtCore/globals.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace libconfig;

option Options[] = {
    { "help", 0, NULL, 'h' },
    { "config", 1, NULL, 'c' },
    { "speed", 1, NULL, 's' },
    { "seek", 1, NULL, 'S' },
    { 0, 0, NULL, 0 }
};

void usage(const char *progName, bool exitp)
{
    cout << "Usage: " << basename(progName) << " [-c config filename]" << endl;
    cout << "Args: " << endl; 
    cout << "   -h, show this messsage and exit." << endl; 
    cout << "   -c configfile - If not given a filename of the form \""<< basename(progName) << ".cfg\" is assumed." << endl;
    cout << "   -s, --speed=FLOAT - specify the event playback speed." << endl;
    cout << "   -S, --seek=INT - specify the offset in milliseconds to start event playback (default: live playback)." << endl;
    cout << "If a configuration file is not found on startup, a default one will be created, used, and saved on program exit." << endl;
    cout << "Config file settings:" << endl;
    cout << "server - name or ipaddress of the server to connect to." << endl;
    cout << "service - name of service (usaully \"watcherd\") or port number on which the server is listening." << endl;

    if(exitp)
        exit(EXIT_FAILURE); 
}

int main(int argc, char** argv)
{
    TRACE_ENTER(); 

    int i;
    float rate = 1.0; // playback rate
    Timestamp pos = -1; // default to live playback

    while ((i = getopt_long(argc, argv, "hc:s:S:", Options, NULL)) != -1) {
        switch (i) {
            case 'c':
                //handled below
                break;
            case 's':
                rate = atof(optarg);
                break;
            case 'S':
                pos = strtoll(optarg, NULL, 10);
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
        cout << endl << "Configuration file not found. Creating new configuration file and using default runtime values." << endl << endl;
    }
    SingletonConfig::unlock();

    string logConf(basename(argv[0]));
    logConf+=".log.properties"; 
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf 
             << " and adding it to the configuration file." << endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString)=logConf;
    }

    LOAD_LOG_PROPS(logConf);

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    string serverName("127.0.0.1");
    string service("watcherd");

    if (!config.lookupValue("server", serverName))
    {
        LOG_INFO("'server' not found in the configuration file, using default: " << serverName 
                << " and adding this to the configuration file.");
        config.getRoot().add("server", libconfig::Setting::TypeString) = serverName;
    }

    if (!config.lookupValue("service", service))
    {
        LOG_INFO("'service' not found in the configuration file, using default: " << service  
                << " and adding this to the configuration file.");
        config.getRoot().add("service", libconfig::Setting::TypeString)=service;
    }

    //
    // My code (the rest of the code is borrowed from messageStream2Text.cpp)
    //

    // Setup path
    std::string dataPath = dtCore::GetDeltaDataPathList();
    dtCore::SetDataFilePathList(dataPath + ";" + dtCore::GetDeltaRootPath() +
        "data" + ";" + dtCore::GetDeltaRootPath() + "data/models");

    // Create application
    dtCore::RefPtr<dtGame::GameApplication> app = new dtGame::GameApplication(argc, argv, "config.xml");
    app->SetGameLibraryName("Watcher3D"); // (libWatcher3D.so)
    app->Config();
    app->Run();
    dtCore::RefPtr<dtGame::GameManager> gameManager = app->GetGameManager();

    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT_RET(EXIT_SUCCESS); 
    return EXIT_SUCCESS;
}
