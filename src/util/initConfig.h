#ifndef INIT_CONFIG_H
#define INIT_CONFIG_H

#include "libconfig.h++"

namespace watcher
{
    // Initialize the Config passed in. 
    //
    // Looks for configChar or configString on the command line
    // and uses the value of the config file found to initialize
    // the passed in Config object. Returns false if the argument 
    // was not found, true otherwise.
    //
    bool initConfig(
            libconfig::Config &config, 
            int argc, 
            char **argv, 
            const char configFileChar='c',
            const char *configFileString="configFile");
}

#endif // INIT_CONFIG_H
