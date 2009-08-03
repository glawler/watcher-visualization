/** 
 * @file ogreWatchherMain.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20 
 */
#include "logger.h"
#include "libconfig.h++"
#include "initConfig.h"
#include "singletonConfig.h"

#include "ogreWatcherApplication.h"
#include "watcherMessageFrameListener.h"

using namespace std;
using namespace watcher;
using namespace libconfig;

using namespace ogreWatcher;

int main(int argc, char *argv[])
{
    TRACE_ENTER();

    string configFilename;
    SingletonConfig::lock(); 
    Config &config=SingletonConfig::instance(); 
    if (false==initConfig(config, argc, argv, configFilename))
    {
        cerr << "Error reading configuration file, unable to continue." << endl;
        cerr << "Usage: " << basename(argv[0]) << " [-f|--configFile] configfile [standard watcher arguments]" << endl;
        return 1;
    }
    SingletonConfig::setConfigFile(configFilename);
    SingletonConfig::unlock();

    string logConf("ogreWatcher.log.properties");
    if (!config.lookupValue("logProperties", logConf))
    {
        cerr << "Unable to find logproperties setting in the configuration file, using default: " << logConf << endl;
        config.getRoot().add("logProperties", Setting::TypeString)=logConf;
    }

    LOAD_LOG_PROPS(logConf); 

    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    // Create application object
    OgreWatcherApplication app;

    try {
        app.go();
    } 
    catch( Ogre::Exception& e ) {
        LOG_FATAL("An exception has occurred: " << e.getFullDescription());
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
    TRACE_EXIT();
}