#ifndef INIT_CONFIG_H
#define INIT_CONFIG_H

#include <string>
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
            std::string &configFilename,
            const char commandLineiShort='c',
            const char *commandLineLong="configFile");
}

#endif // INIT_CONFIG_H
