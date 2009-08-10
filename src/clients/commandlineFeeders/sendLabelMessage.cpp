/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file sendLabelMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
/**
 * @page sendLabelMessage 
 *
 * sendLabelMessage is a test node command line program that sends a watcher::event::LabelMessage message to the watcher daemon, specifing that a label should be
 * attached to the specified node (or float if given coords).
 *
 * If address is specified, the label will attach to the node with that address. If cooridinates are
 * specified, the label will float at those coordinates. The node address takes precedence. If neither
 * option is specified, the label will attach to the node from which the message saw sent.
 *
 * Usage: 
 * @{
 * <b>sendLabelMessage -s server -l label [optional args]</b>
 * @}
 * @{
 * Args:
 * @arg <b>-l, --label=text</b>, The text of the label
 * @arg <b>-s, --server=address</b>, The address|name of the node running watcherd, the server.
 * @}
 * Optional args:
 * @arg <b>-n, --node=address</b>, The node to change color, if empty the local node's address is used
 * @arg <b>-x, --latitude=coord</b>        The latitude to float the node at.
 * @arg <b>-y, --longitude=coord</b>       The longitude to float the node at.
 * @arg <b>-z, --altitiude=coord</b>       The altitude to float the node at.
 * @arg <b>-t, --fontSize=size</b>         The font size of the label
 * @arg <b>-f, --foreground=color</b>      The foreground color of the label. Can be ROYGBIV or RGBA format, string or hex value.
 * @arg <b>-b, --background=color</b>      The background color of the label. Can be ROYGBIV or RGBA format, string or hex value.
 * @arg <b>-e, --expiration=seconds</b>    How long in millisecond to diplay the label
 * @arg <b>-r, --remove</b>                Remove the label if it is attached
 * @arg <b>-L, --layer=layer</b>           Which layer the label is part of. Default is "Physcial".
 * @arg <b>-x, --expiration=seconds</b>, How long in seconds to change the color. 0==forever
 * @arg <b>-p, --logProps</b>, log.properties file, which controls logging for this program
 * @arg <b>-h, --help</b>, Show help message
 *
 * Examples:
 * @{
 * @code 
 * sendLabelMessage -s glory -n 192.168.1.102 -l "Correlation Layer" -e 1500 -f red -b green -L Correlation
 * sendLabelMessage -s glory -n 192.168.1.102 -l "Physical Layer" -e 1500 -L Physical
 * sendLabelMessage -s glory -n 192.168.1.104 -l "Create a node with a label" -e 1500 -f yellow -b blue -L Physical 
 * @endcode
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
    unsigned int fontSize=0;
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
            case 'b': { bool val=bg.fromString(optarg); if (!val) { printf("\nBad argument for bg color\n\n"); usage(argv[0]); } break; }
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
            case 'L': layer=optarg; break;
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

