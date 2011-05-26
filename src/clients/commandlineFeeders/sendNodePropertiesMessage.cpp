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
 * @file sendNodePropertiesMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-09-04
 */

/**
 * @page sendsendNodePropertiesMessage 
 *
 * sendNodePropertiesMessage sends a node properties message. So there's that. 
 */
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "logger.h"
#include <libwatcher/client.h>
#include <libwatcher/colors.h>
#include <libwatcher/sendMessageHandler.h>
#include <libwatcher/nodePropertiesMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

DECLARE_GLOBAL_LOGGER("sendNodePropertiesMessage"); 

void usage(const char *progName)
{ 
    fprintf(stderr, "Usage: %s [args]\n", basename(progName)); 
    fprintf(stderr, "Args:\n");
    fprintf(stderr, "   -s, --server=address        The address of the node running watcherd.\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional args:\n");
    fprintf(stderr, "   -n, --node=address          The node to modify, if empty the local node's address is used.\n"); 
    fprintf(stderr, "   -l, --layer=layer           The layer to apply these properties to. string, e.g. \"hierachy\". \n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -a, --shape=shape           The shape to make the node. One of: circle, triangle, square, torus, teapot.\n"); 
    fprintf(stderr, "   -e, --effect=effect         The effect to apply to the node, one of: flash, spin, sparkle.\n");
    fprintf(stderr, "                                  This agrument can be used multiple times. Not all GUIs support all effects.\n");
    fprintf(stderr, "   -P, --property=prop         Tell the daemon that this node has a certain property, one of\n");
    fprintf(stderr, "                                  leafnode, neighborhoor, regional, root, attacker, victim. This argument can be used\n");
    fprintf(stderr, "                                  more than once.\n"); 
    // fprintf(stderr, "   -x, --expiration=seconds    How long in seconds to apply the properties. 0==forever, default forever.\n"); 
    fprintf(stderr, "   -c, --color=color           Set the node's color to color. Color can be string, e.g. \"blue\", or \n"); 
    fprintf(stderr, "                                   hex RRGGBBAA (red, green, blue, alpha) value, \"0xff00ff64\"\n"); 
    fprintf(stderr, "   -z, --size=float            How large (or small) to make the node relative to other nodes.\n"); 
    fprintf(stderr, "   -T, --timestamp=ms          Optionally specify a timestamp for this event\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "   -r, --logProps              log.properties file, which controls logging for this program\n");
    fprintf(stderr, "                                    Default value is watcher.log.props in the system config directory.\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "   -h, --help                  Show this message\n"); 

    exit(EXIT_FAILURE); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    Color color(colors::red);
    string server, logProps(SYSCONFDIR "/watcher.log.props");

    NodePropertiesMessagePtr message(new NodePropertiesMessage);

    bool colorSet=false;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"node", required_argument, 0, 'n'},
            {"layer", required_argument, 0, 'l'},
            {"shape", required_argument, 0, 'a'},
            {"effect", required_argument, 0, 'e'},
            {"property", required_argument, 0, 'P'},
            // {"expiration", required_argument, 0, 'x'},
            {"color", required_argument, 0, 'c'},
            {"size", required_argument, 0, 'z'},
            {"logProps", required_argument, 0, 'r'},
            {"timestamp", required_argument, 0, 'T'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "s:n:l:a:e:p:x:c:z:r:hHT:?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 's': server=optarg; 
                      break;
            case 'n': 
                      {
                          boost::system::error_code e;
                          message->fromNodeID=asio::ip::address::from_string(optarg, e);
                          if (e)
                          {
                              fprintf(stderr, "\nI did not understand the \"node\" argument: %s. It should be a host address.\n\n", optarg);
                              usage(argv[0]);
                          }
                      }
                      break;
            case 'l': message->layer=optarg; 
                      break;
            case 'a': {
                          bool success=NodePropertiesMessage::stringToNodeShape(optarg, message->shape);
                          if (!success) {
                              fprintf(stderr, "Unable to parse shape argument \"%s\".\n\n", optarg); 
                              usage(argv[0]);
                          }
                          message->useShape=true;
                      }
                      break;
            case 'e': {
                          NodePropertiesMessage::DisplayEffect e;
                          bool success=NodePropertiesMessage::stringToDisplayEffect(optarg, e);
                          if (!success) {
                              fprintf(stderr, "Unable to parse effect argument \"%s\".\n\n", optarg); 
                              usage(argv[0]);
                          }
                          message->displayEffects.push_back(e);
                      }
                      break;
            case 'p': {
                          NodePropertiesMessage::NodeProperty p;
                          bool success=NodePropertiesMessage::stringToNodeProperty(optarg, p);
                          if (!success) {
                              fprintf(stderr, "Unable to parse property argument \"%s\".\n\n", optarg); 
                              usage(argv[0]);
                          }
                          message->nodeProperties.push_back(p);
                      }
                      break;
            // case 'x': {
            //               try
            //               {
            //                   message->expiration=lexical_cast<Timestamp>(optarg); break;
            //               }
            //               catch (const boost::bad_lexical_cast &e)
            //               {
            //                   fprintf(stderr, "\nBad expiration argument: %s. It needs to be a number\n\n", optarg);
            //                   usage(argv[0]);
            //               }
            //           }
            //           break;
            case 'c': { 
                          bool val=message->color.fromString(optarg); 
                          if (!val) 
                          { 
                              printf("\nBad color argument: %s. Needs to be in ROYGBIV or \"0x12345678\" format.\n\n", optarg); 
                              usage(argv[0]); 
                          }
                          message->useColor=true;
                      }
                      break;
            case 'z': {
                          try
                          {
                              message->size=lexical_cast<float>(optarg); break;
                          }
                          catch (const boost::bad_lexical_cast &e)
                          {
                              fprintf(stderr, "\nBad size argument: %s. It needs to be a floating point number.\n\n", optarg);
                              usage(argv[0]);
                          }
                          if (message->size<0.0) {
                              fprintf(stderr, "\nBad size argument: %s. It needs to be a positive floating point number.\n\n", optarg);
                              usage(argv[0]);
                          }
                      }
                      break;
            case 'r': logProps=optarg; 
                      break;
            case 'T':
				message->timestamp=lexical_cast<Timestamp>(optarg); break;
				break;
            case 'h':
            case 'H':
            case '?':
            default:
                      usage(argv[0]); 
                      break;
        }
    }

    if (server=="") {
        fprintf(stderr, "\n\nserver is a required argument.\n\n");
        usage(argv[0]);
    }

    //
    // Now do some actual work.
    // 
    LOAD_LOG_PROPS(logProps);

    watcher::Client client(server); 
    LOG_INFO("Connecting to " << server << " and sending message."); 
    client.addMessageHandler(SingleMessageHandler::create());

    if(!client.sendMessage(message))
    {
        LOG_ERROR("Error sending node properties message: " << *message);
        TRACE_EXIT_RET(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    client.wait();

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

