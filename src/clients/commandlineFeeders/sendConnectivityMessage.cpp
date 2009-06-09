#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <boost/foreach.hpp>
#include <boost/asio/ip/address.hpp>

#include <libwatcher/connectivityMessage.h>
#include <libwatcher/client.h>
#include "logger.h"
#include "sendMessageHandler.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

void usage(const char *progName)
{
    fprintf(stderr, "Usage: %s -s|--servername server [-l|--logProps log.propertiesFile] [-f|--fromNode fromNodeAddr] nbr1 nbr2 ... nbrN\n", basename(progName)); 
    fprintf(stderr, "Where:\n"); 
    fprintf(stderr, "\tnbr1 nbr2 ... nbrN is a list of ip addresses that the node is connected to.\n");
    fprintf(stderr, "\tserver is the server address or hostname\n"); 
    fprintf(stderr, "\tfromNodeAddr is the address of the node the neighbors are attached to.\n"); 

    exit(1); 
}

int main(int argc, char **argv)
{
    int c;
    char *serverName=NULL;
    char *logProps=NULL;
    boost::asio::ip::address_v4 fromNodeAddr;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"logProps", required_argument, 0, 'l'},
            {"servername", required_argument, 0, 's'},
            {"fromNode", required_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "l:s:f:hH?", long_options, &option_index);

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
            case 'f':
                fromNodeAddr=boost::asio::ip::address_v4::from_string(optarg);
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

    LOG_DEBUG("Args: server: " << serverName << " fromAddr: " << fromNodeAddr << " logProps: " << logProps); 

    watcher::Client client(serverName); 
    client.addMessageHandler(SendMessageHandler::create());
    printf("Connecting to %s and sending message.\n", serverName);

    ConnectivityMessagePtr cm(new ConnectivityMessage); 
    if(fromNodeAddr!=boost::asio::ip::address_v4())
    {
        LOG_DEBUG("Setting from address on message to " << fromNodeAddr); 
        cm->fromNodeID=fromNodeAddr;
    }

    for(int i=optind; i<argc; i++)
    {
        boost::system::error_code ec;
        NodeIdentifier addr=NodeIdentifier::from_string(argv[i], ec);
        if(!ec)
            cm->neighbors.push_back(addr);
        else
            LOG_ERROR("Ignoring non address arguement: " << argv[i]);
    }

    LOG_DEBUG("Sending Message: " << *cm); 

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

