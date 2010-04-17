/* Copyright 2009,2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

/** 
 * @file messageStream2Text.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
/**
 * @page messageStream2Text 
 *
 * messageStream2Text is a command line program for logging/testing the watcher::MessageStream mechanism in the watcher API. 
 * When started, messageStream2Text attaches to the watcher daemon at the host given and requests a messge stream. This stream 
 * is then logged according to logging configuration in the log.properties file. (This is usually just to standard out a file.) 
 *
 * Usage: 
 * @{
 * <b>messageStream2Text -s server [optional args]</b>
 * @}
 * @{
 * Args:
 * @arg <b>-s, --server=address|name</b>, The address or name of the node running watcherd
 * @}
 * Optional args:
 * @arg <b>-c, --config</b>, the configuration file. If not given messageStream2Text.cfg is assumed. 
 * @arg <b>-s, --speed=FLOAT</b> - specify the event playback speed.
 * @arg <b>-S, --seek=INT</b> - specify the offset in milliseconds to start event playback (default: live playback).
 * @arg <b>-h, --help</b>, Show help message
 *
 * If a configuration file is not found on startup, a default one will be created, used, and saved on program exit.
 *
 * Config file settings:
 * @arg <b>logPropertiesFile</b> - the log.properties file. 
 * @arg <b>server</b> - name or ipaddress of the server to connect to.
 * @arg <b>service</b> - name of service (usaully \"watcherd\") or port number on which the server is listening.
 *
 * Sample config file:
 * @code
 * logPropertiesFile = "messageStream2Text.log.properties";
 * server = "glory";
 * service = "watcherd";
 * @endcode
 *
 */
#include <iostream>
#include <cstdlib>     // for EXIT_SUCCESS/FAILURE
#include <unistd.h>
#include <getopt.h>

#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"
#include "logger.h"

#define DEFAULT_DESCRIPTION "messageStream2Text client"

DECLARE_GLOBAL_LOGGER("messageStream2Text"); 

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace libconfig;

option Options[] = {
    { "help", 0, NULL, 'h' },
    { "config", 1, NULL, 'c' },
    { "description", 1, NULL, 'd' },
    { "join", 1, NULL, 'j' },
    { "list-streams", 0, NULL, 'l' },
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
    cout << "   -d, --description NAME - use NAME as the description string for this client's event stream." << endl;
    cout << "   -j, --join UID - join the specified message stream for sync playback." << endl;
    cout << "   -l, --list-streams - fetch the list of available streams from the watcher server." << endl;
    cout << "   -s, --speed=FLOAT - specify the event playback speed." << endl;
    cout << "   -S, --seek=INT - specify the offset in milliseconds to start event playback (default: live playback)." << endl;
    cout << "If a configuration file is not found on startup, a default one will be created, used, and saved on program exit." << endl;
    cout << "Config file settings:" << endl;
    cout << "server - name or ipaddress of the server to connect to." << endl;
    cout << "service - name of service (usaully \"watcherd\") or port number on which the server is listening." << endl;

    if(exitp)
        exit(EXIT_FAILURE); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER(); 

    int i;
    float rate = 1.0; // playback rate
    Timestamp pos = -1; // default to live playback
    bool do_list = false;
    bool do_join = false;
    uint32_t stream_uid = -1; // stream to join (default: new stream)
    std::string description(DEFAULT_DESCRIPTION);

    while ((i = getopt_long(argc, argv, "hc:d:j:ls:S:", Options, NULL)) != -1) {
        switch (i) {
            case 'c':
                //handled below
                break;
	    case 'd':
		description = optarg;
		break;
	    case 'j':
		do_join = true;
		stream_uid = boost::lexical_cast<uint32_t>(optarg);
		break;
	    case 'l':
		do_list = true;
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

    MessageStreamPtr ms=MessageStream::createNewMessageStream(serverName, service, pos, rate); 

    if(!ms)
    {
        LOG_FATAL("Unable to create new message stream to server \"" << serverName << "\" using service (or port) \"" << service);
        TRACE_EXIT_RET(EXIT_SUCCESS); 
        return EXIT_FAILURE;
    }

    ms->setDescription(description);


    if (do_list)
	ms->listStreams();
    else if (do_join)
	ms->subscribeToStream(stream_uid);
    else {
	LOG_INFO("Starting event playback");
	ms->startStream(); 
    }

    LOG_INFO("Waiting for events ");
    unsigned int messageNumber=0;
    MessagePtr mp(new Message);
    while(ms->getNextMessage(mp))
        cout << "Message #" << (++messageNumber) << ": " << *mp << endl; 

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT_RET(EXIT_SUCCESS); 
    return EXIT_SUCCESS;
}
