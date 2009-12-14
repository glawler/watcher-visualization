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
 * Connects to a Watcher server and writes the event stream to a KML file suitable for use
 * with Google Earth.
 *
 * @author michael.elkins@cobham.com
 */

#include <unistd.h>
#include <getopt.h>
#include <stdint.h>

#include <cassert>
#include <iostream>
#include <cstdlib>

#include "logger.h"
#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"
#include "libwatcher/watcherGraph.h"
#include "libwatcher/playbackTimeRange.h"

#define TOOL_NAME "earthWatcher"
DECLARE_GLOBAL_LOGGER(TOOL_NAME);

using namespace watcher;

void write_kml(const WatcherGraph& graph, const std::string& outputFile); // kml.cc

namespace watcher {
    float LayerPadding = 10;
    float Lonoff = 0.0;
    float Latoff = 0.0;
    float Altoff = 0.0;
    int SplineSteps = 20;
}

namespace {

//arguments to getopt_long()
const option OPTIONS[] = {
    { "latoff", required_argument, 0, 'a' },
    { "altoff", required_argument, 0, 'A' },
    { "config", required_argument, 0, 'c' },
    { "help", no_argument, 0, 'h' },
    { "output", required_argument, 0, 'o' },
    { "lonoff", required_argument, 0, 'O' },
    { "refresh", required_argument, 0, 'r' },
    { "seek", required_argument, 0, 'S' },
    { "server", required_argument, 0, 's' },
    { "speed", required_argument, 0, 'd' },
    { "steps", required_argument, 0, 't' },
    { 0, 0, 0, 0 }
};

const char *CONFIG_FILE = TOOL_NAME ".cfg";
const char *OUTPUT_FILE = "watcher.kml";
const char *PROPERTY_FILE = TOOL_NAME ".log.properties";
const char *DEFAULT_HOST = "127.0.0.1";
const unsigned int DEFAULT_REFRESH = 1; // SECONDS

void usage()
{
    const char *SEP = "\t\t";   // separator for argument/description columns

    std::cout << "usage: earthWatcher [ -h ] [ -c FILE ]\n"
        "  -a, --latoff OFF" << SEP << "translate GPS coordinates relative to a given latitude\n"
        "  -A, --altoff OFF" << SEP << "translate GPS coordinates relative to the given altitude\n"
        "  -c, --config FILE" << SEP << "specify a configuration file (default: " << CONFIG_FILE << ")\n"
        "  -d, --speed SPEED" << SEP << "specify the event playback rate (default: 1.0)\n"
        "  -h, --help\t" << SEP << "display this help message\n"
        "  -o, --output FILE" << SEP << "specifies the output KML file (default: " << OUTPUT_FILE << ")\n"
        "  -O, --lonoff OFF" << SEP << "translate GPS coordinates relative to a given longitude\n"
        "  -r, --refresh SECS" << SEP << "write the the output every SECS seconds (default: " << DEFAULT_REFRESH << ")\n"
        "  -s, --server HOST" << SEP << "connect to the watcher server on the given host (default: " << DEFAULT_HOST << ")\n"
        "  -S, --seek POS" << SEP << "start event playback at timestamp POS (default: -1)\n"
        "  -t, --steps NUM" << SEP << "number of points to use when drawing a spline (default: " << SplineSteps << ")\n"
        "\tPOS may be specified relative to the first and last timestamps in the Watcher database by prefixing the offset with + or -\n"
        "\tExample: +5000 means 5 seconds after the first event in the database.\n"
        << std::endl;
}

} // end namespace

