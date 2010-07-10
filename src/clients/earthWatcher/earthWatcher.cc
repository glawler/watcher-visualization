/* Copyright 2009,2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

namespace earthwatcher {
    void write_kml(const WatcherGraph& graph, const std::string& outputFile); // kml.cc

    float LayerPadding = 10;
    float Lonoff = 0.0;
    float Latoff = 0.0;
    float Altoff = 0.0;
    float IconScale = 1.0;
    int SplineSteps = 2;
    bool autoRewind = false;
    std::string IconPath ( "placemark_circle.png") ;
}

using namespace earthwatcher;

namespace {

//arguments to getopt_long()
const option OPTIONS[] = {
    { "latoff", required_argument, 0, 'a' },
    { "altoff", required_argument, 0, 'A' },
    { "config", required_argument, 0, 'c' },
    { "description", required_argument, 0, 'D' },
    { "help", no_argument, 0, 'h' },
    { "output", required_argument, 0, 'o' },
    { "lonoff", required_argument, 0, 'O' },
    { "refresh", required_argument, 0, 'r' },
    { "seek", required_argument, 0, 'S' },
    { "server", required_argument, 0, 's' },
    { "speed", required_argument, 0, 'd' },
    { "steps", required_argument, 0, 't' },
    { "icon-scale", required_argument, 0, 'i' },
    { "icon-path", required_argument, 0, 'I' },
    { "join", required_argument, 0, 'j' },
    { 0, 0, 0, 0 }
};

const char *CONFIG_FILE = TOOL_NAME ".cfg";
const char *OUTPUT_FILE = "watcher.kml";
const char *PROPERTY_FILE = TOOL_NAME ".log.properties";
const char *DEFAULT_HOST = "127.0.0.1";
const unsigned int DEFAULT_REFRESH = 1; // SECONDS
uint32_t StreamUID = -1;
uint32_t maxNodes = 0; 
uint32_t maxLayers = 0; 
const char *DEFAULT_DESCRIPTION = "earthWatcher client";

void usage()
{
    const char *SEP = "\t\t";   // separator for argument/description columns

    std::cout << "usage: earthWatcher [ -h ] [ -c FILE ]\n"
        "  -a, --latoff OFF" << SEP << "translate GPS coordinates relative to a given latitude\n"
        "  -A, --altoff OFF" << SEP << "translate GPS coordinates relative to the given altitude\n"
        "  -c, --config FILE" << SEP << "specify a configuration file (default: " << CONFIG_FILE << ")\n"
        "  -d, --speed SPEED" << SEP << "specify the event playback rate (default: 1.0)\n"
        "  -D, --description NAME" << SEP << "set the description for the event stream\n"
        "  -h, --help\t" << SEP << "display this help message\n"
        "  -i, --icon-scale=NUM" << SEP << "adjust the size of node icons\n"
        "  -j, --join UID" << SEP << "subscribe to a synchronized stream on the watcher server by UID\n"
        "  -l, --maxLayers=NUM" << SEP << "maximum number of layers used in the test run\n"
        "  -I, --icon-path=STR" << SEP << "specify the node icon to use (default: " << IconPath << ")\n"
        "  -n, --maxNodes=NUM" << SEP << "maximum number of nodes used in the test run\n"
        "  -o, --output FILE" << SEP << "specifies the output KML file (default: " << OUTPUT_FILE << ")\n"
        "  -O, --lonoff OFF" << SEP << "translate GPS coordinates relative to a given longitude\n"
        "  -r, --refresh SECS" << SEP << "write the the output every SECS seconds (default: " << DEFAULT_REFRESH << ")\n"
        "  -s, --server HOST" << SEP << "connect to the watcher server on the given host (default: " << DEFAULT_HOST << ")\n"
        "  -S, --seek POS" << SEP << "start event playback at timestamp POS (default: -1)\n"
        "\tPOS may be specified relative to the first and last timestamps in the Watcher database by prefixing the offset with + or -\n"
        "\tExample: +5000 means 5 seconds after the first event in the database.\n"
        "  -t, --steps NUM" << SEP << "number of points to use when drawing a spline (default: " << SplineSteps << ")\n"
        << std::endl;
}

sigset_t Signals;

void init_signals()
{
    if (sigemptyset(&Signals) ||
        sigaddset(&Signals, SIGHUP) ||
        sigaddset(&Signals, SIGTERM) ||
        sigaddset(&Signals, SIGINT)) {
        throw std::runtime_error("failed to initialiaze sigset_t");
    }
    pthread_sigmask(SIG_BLOCK, &Signals, 0);
}

// Global variables shared between main() and process_events()
std::string ServerName(DEFAULT_HOST);
std::string Service("watcherd");
std::string ConfigFilename; //filled by initConfig()
std::string OutputFile(OUTPUT_FILE);
Timestamp StartTimestamp = -1 ; // default to EOF (live playback)
unsigned int Refresh = DEFAULT_REFRESH;
float Speed = 1.0;
bool RelativeTS = false; // when true, start_timestamp is relative to first or last event in the Watcher DB
std::string Description(DEFAULT_DESCRIPTION); // what this client calls itself

void *process_events(void *)
{
    // open a message stream of live events for now
    MessageStreamPtr ms(MessageStream::createNewMessageStream(ServerName, Service, StartTimestamp, Speed));
    if (!ms) {
        LOG_FATAL("Unable to create new message stream to server \"" << ServerName << "\" using service (or port) \"" << Service);
        TRACE_EXIT_RET(EXIT_FAILURE); 
        exit( EXIT_FAILURE );
    }

    if (RelativeTS) {
        ms->getMessageTimeRange();
    } else {
	if (StreamUID != -1) {
	    LOG_INFO("Joining stream uid " << StreamUID);
	    ms->subscribeToStream(StreamUID);
	} else {
	    LOG_INFO("Setting stream description to: " << Description);
	    ms->setDescription(Description);
	}
        LOG_INFO("Starting event playback");
        ms->startStream(); 
    }

    WatcherGraph graph(maxNodes, maxLayers);

    unsigned int messageNumber = 0;
    MessagePtr mp;
    time_t last_output = 0; // counter to allow for writing the kml file on a fixed time interval
    bool changed = false;
    bool needTimeRange = RelativeTS;

    watcher::Timestamp currentMessageTimestamp; 
    watcher::Timestamp playbackRangeEnd; 

    const char *cfname = ConfigFilename.c_str();
    struct stat sb;
    time_t cftime;      // time at which config file was last modified
    if (stat(cfname, &sb) == -1) {
        LOG_ERROR("stat: " << strerror(errno));
        exit( EXIT_FAILURE );
    }
    cftime = sb.st_mtime;

    LOG_INFO("Waiting for events ");
    while (ms->getNextMessage(mp)) {

        if (needTimeRange) {
            PlaybackTimeRangeMessagePtr trp(boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(mp));
            if (trp.get() != 0) {
                LOG_INFO( "first offset=" << trp->min_ << ", last offset=" << trp->max_ );
                needTimeRange = false;
                playbackRangeEnd = trp->max_;
                currentMessageTimestamp=trp->min_;

                Timestamp off = StartTimestamp + ((StartTimestamp >= 0 ) ? trp->min_ : trp->max_ );
                ms->setStreamTimeStart(off);

                LOG_INFO("Starting event playback at time " << off);
                ms->startStream(); 
                continue;
            }
        }

        changed |= graph.updateGraph(mp);

        if (changed) {
            time_t now = time(0);
            if (now - last_output >= Refresh) {
                // reload config file if changed
                if (stat(cfname, &sb) == -1) {
                    LOG_ERROR( "stat: " << strerror(errno) );
                    exit( EXIT_FAILURE) ;
                }
                if (sb.st_mtime > cftime) {
                    cftime = sb.st_mtime;
                    LOG_INFO("reloading configuration file");
                    SingletonConfig::lock();
                    SingletonConfig::instance().readFile(cfname);
                    SingletonConfig::unlock();
                }

                graph.doMaintanence(); // expire stale links
                LOG_DEBUG("writing kml file");
                last_output = now;
                write_kml(graph, OutputFile);
            }
            changed = false; // reset flag
        }

        if (autoRewind) {
            currentMessageTimestamp=mp->timestamp;
            // LOG_DEBUG("end of data in: " << playbackRangeEnd-currentMessageTimestamp); 
            if (currentMessageTimestamp==playbackRangeEnd) {
                LOG_WARN("Autorewind engaged - jumping to start of data...");
                if (RelativeTS) {
                    LOG_INFO("Starting event playback from relative start");
                    needTimeRange = true;
                    ms->stopStream();
                    ms->clearMessageCache();
                    ms->getMessageTimeRange();
                }
            }
        }
    }

    return 0;
}

} // end namespace

int main(int argc, char **argv)
{
    TRACE_ENTER();

    const char *output_file = 0;
    unsigned int args = 0;
    enum {
        argLayerPadding = (1<<0),
        argLonoff = (1<<1),
        argLatoff = (1<<2),
        argAltoff = (1<<3),
        argOutputFile = (1<<4),
        argServerName = (1<<5),
        argSplineSteps = (1<<6),
        argIconPath = (1<<7),
        argJoin = (1<<8),
        argDescription = (1<<9),
        argMaxNodes = (1<<0xa),
        argMaxLayers = (1<<0xb),

    };

    for (int i; (i = getopt_long(argc, argv, "a:A:hi:j:I:c:d:D:l:n:o:O:r:s:S:t:", OPTIONS, 0)) != -1; ) {
        switch (i) {
            case 'c':
                break; //handled by initConfig()

            case'D':
                Description = optarg;
                break;

            case 'a':
                Latoff = atof(optarg);
                args |= argLatoff;
                break;

            case 'A':
                Altoff = atof(optarg);
                args |= argAltoff;
                break;

            case 'd':
                Speed = atof(optarg);
                break;

            case 'i':
                IconScale = atof(optarg);
                break;

            case 'j':
                StreamUID = strtol(optarg, NULL, 10);
                args |= argJoin;
                break;

            case 'l':
                maxLayers = strtol(optarg, NULL, 10); 
                args |= argMaxLayers;
                break;

            case 'I':
                IconPath = optarg;
                args |= argIconPath;
                break;

            case 'n':
                maxNodes = strtol(optarg, NULL, 10); 
                args |= argMaxNodes;
                break;

            case 'o': // output-file
                OutputFile = optarg;
                args |= argOutputFile;
                break;

            case 'O':
                Lonoff = atof(optarg);
                args |= argLonoff;
                break;

            case 'r':
                Refresh = atoi(optarg);
                break;

            case 'S':
                RelativeTS = (optarg[0] == '+' || optarg[0] == '-'); // when +/- is prefix, seek relative to starting/ending event
                StartTimestamp = atol(optarg);
                if (StartTimestamp == -1)
                    RelativeTS = false; // special case for EOF value
                break;

            case 's':
                ServerName = optarg;
                args |= argServerName;
                break;

            case 't':
                SplineSteps = atoi(optarg);
                if (SplineSteps < 2) {
                    std::cerr << "error: number of steps can not be less than 2" << std::endl;
                    return EXIT_FAILURE;
                }
                args |= argSplineSteps;
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
    if (! watcher::initConfig(config, argc, argv, ConfigFilename))
        std::cout << "Configuration file not found. Creating new configuration file and using default runtime values." << std::endl;
    SingletonConfig::setConfigFile(ConfigFilename);
    SingletonConfig::unlock();

    /* handle log.properties file as a special case */
    std::string logConf(PROPERTY_FILE);
    if (!config.lookupValue("logProperties", logConf)) {
        config.getRoot().add("logProperties", libconfig::Setting::TypeString) = logConf;
    }
    LOAD_LOG_PROPS(logConf); 
    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    struct {
        const char *configName;
        std::string *value;
        unsigned int bit;
    } ConfigString[] = {
        { "server", &ServerName, argServerName },
        { "service", &Service, 0 },
        { "outputFile", &OutputFile, argOutputFile },
        { "iconPath", &IconPath, argIconPath },
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

    struct {
        const char *configName;
        int* value;
        unsigned int bit;
    } ConfigInt[] = {
        { "splineSteps", &SplineSteps, argSplineSteps },
        { "maxNodes", (int*)&maxNodes, argMaxNodes },
        { "maxLayers", (int*)&maxLayers, argMaxLayers },
        { 0, 0, 0 } // terminator
    };

    for (size_t i = 0; ConfigInt[i].configName != 0; ++i) {
        if ((args & ConfigInt[i].bit) == 0 && !config.lookupValue(ConfigInt[i].configName, *ConfigInt[i].value)) {
            LOG_INFO("'" << ConfigInt[i].configName << "' not found in the configuration file, using default: " << *ConfigInt[i].value
                     << " and adding this to the configuration file.");
            config.getRoot().add(ConfigInt[i].configName, libconfig::Setting::TypeInt) = *ConfigInt[i].value;
        }
    }

    if (!maxLayers || !maxNodes) { 
        LOG_FATAL("You must specify both maxNodes and maxLayers in the cfg file.");
        TRACE_EXIT_RET(EXIT_FAILURE);
        exit (EXIT_FAILURE);
    }

    struct {
        const char *configName;
        bool* value;
        unsigned int bit;
    } ConfigBool[] = {
        { "autorewind", &autoRewind, false },
        { 0, 0, 0 } // terminator
    };

    for (size_t i = 0; ConfigBool[i].configName != 0; ++i) {
        if ((args & ConfigBool[i].bit) == 0 && !config.lookupValue(ConfigBool[i].configName, *ConfigBool[i].value)) {
            LOG_INFO("'" << ConfigBool[i].configName << "' not found in the configuration file, using default: " << *ConfigBool[i].value
                     << " and adding this to the configuration file.");
            config.getRoot().add(ConfigBool[i].configName, libconfig::Setting::TypeBoolean) = *ConfigBool[i].value;
        }
    }

    init_signals();

    // spawn thread to receive Watcher events
    pthread_t tid;
    pthread_create(&tid, 0, process_events, 0);

    // wait for user interrupt
    int caughtSig;
    int r = sigwait(&Signals, &caughtSig);
    if (r == -1) {
        throw std::runtime_error("sigwait returned -1");
    }
    LOG_INFO("caught signal " << caughtSig);

    pthread_cancel(tid);

    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << ConfigFilename); 
    SingletonConfig::saveConfig();
    LOG_INFO("Done.");

    TRACE_EXIT_RET(EXIT_SUCCESS);
    exit( EXIT_SUCCESS );
}
