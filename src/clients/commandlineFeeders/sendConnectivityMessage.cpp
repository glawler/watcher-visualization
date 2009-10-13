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
 * @file sendConnectivityMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15
 *
 * @page sendConnectivityMessage 
 *
 * sendConnectivityMessage is a test node command line program that sends a watcher::event::ConnectivityMessage message to the watcher daemon, specifying the current list of neighbors that the node has. 
 * The GUI(s) that are listening to that daemon, then draw the neighbors somehow. 
 *
 * Usage: 
 * @{
 * <b>showColor -s server [optional args] nbr1 nbr2 nbr3 ... nbrN</b>
 * @}
 * @{
 * Args:
 * @arg <b>-s, --server=address</b>, The addres of the node running watcherd
 * @arg <b>nbr1 nbr2 nbr3 ... nbrN</b> - the list if neighbors by ipaddress
 * @}
 * Optional args:
 * @arg <b>-l, --layer=layer</b>, the layer that these neighbors should show up on when displayed in the GUI(s)
 * @arg <b>-p, --logProps=log.propertiesFile</b>, the log properties file to use
 * @arg <b>-f, --fromNode=fromNodeAddr</b>, the node that has these neighbors, if not given the local node is assumed.
 *
 * @{
 *
 * Examples:
 * @{
 *
 * This tells the GUI(s) that are listening to the daemon running on 'glory' the local test node has neighbors 192.168.1.101 and 192.168.1.102
 * @code 
 * sendConnectivityMessage -s glory 192.168.1.101 192.168.1.102
 * @endcode
 *
 * This tells the GUI(s) that are listening to the daemon running on 'glory' the local test node 192.168.1.101 has neighbor nodes 192.168.1.110-192.168.1.115 and 
 * they should be displayed on the "children" layer. (Note that 192.168.1.11{0..5} is a bashism which expands to the sequenctial list of nodes 192.168.1.110-192.168.1.115.) 
 * @code 
 * sendConnectivityMessage -s glory -l children -f 192.168.1.101 192.168.1.11{0..5}
 * @endcode
 *
 * @}
 */

#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <boost/foreach.hpp>
#include <boost/asio/ip/address.hpp>

#include <libwatcher/connectivityMessage.h>
#include <libwatcher/client.h>
#include "logger.h"
#include <libwatcher/sendMessageHandler.h>

DECLARE_GLOBAL_LOGGER("sendConnectivityMessage"); 

using namespace std;
using namespace watcher;
using namespace watcher::event;

void usage(const char *progName)
{
    fprintf(stderr, "Usage: %s -s|--servername server [-l|--layer=layer -p|--logProps=log.propertiesFile] [-f|--fromNode=fromNodeAddr] nbr1 nbr2 ... nbrN\n", basename(progName)); 
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
    GUILayer layer(PHYSICAL_LAYER); 
    boost::asio::ip::address_v4 fromNodeAddr;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"logProps", required_argument, 0, 'p'},
            {"servername", required_argument, 0, 's'},
            {"fromNode", required_argument, 0, 'f'},
            {"layer", required_argument, 0, 'l'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "p:s:f:l:hH?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 'p': 
                logProps = strdup(optarg); // buffer overflow here. :) 
                break;
            case 's':
                serverName = strdup(optarg);  // buffer overflow here :) 
                break;
            case 'f':
                fromNodeAddr=boost::asio::ip::address_v4::from_string(optarg);
                break;
            case 'l':
                layer = strdup(optarg); 
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
    cm->layer=layer;

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

