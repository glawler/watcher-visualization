#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "watcherdClientConnection.h"
#include <libwatcher/testMessage.h>

#include "initConfig.h"
#include "singletonConfig.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;
using namespace libconfig;

int main(int argc, char **argv)
{
    TRACE_ENTER();

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

    string logConf("testCC.log.properties");
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf
            << " and adding it to the configuration file." << endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString)=logConf;
    }

    LOAD_LOG_PROPS(logConf);

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    int loopCount=1;
    if (!config.lookupValue("loopCount", loopCount))
    {
        LOG_INFO("'loopCount' not found in the configuration file, using default: " << loopCount
                << " and adding this to the configuration file.");
        config.getRoot().add("loopCount", libconfig::Setting::TypeInt)=loopCount;
    }

    string serverName("glory"); 
    if (!config.lookupValue("serverName", serverName))
    {
        LOG_INFO("'serverName' not found in the configuration file, using default: " << serverName
                << " and adding this to the configuration file.");
        config.getRoot().add("serverName", libconfig::Setting::TypeString)=serverName;
    }

    try
    {
        asio::io_service ioserv;

        WatcherdClientConnection c(ioserv, serverName, "watcherd"); 

        vector<int> ints;
        string strVal = "from testCC"; 
        for (int i = 0; i < loopCount; i++)
        {
            ints.push_back(i);
            ints.push_back(i*2);
            c.sendMessage(shared_ptr<TestMessage>(new TestMessage(strVal, ints))); 
        }

        ioserv.run(); 

        // sleep(60); 

        c.close();
        // t.join();
    }
    catch (std::exception &e)
    {
        cerr << "Caught exception: " << e.what() << endl;
    }

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    return 0;
}
