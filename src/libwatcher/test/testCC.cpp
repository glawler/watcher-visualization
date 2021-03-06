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
/** 
 * @file testCC.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "libwatcher/client.h"
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
        cerr << "Usage: " << basename(argv[0]) << " [-c|--configFile]" << endl;
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

        Client c(serverName); 

        vector<int> ints;
        string strVal = "from testCC"; 
        for (int i = 0; i < loopCount; i++)
        {
            if(i<100)
            {
                ints.push_back(i+1);
                ints.push_back((i+1)*2);
            }
            LOG_INFO("Sending message number " << i); 
            c.sendMessage(shared_ptr<TestMessage>(new TestMessage(strVal, ints))); 
            sleep(1);
        }

        // This will quickly break things as the packet gets to be too large. 
        // vector<MessagePtr> messages;
        // for (int i = 0; i < loopCount; i++)
        // {
        //     if(i<100)
        //     {
        //         ints.push_back(i+1);
        //         ints.push_back((i+1)*2);
        //     }
        //     MessagePtr m=MessagePtr(new TestMessage(strVal, ints));
        //     messages.push_back(m);
        // }
        // c.sendMessages(messages); 

        c.wait(); 
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
