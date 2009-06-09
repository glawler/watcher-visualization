#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/labelMessage.h>
#include "sendMessageHandler.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

void usage(const char *progName)
{ 
    fprintf(stderr, "Usage: %s [args] [optional args]\n", basename(progName)); 
    fprintf(stderr, "Args:\n");
    fprintf(stderr, "   -l, --label=label        The text to put in the label\n");
    fprintf(stderr, "   -s, --server=server      The name/address of the watcherd server\n");
    fprintf(stderr, "   -h,-H,-?,-help           Show this usage message\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional args:\n");
    fprintf(stderr, "   If address is specified, the label will attach to the node with that address. If cooridinates are\n");
    fprintf(stderr, "   specified, the label will float at those coordinates. The node address takes precedence. If neither\n"); 
    fprintf(stderr, "   option is specified, the label will attach to the local node in the watcher.\n"); 
    fprintf(stderr, "   -n, --node=address          The node to attach the label to.\n"); 
    fprintf(stderr, "   -x, --latitude=coord        The latitude to float the node at.\n"); 
    fprintf(stderr, "   -y, --longitude=coord       The longitude to float the node at.\n"); 
    fprintf(stderr, "   -z, --altitiude=coord       The altitude to float the node at.\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "   -p, --logProps              log.properties file, which controls logging for this program\n");
    fprintf(stderr, "   -t, --fontSize=size         The font size of the label\n");
    fprintf(stderr, "   -f, --foreground=color      The foreground color of the label. Can be ROYGBIV or RGBA format, string or hex value.\n"); 
    fprintf(stderr, "   -b, --background=color      The background color of the label. Can be ROYGBIV or RGBA format, string or hex value.\n");
    fprintf(stderr, "   -e, --expiration=seconds    How long in secondt to diplay the label\n");
    fprintf(stderr, "   -r, --remove                Remove the label if it is attached\n"); 
    fprintf(stderr, "   -L, --layer=layer           Which layer the label is part of. Default is physical\n"); 

    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    string label;
    string server;
    string logProps("sendMessage.log.properties");
    unsigned int fontSize=10;
    asio::ip::address address;
    Color fg=Color::black;
    Color bg=Color::white;
    uint32_t expiration=10000;
    float lat=0.0, lng=0.0, alt=0.0;
    bool remove=false;
    GUILayer layer=PHYSICAL_LAYER;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"label", required_argument, 0, 'l'},
            {"server", required_argument, 0, 's'},
            {"node", required_argument, 0, 'n'},
            {"latitude", required_argument, 0, 'x'},
            {"longitude", required_argument, 0, 'y'},
            {"altitiude", required_argument, 0, 'z'},
            {"logProps", required_argument, 0, 'p'},
            {"fontSize", required_argument, 0, 't'},
            {"foreground", required_argument, 0, 'f'},
            {"background", required_argument, 0, 'b'},
            {"expiration", required_argument, 0, 'e'},
            {"remove", no_argument, 0, 'r'},
            {"layer", required_argument, 0, 'L'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "l:s:n:x:y:t:p:z:f:b:e:L:rhH?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 'l': label=optarg; break;
            case 's': server=optarg; break;
            case 'p': logProps=optarg; break;
            case 't': fontSize=lexical_cast<unsigned int>(optarg); break;
            case 'f': { bool val=fg.fromString(optarg); if (!val) { printf("\nBad argument for fg color\n\n"); usage(argv[0]); } break; }
            case 'g': { bool val=bg.fromString(optarg); if (!val) { printf("\nBad argument for bg color\n\n"); usage(argv[0]); } break; }
            case 'e': expiration=lexical_cast<uint32_t>(optarg); break;
            case 'n': 
                      {
                          boost::system::error_code e;
                          address=asio::ip::address::from_string(optarg, e);
                          if (e)
                          {
                                fprintf(stderr, "\nI did not understand the \"node\" argument: %s. It should be a host address.\n\n", optarg);
                                usage(argv[0]);
                                TRACE_EXIT_RET(EXIT_FAILURE);
                                return EXIT_FAILURE;
                          }
                      }
                      break;
            case 'x': lat=lexical_cast<float>(optarg); break; // GTL should try{}catch{} here for invalid values.
            case 'y': lng=lexical_cast<float>(optarg); break; // GTL should try{}catch{} here for invalid values.
            case 'z': alt=lexical_cast<float>(optarg); break; // GTL should try{}catch{} here for invalid values.
            case 'r': remove=true; break;
            case 'L': label=optarg; break;
            case 'h':
            case 'H':
            case '?':
            default:
                usage(argv[0]); 
                break;
        }
    }

    if (server=="" || label=="")
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
    
    LabelMessagePtr lm = LabelMessagePtr(new LabelMessage);

    lm->label=label;
    lm->fontSize=fontSize;
    lm->fromNodeID=address;
    lm->foreground=fg;
    lm->background=bg;
    lm->expiration=expiration;
    lm->lat=lat;
    lm->lng=lng;
    lm->alt=alt;
    lm->addLabel=!remove;
    lm->layer=layer;

    if(!client.sendMessage(lm))
    {
        LOG_ERROR("Error sending label message: " << *lm);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    client.wait(); 

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

