/**
 * File: configuration.h
 * Holds declaration for functions to get system singletons. 
 *
 * Author: Geoff Lawler <geoff.lawler@cobham.com>
 * Date: 2010-08-09
 *
 */
#ifndef GPS2EVENT_CONF_H
#define GPS2EVENT_CONF_H

#include <iostream>
#include <boost/program_options.hpp>

namespace Gps2EventDb {

    /** Return a reference to the one and only variable_map used to hold the configuration. */
    boost::program_options::variables_map &getConfig(); 

    /** 
     * Load configuration from command line or cfg file. This will load the 
     * configuration into the singleton configuration returned by getConfig(). 
     * This should really only be called once. (?)
     */
    bool loadConfiguration(int argc, char **argv);

    /**
     * Write current configuration stored in configuration singleton to the ostream given
     */
    void dumpConfiguration(std::ostream &out);
}

#endif //  WORMHOLE_CONFIGURATION_IS_A_BLAST_MAN
