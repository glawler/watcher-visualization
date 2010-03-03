/**
 * File: configuration.h
 * Holds declaration for functions to get system singletons. 
 *
 * Author: Geoff Lawler <geoff.lawler@cobham.com>
 * Date: 2009-08-18
 *
 */
#ifndef CONFIG_IS_IN_THE_HOUSE
#define CONFIG_IS_IN_THE_HOUSE

#include <iostream>
#include <boost/program_options.hpp>

namespace randomScenario {

    /** Return a reference to the one and only variable_map used to hold the configuration. */
    boost::program_options::variables_map &getConfig(); 

    /** 
     * Load configuration from command line or cfg file. This will load the 
     * configuration into the singleton configuration returned by getConfig(). 
     * This should really only be called once. 
     */
    bool loadConfiguration(int argc, char **argv);

    /**
     * Print configuration to stream given.
     */
    void printConfiguration(std::ostream &out); 

}

#endif //  CONFIG_IS_IN_THE_HOUSE
