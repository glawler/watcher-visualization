#include <iostream>
#include <cstdlib>     // for EXIT_SUCCESS/FAILURE
#include <unistd.h>
#include <getopt.h>

#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"
#include "logger.h"

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

int main(int argc, char **argv)
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

    MessageStreamPtr ms=MessageStream::createNewMessageStream(serverName, service, pos, rate); 

    if(!ms)
    {
        LOG_FATAL("Unable to create new message stream to server \"" << serverName << "\" using service (or port) \"" << service);
        TRACE_EXIT_RET(EXIT_SUCCESS); 
        return EXIT_FAILURE;
    }


    MessagePtr mp(new Message);

    LOG_INFO("Starting event playback");
    ms->startStream(); 

    LOG_INFO("Waiting for events ");
    unsigned int messageNumber=0;
    while(ms->getNextMessage(mp))
        cout << "Message #" << (++messageNumber) << ": " << *mp << endl; 

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT_RET(EXIT_SUCCESS); 
    return EXIT_SUCCESS;
}
