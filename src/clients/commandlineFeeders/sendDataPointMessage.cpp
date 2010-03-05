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
 * @file sendDataPointMessage.cpp
 *
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15
 *
 * @page sendDataPointMessage 
 *
 * sendDataPointMessage is a test node command line program that sends a watcher::event::DataPointMessage message to the watcher daemon, specifing a set of timestamped data point(s)
 * for the node. The data point is labeled with a string saying what the data points represent. 
 *
 * Usage: 
 * @{
 * <b>sendDataPointMessage -s server -g name [optional args] -d dp1 -d dp2 ... -d dpN</b>
 * @}
 * @{
 * Args:
 * @arg <b>-s address|name</b>, The address or name of the node running watcherd
 * @arg <b>-g name</b>, the "graphname" of the data - what the data is measuring
 * @arg <b>-d datapoint</b>, a single data point measuring something
 * @}
 * Optional args:
 * @arg <b>-n, --node=address</b>, the node the data is from 
 * @arg <b>-h, --help</b>, Show help message
 *
 * Examples:
 * @{
 *
 * @code 
 * sendDataPointMessage -s glory -g "CPU Usage" -n 192.168.1.105 -d .45432
 * sendDataPointMessage -s glory -g "Logged In Users" -n 192.168.1.105 -d 23
 *
 * @endcode
 *
 * @}
 */
#include <stdio.h>
#include <string>
#include <boost/lexical_cast.hpp>

/* Test program for feeding the scrolling graphs in the (new old) watcher.
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#include <libwatcher/watcherTypes.h>
#include <libwatcher/client.h>
#include <libwatcher/dataPointMessage.h>
#include <libwatcher/sendMessageHandler.h>

#include "logger.h"

DECLARE_GLOBAL_LOGGER("sendDataPointMessage"); 

using namespace watcher;
using namespace watcher::event;
using namespace boost;
using namespace std;

void usage(const char *name);
void usage(const char *name)
{
    fprintf(stderr,"%s -s addr [-n addr] -g name -d pt1 -d pt2 ... -d ptN\n"
            "-s IPaddr - watcherd server to connect to\n"
            "-n IPaddr - node to affect - if not given, effect the local node\n"
            "-g string (graph name)\n"
            "-d double (data point), this argument can be given multiple times for multiple data points, if needed\n"
            "-l file - the log.property file. If not given, looks  for \"sendMessage.log.properties\"\n",
            basename(name)); 
}

int main(int argc, char *argv[])
{
    string serverName;
    DataPointMessagePtr message(new DataPointMessage);
    string logPropsFile("sendMessage.log.properties");
    int ch;

	while ((ch = getopt(argc, argv, "s:g:d:n:l:hH?")) != -1)
		switch (ch)
        {
            case 's':
                serverName=optarg; 
                break;
            case 'd':
                {
                    try {
                        message->dataPoints.push_back(lexical_cast<double>(optarg));
                    }
                    catch (bad_lexical_cast &) {
                        usage(argv[0]);
                        fprintf(stderr, "\nI have no idea what %s is, but I can tell you it's not a double.\n", optarg);
                        return EXIT_FAILURE;
                    }
                }
                break;
            case 'g':
                message->dataName=optarg;
                break;
            case 'n': 
                message->fromNodeID=NodeIdentifier::from_string(optarg);
                break;
            case 'l':
                logPropsFile=optarg;
                break;
            default:
            case '?':
            case 'H':
            case 'h':
                usage(argv[0]);
                return EXIT_FAILURE;
                break;
        }

    if (message->dataName.empty())
    {
        usage(argv[0]);
        fprintf(stderr, "\nThe -g string (graphname) argument is mandatory\n"); 
        return EXIT_FAILURE;
    }

    if (!message->dataPoints.size())
    {
        usage(argv[0]);
        fprintf(stderr, "\nWhat's the use of sending a datapoint message without any data points? What is the sound of no data being sent?\n");
        return EXIT_FAILURE;
    }

    LOAD_LOG_PROPS(logPropsFile.c_str());

    watcher::Client client(serverName);
    client.addMessageHandler(SingleMessageHandler::create());

    printf("Connecting to %s and sending message.\n", serverName.c_str());

    if(!client.sendMessage(message))
    {
        LOG_ERROR("Error sending data point message: " << *message);
        return EXIT_FAILURE;
    }

    client.wait();

	return EXIT_SUCCESS;
}
