#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>

#include "configuration.h"

using namespace std;

namespace Gps2EventDb {

    using namespace boost::program_options;

    boost::program_options::variables_map &getConfig()
    {
        static boost::program_options::variables_map theOneAndOnlyVariableMapInstance;
        return theOneAndOnlyVariableMapInstance;
    }

    void dumpConfiguration(ostream &out) 
    {
        // Print out the configuration information. 
        variables_map &config=getConfig();
        out << endl << "Using this runtime configuration: " << endl;
        for (variables_map::const_iterator i=config.begin(); i!=config.end(); ++i) {
            // Annoying that we have to iterate over types. 
            if (i->second.value().type()==typeid(string))
                out << i->first << " = " << i->second.as<string>() << endl;
            else if (i->second.value().type()==typeid(int))
                out << i->first << " = " << i->second.as<int>() << endl;
            else if (i->second.value().type()==typeid(unsigned int))
                out << i->first << " = " << i->second.as<unsigned int>() << endl;
            else if (i->second.value().type()==typeid(double))
                out << i->first << " = " << i->second.as<double>() << endl;
            else if (i->second.value().type()==typeid(float))
                out << i->first << " = " << i->second.as<float>() << endl;
            else if (i->second.value().type()==typeid(time_t))
                out << i->first << " = " << i->second.as<time_t>() << endl;
            else if (i->second.value().type()==typeid(bool))
                out << i->first << " = " << (i->second.as<bool>()==true ? "true" : "false") << endl;
            else 
                out << i->first << " = " << "[I don't know how to display this type. Ask Geoff to add it.]" << endl;
        }
        out << endl;
    }

    bool loadConfiguration(int argc, char **argv)
    {
        string binName=basename(argv[0]);
        const unsigned int lineLen=120;

        // Declare a group of options that will be 
        // allowed only on command line
        options_description generic("Command line only options", lineLen);
        generic.add_options()
            ("help,h", "Show help message.")
            ("configFile,c", value<string>()->default_value(binName + ".cfg"), "The location of the configuration file.")
            ;

        options_description config("Functional Options. (Allowed on command line or in configuration file.)", lineLen); 
        config.add_options()
            ("radius,r", value<double>()->default_value(180.0), "The radius, in meters, in which nodes can hear each other")
            ;

        options_description cmdline_options(lineLen);
        cmdline_options.add(generic).add(config);

        options_description config_file_options(lineLen);
        config_file_options.add(config);

        variables_map &vm=getConfig();

        // Try to sort out all options and complain/exit on error
        try {
            store(parse_command_line(argc, argv, cmdline_options), vm);
            ifstream confFile(vm["configFile"].as<string>().c_str());  // GTL god this is ugly.
            if (confFile.is_open()) {
                store(parse_config_file(confFile, config_file_options), vm); 
                confFile.close();
            }
        }
        catch (const invalid_option_value &e) { 
            cerr << "Option Error - " << e.what() << endl;
            exit(EXIT_FAILURE); 
        }
        catch (const invalid_command_line_syntax &e) {
            cerr << "Syntax Error - " << e.what() << endl;
            exit(EXIT_FAILURE); 
        }

        notify(vm);    

        if (vm.count("help")) {
            cerr << cmdline_options << endl;
            return false;
        }

        return true;
    }
}
