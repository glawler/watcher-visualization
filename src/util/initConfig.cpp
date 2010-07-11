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

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

#include "initConfig.h"
#include "libconfig.h++"
#include "getopt.h"

using namespace watcher;
using namespace std;
namespace bf=boost::filesystem;

using namespace libconfig;
static bool readConfig(libconfig::Config &config, const string &filename)
{
    try
    {
        // make sure it exists. 
        if (!bf::exists(filename))
        {
            ofstream f(filename.c_str());
            f.close();
        }

        config.readFile(filename.c_str());
        return true;
    }
    catch (ParseException &e)
    {
        // Can't use logging here - if we're reading the config file we probably have not
        // init'd the logging mechanism. 
        cerr << "Error reading configuration file " << filename << ": " << e.what() << endl;
        cerr << "Error: \"" << e.getError() << "\" on line: " << e.getLine() << endl;
        exit(EXIT_FAILURE);  // !!!
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
    bool retVal=false;
    static struct option long_options[] = {
        {configFileString, required_argument, 0, configFileChar},
        {0, 0, 0, 0}
    };

    opterr=0; // Don't print message just because we see an option we don't understand.
    optind=1; // reset so getopt starts at the start again.

    char args[] = { configFileChar, ':', '\0' };

    while(-1!=(c = getopt_long(argc, argv, args, long_options, NULL))) {
        if (c==configFileChar) {
            if(true==(retVal=readConfig(config, optarg))) {
                configFilename=optarg;
                break;
            }
        }
    }

    // Last ditch: look for a file called `echo argv[0]`.cfg.
    if(retVal==false)
    {
        string fname(bf::basename(argv[0])); 
        fname+=".cfg"; 

        retVal=readConfig(config, fname);
        if(retVal)
            configFilename=fname;
        else
            configFilename="";
    }

    opterr=1; 
    optind=1; // reset so getopt starts at the start again in case someone else calls getopt()...

    return retVal;
}

