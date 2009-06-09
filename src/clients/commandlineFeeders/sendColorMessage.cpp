#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/colorMessage.h>
#include <libwatcher/watcherColors.h>
#include "sendMessageHandler.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

void usage(const char *progName)
{ 
    fprintf(stderr, "Usage: %s [args]\n", basename(progName)); 
    fprintf(stderr, "Args:\n");
    fprintf(stderr, "   -c, --color=color           The color of the node. Can be ROYGBIV or RGBA format, string or hex value.\n"); 
    fprintf(stderr, "   -s, --server=address        The addres of the node running watcherd\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional args:\n");
    fprintf(stderr, "   -n, --node=address          The node to change color, if empty the local node's address is used\n"); 
    fprintf(stderr, "   -f, --flash=milliseconds    Flash between the new color and the orginal color every milliseconds seconds, 0 for no flash.\n"); 
    fprintf(stderr, "   -x, --expiration=seconds    How long in seconds to change the color. 0==forever\n"); 
    fprintf(stderr, "   -p, --logProps              log.properties file, which controls logging for this program\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -h, --help                  Show this message\n"); 

    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    Color color(Color::red);
    string server;
    asio::ip::address nodeAddr=boost::asio::ip::address::from_string("127.0.0.1"); 
    uint32_t flashTime=0;
    uint32_t expiration=0; 
    string logProps("sendMessage.log.properties");

    bool colorSet=false;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"color", required_argument, 0, 'c'},
            {"server", required_argument, 0, 's'},
            {"node", required_argument, 0, 'n'},
            {"flash", required_argument, 0, 'f'},
            {"expiration", required_argument, 0, 'x'},
            {"logProps", required_argument, 0, 'p'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "c:s:n:f:x:p:hH?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 'c': { 
                          bool val=color.fromString(optarg); 
                          if (!val) 
                          { 
                              printf("\nBad color argument: %s. Needs to be in ROYGBIV or \"0x12345678\" format.\n\n", optarg); 
                              usage(argv[0]); 
                          }
                          colorSet=true;
                      }
                      break;
            case 's': server=optarg; break;
            case 'n': 
                      {
                          boost::system::error_code e;
                          nodeAddr=asio::ip::address::from_string(optarg, e);
                          if (e)
                          {
                              fprintf(stderr, "\nI did not understand the \"node\" argument: %s. It should be a host address.\n\n", optarg);
                              usage(argv[0]);
                              TRACE_EXIT_RET(EXIT_FAILURE);
                              return EXIT_FAILURE;
                          }
                      }
                      break;
            case 'f': {
                          try
                          {
                              flashTime=lexical_cast<uint32_t>(optarg); break;
                          }
                          catch (const boost::bad_lexical_cast &e)
                          {
                              fprintf(stderr, "\nBad flash argument: %s. It needs to be a number\n\n", optarg);
                              usage(argv[0]);
                          }
                      }
                      break;
            case 'x': {
                          try
                          {
                              expiration=lexical_cast<uint32_t>(optarg); break;
                          }
                          catch (const boost::bad_lexical_cast &e)
                          {
                              fprintf(stderr, "\nBad expiration argument: %s. It needs to be a number\n\n", optarg);
                              usage(argv[0]);
                          }
                      }
                      break;
            case 'p': logProps=optarg; break;
            case 'h':
            case 'H':
            case '?':
            default:
                      usage(argv[0]); 
                      break;
        }
    }

    if (server=="" || !colorSet)
    {
        usage(argv[0]);
        exit(1); 
    }

    //
    // Now do some actual work.
    // 
    LOAD_LOG_PROPS(logProps);

    watcher::Client client(server); 
    LOG_INFO("Connecting to " << server << " and sending message."); 
    client.addMessageHandler(SendMessageHandler::create());

    ColorMessagePtr cm(new ColorMessage);

    cm->color=color;
    cm->fromNodeID=nodeAddr;
    cm->flashPeriod=flashTime;
    cm->expiration=expiration;

    if(!client.sendMessage(cm))
    {
        LOG_ERROR("Error sending color message: " << *cm);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    client.wait();

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

