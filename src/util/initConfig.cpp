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

static void checkAndWarnFilePermissions(const char *filename)
{
    if (0!=access(filename, R_OK)) { 
        fprintf(stderr, "Configuration file, %s, is not readable. Please check the file permissions. Unable to continue.\n", filename); 
        exit(EXIT_FAILURE); 
    }
    if (0!=access(filename, W_OK)) { 
        fprintf(stderr, "------------------------------------\n"); 
        fprintf(stderr, "Warning: config file %s is read only, changes made during run time will not be saved.\n", filename); 
        fprintf(stderr, "------------------------------------\n"); 
    }
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
			checkAndWarnFilePermissions(optarg); 
			if(true==(retVal=readConfig(config, optarg))) {
				configFilename=optarg;
				break;
			}
		}
	}

	// Last ditch: look for various files in various places. 
	// 1) [PROGRAM_NAME].cfg 
	// 2) watcher.cfg 
	// 3) {/usr/local/etc/,/etc/watcher}/[PROGRAM_NAME].cfg
	// 4) {/usr/local/etc/,/etc/watcher}/watcher.cfg
	if(retVal==false)
	{
		string basename=bf::basename(argv[0]); 
		const string possPaths[] = { 
			basename + ".cfg", 
			"watcher.cfg", 
			"/etc/watcher/" + basename + ".cfg", 
			"/usr/local/etc/watcher/" + basename + ".cfg", 
			"/etc/watcher/watcher.cfg", 
			"/usr/local/etc/watcher.cfg", 
		}; 

		for (int i=0; i<sizeof(possPaths)/sizeof(possPaths[0]); i++) {
			if (bf::exists(possPaths[i])) {
				checkAndWarnFilePermissions(possPaths[i].c_str()); 
				if(true==(retVal=readConfig(config, possPaths[i]))) {
					configFilename=possPaths[i];
					break;
				}
			}
		}
		if (configFilename.empty())
			configFilename="";
	}

	opterr=1; 
	optind=1; // reset so getopt starts at the start again in case someone else calls getopt()...

	return retVal;
}

