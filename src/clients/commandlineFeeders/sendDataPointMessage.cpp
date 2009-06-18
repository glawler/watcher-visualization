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
#include <sendMessageHandler.h>

#include "logger.h"

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
    client.addMessageHandler(SendMessageHandler::create());

    printf("Connecting to %s and sending message.\n", serverName.c_str());

    if(!client.sendMessage(message))
    {
        LOG_ERROR("Error sending data point message: " << *message);
        return EXIT_FAILURE;
    }

    client.wait();

	return EXIT_SUCCESS;
}
