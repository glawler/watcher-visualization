#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/asio/ip/address.hpp>

#include "configuration.h"

using namespace std;

namespace randomScenario {

    using namespace boost::program_options;

    boost::program_options::variables_map &getConfig()
    {
        static variables_map theOneAndOnlyVariableMapInstance;
        return theOneAndOnlyVariableMapInstance;
    }

    bool validateConfiguration() 
    {
        variables_map &vm=getConfig();

        if (vm.count("nodeDegreePercentage")) { 
            float tmp=vm["nodeDegreePercentage"].as<float>();
            if (tmp>100.0 || tmp <0.0) { 
                cerr << "nodeDegreePercentage must be between 100 and 0." << endl;
                return false;
            }
        }
                 
        if (vm.count("nodeLabelPercentage")) { 
            float tmp=vm["nodeLabelPercentage"].as<float>();
            if (tmp>100.0 || tmp <0.0) { 
                cerr << "nodeLabelPercentage must be between 100 and 0." << endl;
                return false;
            }
        }
        return true;
    }

    bool loadConfiguration(int argc, char **argv)
    {
        variables_map &vm=getConfig();

        string binName=basename(argv[0]);
        const unsigned int lineLen=120;

        // Declare a group of options that will be 
        // allowed only on command line
        options_description generic("Command line only options", lineLen);
        generic.add_options()
            ("help,h", "Show help message.")
            ("configFile,c", value<string>()->default_value(binName + ".cfg"), "The location of the configuration file.");

        // 
        // add your configurarion paramters here
        // 
        string logLevel;
        options_description config("Command line and configuration file options (command line will override config file)", lineLen);
        config.add_options()
            ("nodeNum,n", value<unsigned int>()->default_value(48), "The number of nodes in the random scenario.")
            ("nodeDegreePercentage", value<float>()->default_value(5.0), "The average number of neighbors of each node as percentage.")
            ("nodeLabelPercentage,L", value<float>()->default_value(5.0), "The average number of node that have labels applied as percentage.")
            ("duration,d", value<int>()->default_value(-1), "The duration of the scenario. -1 means go forever.")
            ("layerNum,l", value<unsigned int>()->default_value(1), "The number of layers to add to the scenario.")
            ("server,s", value<string>()->default_value("localhost"), "Where the watcherd is running.")
            ("eastWest,E", value<unsigned int>()->default_value(1000), "Max east/west width of the playing field.")
            ("northSouth,N", value<unsigned int>()->default_value(100), "Max north/south width of the playing field.")
            ("upDown,U", value<unsigned int>()->default_value(1000), "Max up/down of the playing field.")
            ("radius,r", value<unsigned int>()->default_value(20), "Distance in which two nodes are neighbors.\n")
            ("debug", value<bool>()->default_value(false), "If \"true\", show debug information on stdout.\n")
            ("logproperties,p", value<string>()->default_value(binName + ".log.properties"), "The log properties file")
            ("logLevel,l", value<string>(&logLevel)->default_value("debug"), "The level to log at. Valid options: trace, debug, info, warn, error, and fatal");

        options_description cmdline_options(lineLen);
        cmdline_options.add(generic).add(config); 

        options_description config_file_options(lineLen);
        config_file_options.add(config); 

        store(parse_command_line(argc, argv, cmdline_options), vm);

        ifstream confFile(vm["configFile"].as<string>().c_str());  // GTL god this is ugly.
        if (confFile.is_open()) {
            store(parse_config_file(confFile, config_file_options), vm); 
            confFile.close();
        }

        notify(vm);    

        if (vm.count("help")) {
            cerr << cmdline_options << endl;
            return false;
        }

        if (!validateConfiguration()) { 
            cerr << "Error in configuration." << endl;
            return false;
        }

        return true;
    }

    void printConfiguration(ostream &out)
    {
        variables_map &config=getConfig();
        cout << endl << "Using this runtime configuration (you can cut and paste into cfg file if you want):" << endl;
        cout << "----- 8< ---------- start ------ 8< -----------" << endl;
        for (variables_map::const_iterator i=config.begin(); i!=config.end(); ++i) {
            // Annoying that we have to iterate over types. 
            if (i->second.value().type()==typeid(string))
                cout << i->first << " = " << i->second.as<string>() << endl;
            else if (i->second.value().type()==typeid(int))
                cout << i->first << " = " << i->second.as<int>() << endl;
            else if (i->second.value().type()==typeid(unsigned int))
                cout << i->first << " = " << i->second.as<unsigned int>() << endl;
            else if (i->second.value().type()==typeid(bool))
                cout << i->first << " = " << (i->second.as<bool>()==true ? "true" : "false") << endl;
            else if (i->second.value().type()==typeid(double))
                cout << i->first << " = " << i->second.as<double>() << endl;
            else if (i->second.value().type()==typeid(float))
                cout << i->first << " = " << i->second.as<float>() << endl;
            else if (i->second.value().type()==typeid(long))
                cout << i->first << " = " << i->second.as<long>() << endl;
            else if (i->second.value().type()==typeid(unsigned long))
                cout << i->first << " = " << i->second.as<unsigned long>() << endl;
            else if (i->second.value().type()==typeid(long long))
                cout << i->first << " = " << i->second.as<long long>() << endl;
            else 
                cout << i->first << " = " << "[I don't know how to display this type. Ask Geoff to add it.]" << endl;
        }
        cout << "----- 8< ---------- end ------ 8< -----------" << endl;
        cout << endl;
    }
}
