#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <boost/foreach.hpp>

#include <libwatcher/connectivityMessage.h>
#include "logger.h"
#include "client.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

void usage(const char *progName)
{
    fprintf(stderr, "Usage: %s -s|servername server [-l|logProps log.propertiesFile] nbr1 nbr2 ... nbrN\n", basename(progName)); 
    fprintf(stderr, "Where nbrX is an ipaddress of a nore this node is connected to\n"); 
    fprintf(stderr, "   and s|servername == the server address or hostname\n"); 

    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    char *serverName=NULL;
    char *logProps=NULL;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"logProps", required_argument, 0, 'l'},
            {"servername", required_argument, 0, 's'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "l:s:hH?", long_options, &option_index);

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
    LOAD_LOG_PROPS(logProps ? logProps : "log.properties");

    watcher::Client client(serverName); 
    printf("Connecting to %s and sending message.\n", serverName);

    ConnectivityMessagePtr cm(new ConnectivityMessage); 

    vector<NodeIdentifier> nbrs;
    for(int i=optind; i<argc; i++)
    {
        boost::system::error_code ec;
        NodeIdentifier addr=NodeIdentifier::from_string(argv[i], ec);
        if(!ec)
            nbrs.push_back(addr);
        else
            LOG_ERROR("Ignoring non address arguement: " << argv[i]);
    }

    LOG_INFO("Sending this neighbor list to watcherd:"); 
    BOOST_FOREACH(NodeIdentifier a, nbrs)
        LOG_INFO("  " << a); 

    if(!client.sendMessage(cm))
    {
        LOG_ERROR("Error sending gps message: " << *cm);
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

