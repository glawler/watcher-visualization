#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "client.h"
#include "labelMessage.h"

using namespace std;
using namespace watcher;
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
    fprintf(stderr, "-n, --node=address          The node to attach the label to. If no node is given, the label will float in space somewhere.\n"); 
    fprintf(stderr, "-p, --logProps              log.properties file, which controls logging for this program\n");
    fprintf(stderr, "-z, --fontSize=size         The font size of the label\n");
    fprintf(stderr, "-f, --foreground=color      The foreground color of the label. Can be ROYGBIV or RGBA format\n");
    fprintf(stderr, "-f, --background=color      The background color of the label. Can be ROYGBIV or RGBA format\n");
    fprintf(stderr, "-x, --expiration=seconds    How long in secondt to diplay the label\n");

    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    string label;
    string server;
    string logProps("log.properties");
    unsigned int fontSize=10;
    asio::ip::address address;
    Color fg=Color::black;
    Color bg=Color::white;
    uint32_t expiration=10;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"label", required_argument, 0, 'l'},
            {"server", required_argument, 0, 's'},
            {"node", required_argument, 0, 'n'},
            {"logProps", required_argument, 0, 'p'},
            {"fontSize", required_argument, 0, 'z'},
            {"foreground", required_argument, 0, 'f'},
            {"background", required_argument, 0, 'g'},
            {"expiration", required_argument, 0, 'x'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "l:s:n:p:z:f:g;x:hH?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 'l': label=optarg; break;
            case 's': server=optarg; break;
            case 'p': logProps=optarg; break;
            case 'z': fontSize=lexical_cast<unsigned int>(optarg); break;
            case 'f': fg=Color(optarg); break;
            case 'g': bg=Color(optarg); break;
            case 'x': expiration=lexical_cast<uint32_t>(optarg); break;
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
    
    LabelMessagePtr lm = LabelMessagePtr(new LabelMessage);

    lm->label=label;
    lm->fontSize=fontSize;
    lm->address=address;
    lm->foreground=fg;
    lm->background=bg;
    lm->expiration=expiration;

    if(!client.sendMessage(lm))
    {
        LOG_ERROR("Error sending label message: " << *lm);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

