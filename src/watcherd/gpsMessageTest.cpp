#include <errno.h>
#include <stdlib.h>
#include <getopt.h>

#include <boost/asio.hpp>
#include "logger.h"
#include "client.h"
#include <libwatcher/gpsMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;

void usage(const char *progName)
{
    fprintf(stderr, "Usage: %s -x val -y val -z val -s|servername server [-l|logProps log.propertiesFile]\n", basename(progName)); 
    fprintf(stderr, "Where x == longitude, y == latitude, and z == altitude\n"); 
    fprintf(stderr, "   and s|servername == the server address or hostname\n"); 

    exit(1); 
}

float str2float(const char *arg, const char *name)
{
    TRACE_ENTER();

    float retVal = strtod(arg, NULL);
    if(errno == EINVAL || errno == ERANGE)
    {
        fprintf(stderr, "Unable to make sense of the %s argument '%s'. It should be a float.", name, arg);
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

    float latitude, longitude, altitude;
    latitude = longitude = altitude = 0.0;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"logProps", required_argument, 0, 'l'},
            {"servername", required_argument, 0, 's'},
            {"latitude", required_argument, 0, 'x'},
            {"longitude", required_argument, 0, 'y'},
            {"altitude", required_argument, 0, 'z'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "l:s:x:y:z:hH?", long_options, &option_index);

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
                latitude = str2float(optarg, "latitude (x)"); 
                break;
            case 'y':
                longitude = str2float(optarg, "longitude (y)"); 
                break;
            case 'z': 
                altitude = str2float(optarg, "altitude (z)"); 
                break;
            case 'h':
            case 'H':
            case '?':
            default:
                usage(argv[0]); 
                break;
        }
    }

    if (latitude == 0.0 || longitude == 0.0 || altitude == 0.0 || !serverName)
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
    LOAD_LOG_PROPS(logProps ? logProps : "log.properties");

    watcher::Client client(serverName); 
    printf("Connecting to %s and sending message.\n", serverName);
    GPSMessagePtr message = GPSMessagePtr(new GPSMessage(latitude, longitude, altitude));

    if(!client.sendMessage(message))
    {
        LOG_ERROR("Error sending gps message: " << *message);
        free(serverName);
        if (logProps)
            free(logProps);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    // Messages are sent and recv'd async and there is no current method to 
    // wait until the sequence is done. So sleep for awhile. 
    // sleep(10); 

    free(serverName);
    if (logProps)
        free(logProps);

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

