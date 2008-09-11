#include <iostream>

#include "logger.h"
#include "libconfig.h++"
#include "initConfig.h"
#include "singletonConfig.h"

using namespace std;
using namespace watcher;
using namespace libconfig; 
    
int main(int argc, char *argv[])
{
    TRACE_ENTER();

    Config &config=singletonConfig::instance();
    singletonConfig::lock();
    if (false==initConfig(config, argc, argv))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-c|--configFile] configfile" << endl;
        return 1;
    }
    singletonConfig::unlock();

    string logConf("log.properties");
    if (!config.lookupValue("logproperties", logConf))
    {
        cerr << "Unable to find logproperties setting in the configuration file, using default: " << logConf << endl;
    }

    PropertyConfigurator::configureAndWatch(logConf);

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");
}