int main(int argc, char **argv)
{
    TRACE_ENTER();

    const char *output_file = 0;
    Timestamp start_timestamp = -1 ; // default to EOF (live playback)
    unsigned int refresh = DEFAULT_REFRESH;
    float speed = 1.0;
    bool relativeTS = false; // when true, start_timestamp is relative to first or last event in the Watcher DB
    unsigned int args = 0;
    enum {
        argLayerPadding = (1<<0),
        argLonoff = (1<<1),
        argLatoff = (1<<2),
        argAltoff = (1<<3),
        argOutputFile = (1<<4),
        argServerName = (1<<5)
    };
    std::string outputFile(OUTPUT_FILE);
    std::string serverName(DEFAULT_HOST);

    for (int i; (i = getopt_long(argc, argv, "a:A:hc:d:o:O:r:S:t:", OPTIONS, 0)) != -1; ) {
        switch (i) {
            case 'c':
                break; //handled by initConfig()

            case 'a':
                Latoff = atof(optarg);
                args |= argLatoff;
                break;

            case 'A':
                Altoff = atof(optarg);
                args |= argAltoff;
                break;

            case 'd':
                speed = atof(optarg);
                break;

            case 'o': // output-file
                outputFile = optarg;
                args |= argOutputFile;
                break;

            case 'O':
                Lonoff = atof(optarg);
                args |= argLonoff;
                break;

            case 'r':
                refresh = atoi(optarg);
                break;

            case 'S':
                relativeTS = (optarg[0] == '+' || optarg[0] == '-'); // when +/- is prefix, seek relative to starting/ending event
                start_timestamp = atol(optarg);
                if (start_timestamp == -1)
                    relativeTS = false; // special case for EOF value
                break;

            case 's':
                serverName = optarg;
                args |= argServerName;
                break;

            case 't':
                SplineSteps = atoi(optarg);
                if (SplineSteps < 2) {
                    std::cerr << "error: number of steps can not be less than 2" << std::endl;
                    return EXIT_FAILURE;
                }
                break;

            case 'h':
            default:
                usage();
                TRACE_EXIT_RET(EXIT_SUCCESS);
                return EXIT_SUCCESS;
                break;
        }
    }

    libconfig::Config& config = SingletonConfig::instance();
    SingletonConfig::lock();
    std::string configFilename; //filled by initConfig()
    if (! watcher::initConfig(config, argc, argv, configFilename))
        std::cout << "Configuration file not found. Creating new configuration file and using default runtime values." << std::endl;
    SingletonConfig::unlock();

    /* handle log.properties file as a special case */
    std::string logConf(PROPERTY_FILE);
    if (!config.lookupValue("logProperties", logConf)) {
        config.getRoot().add("logProperties", libconfig::Setting::TypeString) = logConf;
    }
    LOAD_LOG_PROPS(logConf); 
    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    std::string service("watcherd");
    struct {
        const char *configName;
        std::string *value;
        unsigned int bit;
    } ConfigString[] = {
        { "server", &serverName, argServerName },
        { "service", &service, 0 },
        { "outputFile", &outputFile, argOutputFile },
        { 0, 0, 0 } // terminator
    };

    for (size_t i = 0; ConfigString[i].configName != 0; ++i) {
        if ((args & ConfigString[i].bit) == 0 && !config.lookupValue(ConfigString[i].configName, *ConfigString[i].value)) {
            LOG_INFO("'" << ConfigString[i].configName << "' not found in the configuration file, using default: " << *ConfigString[i].value
                     << " and adding this to the configuration file.");
            config.getRoot().add(ConfigString[i].configName, libconfig::Setting::TypeString) = *ConfigString[i].value;
        }
    }

    LOG_DEBUG("latOff=" << Latoff << " lonOff=" << Lonoff << " altOff=" << Altoff);

    struct {
        const char *configName;
        float* value;
        unsigned int bit;
    } ConfigFloat[] = {
        { "layerPadding", &LayerPadding, argLayerPadding },
        { "lonOff", &Lonoff, argLonoff },
        { "latOff", &Latoff, argLatoff },
        { "altOff", &Altoff, argAltoff },
        { 0, 0, 0 } // terminator
    };

    for (size_t i = 0; ConfigFloat[i].configName != 0; ++i) {
        if ((args & ConfigFloat[i].bit) == 0 && !config.lookupValue(ConfigFloat[i].configName, *ConfigFloat[i].value)) {
            LOG_INFO("'" << ConfigFloat[i].configName << "' not found in the configuration file, using default: " << *ConfigFloat[i].value
                     << " and adding this to the configuration file.");
            config.getRoot().add(ConfigFloat[i].configName, libconfig::Setting::TypeFloat) = *ConfigFloat[i].value;
        }
    }

    // open a message stream of live events for now
    MessageStreamPtr ms(MessageStream::createNewMessageStream(serverName, service, start_timestamp, speed));
    if (!ms) {
        LOG_FATAL("Unable to create new message stream to server \"" << serverName << "\" using service (or port) \"" << service);
        TRACE_EXIT_RET(EXIT_FAILURE); 
        return EXIT_FAILURE;
    }

    if (relativeTS) {
        ms->getMessageTimeRange();
    } else {
        LOG_INFO("Starting event playback");
        ms->startStream(); 
    }

    srandom(time(0));//we use random() to select the icon below
    WatcherGraph graph;

    unsigned int messageNumber = 0;
    MessagePtr mp;
    time_t last_output = 0; // counter to allow for writing the kml file on a fixed time interval
    bool changed = false;
    bool needTimeRange = relativeTS;

    LOG_INFO("Waiting for events ");
    while (ms->getNextMessage(mp)) {
        // std::cout << "Message #" << (++messageNumber) << ": " << *mp << std::endl; 

        if (needTimeRange) {
            PlaybackTimeRangeMessagePtr trp(boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(mp));
            if (trp.get() != 0) {
                LOG_INFO( "first offset=" << trp->min_ << ", last offset=" << trp->max_ );
                needTimeRange = false;

                Timestamp off = start_timestamp + (( start_timestamp >= 0 ) ? trp->min_ : trp->max_ );
                ms->setStreamTimeStart(off);

                LOG_INFO("Starting event playback");
                ms->startStream(); 

                continue;
            }
        }

        changed |= graph.updateGraph(mp);

        if (changed) {
            time_t now = time(0);
            if (now - last_output >= refresh) {
                graph.doMaintanence(); // expire stale links
                LOG_DEBUG("writing kml file");
                last_output = now;
                write_kml(graph, outputFile);
            }
            changed = false; // reset flag
        }
    }

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT_RET(0);
    return 0;
}
