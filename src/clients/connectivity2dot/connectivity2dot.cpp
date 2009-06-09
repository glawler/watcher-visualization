#include <iostream>
#include <fstream>
#include <csignal>
#include <stdlib.h>     // for EXIT_SUCCESS/FAILURE

#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"
#include "libwatcher/watcherGraph.h"
#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace libconfig;

namespace connectivity2dot
{
    sig_atomic_t dumpGraph;
    sig_atomic_t dumpConfig;
}

void sigUsr1Handler(int)
{
    connectivity2dot::dumpGraph=1;
}

void sigUsr2Handler(int)
{
    connectivity2dot::dumpConfig=1;
}

void usage(const char *progName, bool exitp)
{
    cout << "Usage: " << basename(progName) << " [-c config filename]" << endl;
    cout << "Args: " << endl; 
    cout << "   -h, show this messsage and exit." << endl; 
    cout << "   -c configfile - If not given a filename of the form \""<< basename(progName) << ".cfg\" is assumed." << endl;
    cout << "If a configuration file is not found on startup, a default one will be created, used, and saved on program exit." << endl;
    cout << "Config file settings:" << endl;
    cout << "   logPropertiesFile - the location of the log.properties file" << endl;
    cout << "   server - name or ipaddress of the server to connect to." << endl;
    cout << "   service - name of service (usaully \"watcherd\") or port number on which the server is listening." << endl;
    cout << "   outfile - filename where the dot file is written" << endl;
    cout << endl;
    cout << "To use this program: run " << basename(progName) << " when it receives a USR1 signal, it'll write the "; 
    cout << "    current state of the topology to the output file specified in the cfg file." << endl;

    if(exitp)
        exit(EXIT_FAILURE); 
}

void saveConfig(const string &filename)
{
    LOG_INFO("Saving current configuration to " << filename); 
    Config &config=SingletonConfig::instance();
    SingletonConfig::lock();
    config.writeFile(filename.c_str());
    SingletonConfig::unlock();
}

// GTL - this shuld be put in SingletonConfig along with other types, maybe templatized 
string getConfigValue(Config &config, const string &key, const string &defaultVal)
{
    TRACE_ENTER();
    string retVal=defaultVal;
    if (!config.lookupValue(key, retVal))
    {
        LOG_INFO("'" << key << "' not found in the configuration file, using default: " << retVal  
                << " and adding this to the configuration file.");
        config.getRoot().add(key, libconfig::Setting::TypeString)=retVal;
    }
    TRACE_EXIT();
    return retVal;
}

int main(int argc, char **argv)
{
    TRACE_ENTER(); 

    for(int i=0; i<argc; i++)
        if(0==strncmp(argv[i], "-h", sizeof("-h")) || 0==strncmp(argv[i], "--help", sizeof("--help")))
            usage(argv[0], true); 

    connectivity2dot::dumpGraph=0;
    connectivity2dot::dumpConfig=0;

    string configFilename;
    Config &config=SingletonConfig::instance();
    SingletonConfig::lock();
    if (false==initConfig(config, argc, argv, configFilename))
    {
        cout << endl << "Configuration file not found. Creating new configuration file and using default runtime values." << endl << endl;
    }
    SingletonConfig::unlock();

    string logConf(getConfigValue(config, "logPropertiesFile", string(basename(argv[0])) + string(".log.properties")));
    LOAD_LOG_PROPS(logConf); 
    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    string serverName(getConfigValue(config, "server", "127.0.0.1")); 
    string service(getConfigValue(config, "service", "watcherd"));
    string outfileName(getConfigValue(config, "outfile", "connectivity.dot"));

    saveConfig(configFilename);

    // setup signal handling.
    void (*prevFn)(int)=signal(SIGUSR1, sigUsr1Handler);
    if (prevFn==SIG_IGN) 
        signal(SIGUSR1, SIG_IGN);
    prevFn=signal(SIGUSR2, sigUsr2Handler); 
    if (prevFn==SIG_IGN) 
        signal(SIGUSR2, SIG_IGN);

    MessageStreamPtr ms=MessageStream::createNewMessageStream(serverName, service); 

    if(!ms)
    {
        LOG_FATAL("Unable to create new message stream to server \"" << serverName << "\" using service (or port) \"" << service);
        TRACE_EXIT_RET(EXIT_SUCCESS); 
        return EXIT_FAILURE;
    }

    ms->startStream(); 

    WatcherGraph theGraph;
    MessagePtr mp(new Message);

    while(1)
    {
        if(connectivity2dot::dumpGraph)
        {
            connectivity2dot::dumpGraph=false;
            LOG_INFO("Dumping current connectivity graph to " << outfileName); 
            ofstream fout(outfileName.c_str()); 
            fout << theGraph;
            fout.close();
        }
        if(connectivity2dot::dumpConfig)
        {
            connectivity2dot::dumpConfig=false;
            saveConfig(configFilename);
        }
        theGraph.doMaintanence(); // check expiration, etc. 
        while(ms->isStreamReadable())
        {
            ms->getNextMessage(mp);
            LOG_DEBUG("Got message: " << *mp);
            theGraph.updateGraph(mp);
        }
        sleep(1); 
    }

    saveConfig(configFilename);

    TRACE_EXIT_RET(EXIT_SUCCESS); 
    return EXIT_SUCCESS;
}
