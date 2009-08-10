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
 * @file sendEdgeMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
/**
 * @page sendEdgeMessage 
 *
 * sendEdgeMessage is a test node command line program that sends a watcher::event::GPSMessage message to a watcher daemon, specifing a node's current GPS coordinates.
 *
 * Usage: 
 * @{
 * <b>sendEdgeMessage -s server -t tail [optional args]</b>
 * @}
 * @{
 * Args:
 * @arg <b>-s, --server=address</b>, The address or name of the node running watcherd to which the message is sent.
 * @arg <b>-t, --tail=address</b>       The node to attach the tail of the edge to. If no head is given, the local node is used.
 * @}
 * Optional args:
 * @arg <b>-h, --head=address</b>       The node to attach the head of the edge to.
 * @arg <b>-c, --color=color</b>        The color of the edge. Can be ROYGBIV or RGBA format, string or hex value. Supports transparent colors.
 * @arg <b>-w, --width=width</b>        The width of the edge in some arbitrary, unknown unit.
 * @arg <b>-y, --layer=layer</b>        Which layer the edge is on in the GUI.
 * @arg <b>-d, --bidirectional=bool</b> Is this edge bidirectional or unidirectional. Use 'true' for true, anything else for false.
 * @arg <b>-l, --label=label</b>        The text to put in the middle label (This program only supports creating a middle label, although the message supports labels on node1 and node2 as well. May add that later)
 * @arg <b>-f, --labelfg=color</b>      The foreground color of the middle label. Can be ROYGBIV or RGBA format, string or hex value. Supports transparent colors.
 * @arg <b>-b, --labelbg=color</b>      The background color of the middle label. Can be ROYGBIV or RGBA format, string or hex value. Supports transparent colors.
 * @arg <b>-z, --fontSize=size</b>      The font size of the middle label
 * @arg <b>-x, --expiration=seconds</b> How long in milliseconds to diplay the edge
 * @arg <b>-p, --logProps</b>           log.properties file, which controls logging for this program
 *
 * @{
 * Examples:
 * @{
 *
 * Draw an edge between node 101 and node 102 on the "QoS" layer and make the color a translucent red.
 * @code 
 * sendEdgeMessage -s glory -h 192.168.1.101 -t 192.168.1.102 -l QoS -c 255.0.0.64
 * @endcode
 * @}
 * @}
 */
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/messageTypesAndVersions.h>    // for GUILayer
#include "sendMessageHandler.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

