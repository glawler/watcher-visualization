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
 * @file sendGPSMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 *
 * @page sendGPSMessage 
 *
 * sendGPSMessage is a test node command line program that sends a watcher::event::GPSMessage message to a watcher daemon, specifing a node's current GPS coordinates.
 *
 * Usage: 
 * @{
 * <b>sendGPSMessage -s server -x value -y value -z value [optional args]</b>
 * @}
 * @{
 * Args:
 * @arg <b>-s, --server=address</b>, The address or name of the node running watcherd to which the message is sent.
 * @arg <b>-x, --latitude=value</b>, The latitude of the node.
 * @arg <b>-y, --longitude=value</b>, The longitude of the node.
 * @arg <b>-z, --altitude=value</b>, The altitude of the node.
 * @}
 * Optional args:
 * @arg <b>-n, --fromNode=address|name</b>, The node that the coordinates refer to. If not given, assume the local node. 
 * @arg <b>-l, --logProps</b>, log.properties file, which controls logging for this program
 * @arg <b>-h, --help</b>, Show help message
 *
 * @{
 * Examples:
 * @{
 *
 * This tells the GUI(s) attached to the watcherd on glory that node 192.168.1.101 is now at 79.23123123, 43.123123123, 20
 * @code 
 * sendGPSMessage -s glory -n 192.168.1.101 -x 79.23123123 -y 43.123123123 -z 20
 * @endcode
 *
 * This tells the GUI(s) attached to the watcherd on glory that the local node is at 0.0123123 0.123123123 123
 * @code 
 * sendGPSMessage --server glory -n 192.168.1.101 -x 0.0123123 -y 0.123123123 -z 123
 * @endcode
 * @}
 * @}
 */
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>

#include <boost/asio.hpp>
#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/watcherTypes.h> // for NodeIdentifer
#include <libwatcher/sendMessageHandler.h>

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

DECLARE_GLOBAL_LOGGER("sendGPSMessage"); 

using namespace std;
using namespace watcher;
using namespace watcher::event;

void usage(const char *progName)
{
    fprintf(stderr, "Usage: %s -x val -y val -z val -s|servername server [-n|--fromNode=ipaddress][-l|logProps log.propertiesFile]\n", basename(progName)); 
    fprintf(stderr, "Where:\n");
    fprintf(stderr, "   x == longitude, y == latitude, and z == altitude\n"); 
    fprintf(stderr, "   s|servername == the server address or hostname\n"); 
    fprintf(stderr, "   n|fromNode == The address of the node of the GPS data. Localhost is default\n"); 
    fprintf(stderr, "   l|logProps == The file to read the logging properties from\n"); 

    exit(1); 
}

double str2double(const char *arg, const char *name)
{
    TRACE_ENTER();

    double retVal = strtod(arg, NULL);
    if(errno == EINVAL || errno == ERANGE)
    {
        fprintf(stderr, "Unable to make sense of the %s argument '%s'. It should be a double.", name, arg);
        return 0.0;
    }
    return retVal;

    TRACE_EXIT();
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    char *serverName=NULL;
    char *logProps=NULL;

    double latitude, longitude, altitude;
    latitude = longitude = altitude = 0.0;
    NodeIdentifier nodeAddr; 

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"logProps", required_argument, 0, 'l'},
            {"servername", required_argument, 0, 's'},
            {"latitude", required_argument, 0, 'x'},
            {"longitude", required_argument, 0, 'y'},
            {"altitude", required_argument, 0, 'z'},
            {"fromNode", required_argument, 0, 'n'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "l:s:x:y:z:n:hH?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 'l': 
                logProps = strdup(optarg); // buffer overflow here. :) 
                break;
            case 's':
                serverName = strdup(optarg);  // buffer overflow here :) 
                break;
            case 'x':
                latitude = str2double(optarg, "latitude (x)"); 
                break;
            case 'y':
                longitude = str2double(optarg, "longitude (y)"); 
                break;
            case 'z': 
                altitude = str2double(optarg, "altitude (z)"); 
                break;
            case 'n':
                {
                    boost::system::error_code ec;
                    nodeAddr=NodeIdentifier::from_string(optarg, ec);
                    if (ec)
                    {
                        fprintf(stderr, "I don't understand the fromNode argument, \"%s\". It must be an ip address.\n", optarg); 
                        exit(0);
                    }
                }
                break;
            case 'h':
            case 'H':
            case '?':
            default:
                usage(argv[0]); 
                break;
        }
    }

    if (!serverName)
    {
        usage(argv[0]);
        if (serverName)
            free(serverName);
        if (logProps)
            free(logProps);
        exit(1); 
    }

    //
    // Now do some actual work.
    // 
    printf("Initializing logging system\n"); 
    LOAD_LOG_PROPS(logProps ? logProps : SYSCONFDIR "/watcher.log.props");

    watcher::Client client(serverName); 
    printf("Connecting to %s and sending message.\n", serverName);
    client.addMessageHandler(SingleMessageHandler::create());
    GPSMessagePtr message = GPSMessagePtr(new GPSMessage(latitude, longitude, altitude));

    if(NodeIdentifier()!=nodeAddr) // not empty address
    {
        LOG_INFO("Using " << nodeAddr << " as the node's address");
        message->fromNodeID=nodeAddr;
    }

    if(!client.sendMessage(message))
    {
        LOG_ERROR("Error sending gps message: " << *message);
        free(serverName);
        if (logProps)
            free(logProps);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    client.wait();

    free(serverName);
    if (logProps)
        free(logProps);

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

