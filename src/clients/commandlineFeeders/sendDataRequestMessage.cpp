#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/dataRequestMessage.h>
#include <libwatcher/messageTypesAndVersions.h>
#include "sendMessageHandler.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

void printCurrentlySupported(ostream &out)
{
    MessageType types[] = {  
        MESSAGE_STATUS_TYPE,
        TEST_MESSAGE_TYPE,
        GPS_MESSAGE_TYPE, 
        LABEL_MESSAGE_TYPE, 
        EDGE_MESSAGE_TYPE,
        COLOR_MESSAGE_TYPE,
        DATA_REQUEST_MESSAGE_TYPE
    };

    out << "Currently supported types:" << endl;
    for(unsigned int i = 0; i < sizeof(types)/sizeof(types[0]); i++)
        out << "\t" << types[i] << endl;
    out << endl;
}

void usage(const char *progName)
{ 
    fprintf(stderr, "Usage: %s [args] [optional args]\n", basename(progName)); 
    fprintf(stderr, "\n");
    fprintf(stderr, "   -s, --server=address    The watcherd server to connect to.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional Args:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -m, --messages=m1,m2,m3 Which messages to subscribe to, do not set to subscribe to all messages\n"); 
    fprintf(stderr, "   -l, --layers=l1,l2,l3   Which layers to get.\n"); 
    fprintf(stderr, "   -t, --time=startTime    Starting point in time for requested messages, in unix epoch milliseconds. 0==current time\n"); 
    fprintf(stderr, "   -f, --timeFactor=factor Speed at which messages are sent. 1=real time, 2=twice as fast, .5=hlaf as fast. Negative numbers cause messages to be sent in reverse chronology. Default=1\n"); 
    fprintf(stderr, "   -p, --logProps              log.properties file, which controls logging for this program, if not specified, \"log.properties\" is looked for and used.\n"); 
    fprintf(stderr, "   -h,-H,-?,-help           Show this usage message\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "If no options are speficied, you get current time messages as received by the server.\n"); 
    fprintf(stderr, "\n");

    printCurrentlySupported(std::cerr); 

    fprintf(stderr, "\n");
    fprintf(stderr, "Note: Trying to specify anything other than all messages and all layers, currently does not work!\n"); 
    fprintf(stderr, "\n");

    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    string server;
    DataRequestMessage::MessageTypeList messages;
    const unsigned int layers=0;
    watcher::Timestamp startTime=0;
    int timeFactor=1;

    string logProps("sendMessage.log.properties");

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"messages", required_argument, 0, 'm'},
            {"layers", required_argument, 0, 'l'},
            {"time", required_argument, 0, 't'},
            {"timeFactor", required_argument, 0, 'f'},
            {"logProps", required_argument, 0, 'p'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "s:m:l:t:f:p:hH?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 's': server=optarg; break;
            case 'p': logProps=optarg; break;
            case 'm': printf("\nMessage argument is not currently supported\n"); break;
            case 'l': printf("\nLayer argument is not currently supported\n"); break;
            case 't': startTime=lexical_cast<watcher::Timestamp>(optarg); break;
            case 'f': timeFactor=lexical_cast<int>(optarg); break;
            case 'h':
            case 'H':
            case '?':
            default:
                usage(argv[0]); 
                break;
        }
    }

    if (server=="")
    {
        usage(argv[0]);
        exit(1); 
    }

    //
    // Now do some actual work.
    // 
    LOAD_LOG_PROPS(logProps);

    watcher::Client client(server); 
    client.addMessageHandler(SendMessageHandler::create());
    LOG_INFO("Connecting to " << server << " and sending message."); 
    
    DataRequestMessagePtr drm(new DataRequestMessage);

    // drm->dataTypesRequested=messages; 
    drm->dataTypesRequested.push_back(MESSAGE_STATUS_TYPE); 
    drm->dataTypesRequested.push_back(TEST_MESSAGE_TYPE); 
    drm->dataTypesRequested.push_back(GPS_MESSAGE_TYPE); 
    drm->dataTypesRequested.push_back(LABEL_MESSAGE_TYPE); 
    drm->dataTypesRequested.push_back(EDGE_MESSAGE_TYPE); 
    drm->requestAllMessages(); 
    drm->startingAt=startTime;
    drm->timeFactor=timeFactor;
    drm->layers=layers;

    if(!client.sendMessage(drm))
    {
        LOG_ERROR("Error sending label message: " << *drm);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    client.wait();

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