void usage(const char *progName)
{ 
    TRACE_ENTER();

    fprintf(stderr, "Usage: %s [args] [optional args]\n", basename(progName)); 
    fprintf(stderr, "Args:\n");
    fprintf(stderr, "   -s, --server=server      The name/address of the watcherd server\n");
    fprintf(stderr, "   -t, --tail=address       The node to attach the tail of the edge to. If no head is given, the local node is used.\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "   -H,-?,-help              Show this usage message\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional args:\n");
    fprintf(stderr, "   -h, --head=address       The node to attach the head of the edge to.\n"); 
    fprintf(stderr, "   -c, --color=color        The color of the edge. Can be ROYGBIV or RGBA format, string or hex value.\n"); 
    fprintf(stderr, "   -w, --width=width        The width of the edge in some arbitrary, unknown unit\n"); 
    fprintf(stderr, "   -y, --layer=layer        Which layer the edge is on in the GUI.\n"); 
    fprintf(stderr, "   -d, --bidirectional=bool Is this edge bidirectional or unidirectional. Use 'true' for true, anything else for false.\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "                            This program only supports creating a middle label, although the message supports\n");
    fprintf(stderr, "                            labels on node1 and node2 as well. May add that later\n"); 
    fprintf(stderr, "   -l, --label=label        The text to put in the middle label\n");
    fprintf(stderr, "   -f, --labelfg=color      The foreground color of the middle label. Can be ROYGBIV or RGBA format, string or hex value.\n"); 
    fprintf(stderr, "   -b, --labelbg=color      The background color of the middle label. Can be ROYGBIV or RGBA format, string or hex value.\n"); 
    fprintf(stderr, "   -z, --fontSize=size      The font size of the middle label\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -x, --expiration=milliseconds How long in milliseconds to diplay the edge\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -p, --logProps           log.properties file, which controls logging for this program\n");

    TRACE_EXIT_RET("Calling exit(1)"); 
    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    string server;

    asio::ip::address head(boost::asio::ip::address::from_string("127.0.0.1")); 
    asio::ip::address tail;

    bool tailSet=false;

    Color edgeColor=Color::red;
    unsigned int width=15;
    GUILayer layer=UNDEFINED_LAYER;
    bool bidirectional=false;

    LabelMessagePtr lm(new LabelMessage); 

    uint32_t expiration=10000;

    string logProps("sendMessage.log.properties");

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"head", required_argument, 0, 'h'},
            {"tail", required_argument, 0, 't'},

            {"help", no_argument, 0, 'H'},

            {"color", required_argument, 0, 'c'},
            {"width", required_argument, 0, 'w'},
            {"layer", required_argument, 0, 'y'},
            {"bidirectional", required_argument, 0, 'd'},

            {"label", required_argument, 0, 'l'},
            {"labelfg", required_argument, 0, 'f'},
            {"labelbg", required_argument, 0, 'b'},
            {"fontSize", required_argument, 0, 'z'},

            {"expiration", required_argument, 0, 'x'},

            {"logProps", required_argument, 0, 'p'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "s:h:t:c:w:y:d:l:f:b:z:x:p:H?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 's': server=optarg; break;
            case 'h': 
                      {
                          boost::system::error_code e;
                          head=asio::ip::address::from_string(optarg, e);
                          if (e)
                          {
                                fprintf(stderr, "\nI did not understand the \"head\" argument: %s. It should be a host address.\n\n", optarg);
                                usage(argv[0]);
                          }
                      }
                      break;
            case 't': 
                      {
                          boost::system::error_code e;
                          tail=asio::ip::address::from_string(optarg, e);
                          if (e)
                          {
                                fprintf(stderr, "\nI did not understand the \"tail\" argument: %s. It should be a host address.\n\n", optarg);
                                usage(argv[0]);
                          }
                          tailSet=true;
                      }
                      break;
            case 'c': { bool val=edgeColor.fromString(optarg); if (!val) { printf("\nBad argument for edge color\n\n"); usage(argv[0]); } break; }
            case 'w': width=lexical_cast<unsigned int>(optarg); break;
            case 'y': layer=GUILayer(optarg); break;
            case 'd': 
                      {
                          if (strstr(optarg, "true") || strstr(optarg, "TRUE")) bidirectional=true;
                          else bidirectional=false;
                      }
                      break;
            case 'l': lm->label=optarg; break;
            case 'f': { bool val=lm->foreground.fromString(optarg); if (!val) { printf("\nBad argument for label foreground color\n\n"); usage(argv[0]); } break; }
            case 'b': { bool val=lm->background.fromString(optarg); if (!val) { printf("\nBad argument for label background color\n\n"); usage(argv[0]); } break; }
            case 'z': lm->fontSize=lexical_cast<unsigned int>(optarg); break;

            case 'x': expiration=lexical_cast<uint32_t>(optarg); break;
            case 'p': logProps=optarg; break;
            case 'H':
            case '?':
            default:
                usage(argv[0]); 
                break;
        }
    }

    if (server=="" || !tailSet)
    {
        printf("\nRequired arguments are: server and tail\n\n");
        usage(argv[0]);
    }

    //
    // Now do some actual work.
    LOAD_LOG_PROPS(logProps);

    watcher::Client client(server); 
    client.addMessageHandler(SendMessageHandler::create());
    LOG_INFO("Connecting to " << server << " and sending message."); 

    EdgeMessagePtr em = EdgeMessagePtr(new EdgeMessage);
    em->node1=head;
    em->node2=tail;
    em->edgeColor=edgeColor;
    em->expiration=expiration;
    em->width=width;
    em->layer=layer;
    em->setMiddleLabel(lm); 
    em->bidirectional=bidirectional;

    if(!client.sendMessage(em))
    {
        LOG_ERROR("Error sending edge message: " << *em);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    client.wait();

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

