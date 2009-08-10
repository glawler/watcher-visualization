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
 * @file sendColorMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15
 */

/**
 * @page sendColorMessage 
 *
 * sendColorMessage is a test node command line program that sends a watcher::event::ColorMessage message to the watcher daemon, specifing that a node should change it's color. 
 *
 * Usage: 
 * @{
 * <b>sendColorMessage -s server -c color [optional args]</b>
 * @}
 * @{
 * Args:
 * @arg <b>-c, --color=color</b>, The color of the node. Can be ROYGBIV or RGBA format, string or hex value. Supports transparency. 
 * @arg <b>-s, --server=address</b>, The addres of the node running watcherd
 * @}
 * Optional args:
 * @arg <b>-n, --node=address</b>, The node to change color, if empty the local node's address is used
 * @arg <b>-f, --flash=milliseconds</b>, Flash between the new color and the orginal color every milliseconds seconds, 0 for no flash.
 * @arg <b>-x, --expiration=seconds</b>, How long in seconds to change the color. 0==forever
 * @arg <b>-p, --logProps</b>, log.properties file, which controls logging for this program
 * @arg <b>-h, --help</b>, Show help message
 *
 * Examples:
 * @{
 *
 * This tells the GUI(s) that are listening to the daemon running no 'glory' the node at 192.168.1.101 should be drawn in blue. 
 * @code 
 * showColor -s glory -c blue -n 192.168.1.101
 * @endcode
 *
 * This tells the GUI(s) that are listening to the daemon running no 'glory' the node at 192.168.1.101 should be drawn in a transparent blue. 
 * Transparent format is R.G.B.A, where "A" is alpha transparency.
 * @code 
 * showColor -s glory -c 0.0.255.64 -n 192.168.1.101
 * @endcode
 *
 * This tells the GUI(s) that are listening to the daemon running no 'glory' the node at 192.168.1.107 should be drawn in green for 5 seconds.
 * @code 
 * showColor -s glory -c green -n 192.168.1.107 --expiration 5000
 * @endcode
 *
 * This tells the GUI(s) that are listening to the daemon running no 'glory' the node at 192.168.1.104 should flash for 10 seconds.
 * @code 
 * showColor --server glory --color green --node 192.168.1.104 --flash --expiration 10000
 * @endcode
 *
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
    Timestamp flashTime=0;
    Timestamp expiration=0; 
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
                              flashTime=lexical_cast<Timestamp>(optarg); break;
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
                              expiration=lexical_cast<Timestamp>(optarg); break;
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

