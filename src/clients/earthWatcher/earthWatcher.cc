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
#include <map>

#include "logger.h"
#include "initConfig.h"
#include "singletonConfig.h"
#include "libwatcher/messageStream.h"
#include "libwatcher/seekWatcherMessage.h" // for SeekMessage::eof
#include "libwatcher/gpsMessage.h"
#include "libwatcher/edgeMessage.h"

//libkml
#include "kml/convenience/convenience.h"
#include "kml/engine.h"
#include "kml/dom.h"

namespace {

//arguments to getopt_long()
const option OPTIONS[] = {
    { "config", required_argument, 0, 'c' },
    { "help", no_argument, 0, 'h' },
    { "output-file", required_argument, 0, 'o' },
    { 0, 0, 0, 0 }
};

#define TOOL_NAME "earthWatcher"
const char *CONFIG_FILE = TOOL_NAME ".cfg";
const char *OUTPUT_FILE = "watcher.kml";
const char *PROPERTY_FILE = TOOL_NAME ".log.properties";

void usage()
{
    const char *SEP = "\t\t";   // separator for argument/description columns

    std::cout << "usage: earthWatcher [ -h ] [ -c FILE ]\n"
        "  -c, --config FILE" << SEP << "specify a configuration file (default: " << CONFIG_FILE << ")\n"
        "  -h, --help\t" << SEP << "display this help message\n"
        "  -o, --output-file FILE" << SEP << "specifies the output KML file (default: " << OUTPUT_FILE << ")\n"
        << std::endl;
}

} // end namespace

DECLARE_GLOBAL_LOGGER(TOOL_NAME);

using namespace watcher;

using kmldom::ChangePtr;
using kmldom::CreatePtr;
using kmldom::FeaturePtr;
using kmldom::FolderPtr;
using kmldom::KmlPtr;
using kmldom::PlacemarkPtr;
using kmldom::PointPtr;
using kmldom::UpdatePtr;
using kmlengine::KmlFile;
using kmlengine::KmlFilePtr;

int main(int argc, char **argv)
{
    TRACE_ENTER();

    const char *output_file = 0;

    for (int i; (i = getopt_long(argc, argv, "hc:o:", OPTIONS, 0)) != -1; ) {
        switch (i) {
            case 'c':
                break; //handled by initConfig()
            case 'o': // output-file
                output_file = optarg;
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

    std::string logConf(PROPERTY_FILE);
    if (!config.lookupValue("logPropertiesFile", logConf))
    {
        std::cout << "Unable to find logPropertiesFile setting in the configuration file, using default: " << logConf 
            << " and adding it to the configuration file." << std::endl;
        config.getRoot().add("logPropertiesFile", libconfig::Setting::TypeString) = logConf;
    }

    LOAD_LOG_PROPS(logConf); 
    LOG_INFO("Logger initialized from file \"" << logConf << "\"");

    std::string serverName("127.0.0.1");
    if (!config.lookupValue("server", serverName))
    {
        LOG_INFO("'server' not found in the configuration file, using default: " << serverName 
                << " and adding this to the configuration file.");
        config.getRoot().add("server", libconfig::Setting::TypeString) = serverName;
    }

    std::string service("watcherd");
    if (!config.lookupValue("service", service))
    {
        LOG_INFO("'service' not found in the configuration file, using default: " << service  
                << " and adding this to the configuration file.");
        config.getRoot().add("service", libconfig::Setting::TypeString) = service;
    }

    std::string outputFile(OUTPUT_FILE);
    if (output_file)
        outputFile = output_file; //initialize from user supplied argument
    else if (!config.lookupValue("outputFile", outputFile)) {
        LOG_INFO("'outputFile' not found in the configuration file, using default: " << outputFile  
                 << " and adding this to the configuration file.");
        config.getRoot().add("outputFile", libconfig::Setting::TypeString) = outputFile;
    }

    MessageStreamPtr ms(MessageStream::createNewMessageStream(serverName, service));
    if (!ms) {
        LOG_FATAL("Unable to create new message stream to server \"" << serverName << "\" using service (or port) \"" << service);
        TRACE_EXIT_RET(EXIT_FAILURE); 
        return EXIT_FAILURE;
    }

    MessagePtr mp;

    LOG_INFO("Starting event playback");
    ms->startStream(); 

    //libkml boilerplate
    kmldom::KmlFactory* kmlFac(kmldom::KmlFactory::GetFactory());
    kmldom::KmlPtr kml(kmlFac->CreateKml());

    //create initial DOM

    FolderPtr folder(kmlFac->CreateFolder());
    folder->set_name("Watcher");
    kml->set_feature(folder);

    //KmlFilePtr kmlFile(KmlFile::CreateFromImport(kml));

    // keep a mapping from the node to its placemark object
    typedef std::map<NodeIdentifier, PlacemarkPtr> NodeMapType;
    typedef NodeMapType::iterator NodeMapIteratorType;
    NodeMapType nodeMap;

    LOG_INFO("Waiting for events ");
    unsigned int messageNumber = 0;
    while (ms->getNextMessage(mp)) {
        std::cout << "Message #" << (++messageNumber) << ": " << *mp << std::endl; 

        //FeaturePtr obj;

        if (mp->type == GPS_MESSAGE_TYPE) {
            GPSMessagePtr gps( boost::dynamic_pointer_cast<GPSMessage>(mp) );
            assert(gps->dataFormat == GPSMessage::LAT_LONG_ALT_WGS84); //only latlon is currently supported

            PlacemarkPtr ptr;

            NodeMapIteratorType it = nodeMap.find(mp->fromNodeID);
            if (it == nodeMap.end()) {
                //not found, create a new entry
                ptr = kmlFac->CreatePlacemark();
                ptr->set_name("node");
                /*
                //id and target id are both set here
                //target id is required when changing some attribute of a feature already in the dom
                ptr->set_id("node0");
                ptr->set_targetid("node0");
                */
                nodeMap[mp->fromNodeID] = ptr;

                folder->add_feature(ptr);//add to DOM
            } else
                ptr = it->second;

            // set/update the location
            ptr->set_geometry(kmlconvenience::CreatePointLatLon(gps->x, gps->y));

            //obj = ptr;
        } else if (mp->type == EDGE_MESSAGE_TYPE) {
            EdgeMessagePtr edge( boost::dynamic_pointer_cast<EdgeMessage>(mp) );
        } else
            continue; //nothing to do

        //commented out for now since it might be possible to avoid since we can stash PlacemarkPtr's in a map
        /*
        if (true) {
            //adding a Feature
            folder->add_feature(obj);
        } else {
            // changing a Feature
            kmldom::ChangePtr change(kmlFac->CreateChange());
            change->add_object(obj);
            UpdatePtr update(kmlFac->CreateUpdate());
            update->add_updateoperation(change);
            kmlengine::ProcessUpdate(update, kmlFile);
        }
        */

        if (messageNumber == 1) //temporary for testing purposes
            break;
    }

    /*
    std::string xml;
    kmlFile->SerializeToString(&xml);
    kmlbase::File::WriteStringToFile(xml, outputFile);
    */
    kmlbase::File::WriteStringToFile(kmldom::SerializePretty(kml), outputFile);


    // Save any configuration changes made during the run.
    LOG_INFO("Saving last known configuration to " << configFilename); 
    SingletonConfig::lock();
    config.writeFile(configFilename.c_str());

    TRACE_EXIT_RET(0);
    return 0;
}
