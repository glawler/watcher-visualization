#include <iostream>

#include "initConfig.h"
#include "libconfig.h++"
#include "getopt.h"

using namespace watcher;
using namespace libconfig;
using namespace std;

static bool readConfig(libconfig::Config &config, const string &filename)
{
    try
    {
        config.readFile(filename.c_str());
        return true;
    }
    catch (ParseException &e)
    {
        // Can't use logging here - if we're reading the config file we probably have not
        // init'd the logging mechanism. 
        cerr << "Error reading configuration file " << filename << ": " << e.what() << endl;
        cerr << "Error: \"" << e.getError() << "\" on line: " << e.getLine() << endl;
    }
    catch (FileIOException &e)
    {
        cerr << "Unable to read file " << filename.c_str() << " given as configuration file on the command line." << endl;
    }
    return false;
}

bool watcher::initConfig(
            libconfig::Config &config, 
            int argc, 
            char **argv, 
            std::string &configFilename,
            const char configFileChar,
            const char *configFileString)
{
    int c;
    // int digit_optind = 0;

    bool retVal=false;

    while (true) 
    {
        // int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {configFileString, required_argument, 0, configFileChar},
            {0, 0, 0, 0}
        };

        opterr=0; // Don't print message just because we see an option we don't understand.

        char args[] = { configFileChar, ':', '\n' };
        c = getopt_long(argc, argv, args, long_options, &option_index);

        if (c == -1)
            break;

        if (c==configFileChar)
            if(true==(retVal=readConfig(config, optarg)))
                configFilename=optarg;

        // else - ignore things we don't understand
    }

    // Last ditch: look for a file called `echo argv[0]`.cfg.
    if(retVal==false)
    {
        string fname(argv[0]);
        fname+=".cfg"; 

        retVal=readConfig(config, fname);
        configFilename=fname;
    }

    return retVal;
}

