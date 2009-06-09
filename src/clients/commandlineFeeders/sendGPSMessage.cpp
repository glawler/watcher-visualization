#include <errno.h>
#include <stdlib.h>
#include <getopt.h>

#include <boost/asio.hpp>
#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/watcherTypes.h> // for NodeIdentifer
#include "sendMessageHandler.h"

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
    LOAD_LOG_PROPS(logProps ? logProps : "sendMessage.log.properties");

    watcher::Client client(serverName); 
    printf("Connecting to %s and sending message.\n", serverName);
    client.addMessageHandler(SendMessageHandler::create());
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

