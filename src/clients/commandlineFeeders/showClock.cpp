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
 * @file showClock.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
/**
 * @page showClock 
 *
 * showClock is a test node command line program that "draws" a clock by arranging a set of nodes and edges into the shape of an 
 * analog clock.  The "clock" is updated once a second to move "the hands" of the clock around. This program is mostly
 * used to test the "TiVO" functionality built into the watcher system. 
 *
 * Usage: 
 * @{
 * <b>showClock -s server [optional args]</b>
 * @}
 * @{
 * Args:
 * @arg <b>-s, --server=address|name</b>, The address or name of the node running watcherd
 * @}
 * Optional args:
 * @arg <b>-r, --radius</b>, The radius of the clock face in some unknown unit
 * @arg <b>-S, --hideSecondRing</b>        Don't send message to draw the outer, second hand ring.
 * @arg <b>-H, --hideHourRing</b>          Don't send message to draw the inner, hour hand ring.
 * @arg <b>-p, --logProps=file</b>, log.properties file, which controls logging for this program
 * @arg <b>-e, --expireHands</b>           When drawing the hands, set them to expire after a short time.
 * @arg <b>-h, --help</b>, Show help message
 */
#include <getopt.h>
#include <string>
#include <math.h>

#include <boost/lexical_cast.hpp>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <libwatcher/client.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/watcherColors.h>
#include "logger.h"
#include "sendMessageHandler.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

#define PI (3.141592653589793) // Isn't this #DEFINEd somewhere?

void usage(const char *progName)
{ 
    fprintf(stderr, "Usage: %s [args]\n", basename(progName)); 
    fprintf(stderr, "Args:\n");
    fprintf(stderr, "   -s, --server=address        The addres of the node running watcherd\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "Optional args:\n");
    fprintf(stderr, "   -p, --logProps              log.properties file, which controls logging for this program\n");
    fprintf(stderr, "   -r, --radius                The radius of the circle in some unknown unit\n"); 
    fprintf(stderr, "   -S, --hideSecondRing        Don't send message to draw the outer, second hand ring\n");
    fprintf(stderr, "   -H, --hideHourRing          Don't send message to draw the inner, hour hand ring\n");
    fprintf(stderr, "   -e, --expireHands           When drawing edges for the hands, set an expiration on the edges.\n"); 
    fprintf(stderr, "\n");
    fprintf(stderr, "   -h, --help                  Show this message\n"); 

    exit(1); 
}

int main(int argc, char **argv)
{
    TRACE_ENTER();

    int c;
    string server;
    string logProps(string(basename(argv[0]))+string(".log.properties"));
    double radius=50.0; 
    bool showSecondRing=true, showHourRing=true, expireHands=false;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"logProps", required_argument, 0, 'p'},
            {"radius", no_argument, 0, 'r'},
            {"hideSecondRing", no_argument, 0, 'S'},
            {"hideHourRing", no_argument, 0, 'H'},
            {"expireHands", no_argument, 0, 'e'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "r:s:p:eSHh?", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 's': 
                server=optarg; 
                break;
            case 'p': 
                logProps=optarg; 
                break;
            case 'r':
                radius=lexical_cast<double>(optarg);
                break;
            case 'S':
                showSecondRing=false;
                break;
            case 'H':
                showHourRing=false;
                break;
            case 'e':
                expireHands=true;
                break;
            case 'h':
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

    Client client(server); 
    LOG_INFO("Connecting to " << server << " and sending message."); 
    // Do not add empty handler - default Client handler does not close connection.
    // client.addMessageHandler(SendMessageHandler::create());

    unsigned int loopTime=1; 
    // Create hour, min, sec, and center nodes.
    NodeIdentifier centerId=NodeIdentifier::from_string("192.168.1.100");
    NodeIdentifier hourId=NodeIdentifier::from_string("192.168.1.101");
    NodeIdentifier minId=NodeIdentifier::from_string("192.168.1.102");
    NodeIdentifier secId=NodeIdentifier::from_string("192.168.1.103");

    const GUILayer layer("Clock"); 
    const double step=(2*PI)/60;
    struct 
    {
        double theta;
        NodeIdentifier *id;
        const char *label;
        Color color;
        double length;
    } nodeData[]=
    {
        { 0,   &hourId, "hour", Color::red,   radius*0.7 }, 
        { 0,    &minId,  "min", Color::blue,  radius }, 
        { 0,    &secId,  "sec", Color::green, radius}, 
    };
    while (true)  // draw everything all the time as we don't know when watcher will start
    {
        // Draw center node
        GPSMessagePtr gpsMess(new GPSMessage(radius, radius, 0));
        gpsMess->layer=layer;
        gpsMess->fromNodeID=centerId;
        if(!client.sendMessage(gpsMess))
        {
            LOG_ERROR("Error sending gps message: " << *gpsMess);
            TRACE_EXIT_RET(EXIT_FAILURE);
            return EXIT_FAILURE;
        }
        // draw hour, min, and second nodes, connecting them to center.
        for (unsigned int i=0; i<sizeof(nodeData)/sizeof(nodeData[0]); i++)
        {
            // update location offsets by current time.
            time_t nowInSecs=time(NULL);
            struct tm *now=localtime(&nowInSecs);
            if (*nodeData[i].id==hourId)
                nodeData[i].theta=step*((now->tm_hour*(60/12))+(now->tm_min/12));
            else if(*nodeData[i].id==minId)
                nodeData[i].theta=step*now->tm_min;
            else if(*nodeData[i].id==secId)
                nodeData[i].theta=step*now->tm_sec;

            // Move hour. min, and sec nodes to appropriate locations. 
            GPSMessagePtr gpsMess(new GPSMessage(
                        (sin(nodeData[i].theta)*nodeData[i].length)+radius, 
                        (cos(nodeData[i].theta)*nodeData[i].length)+radius, 
                        (double)i));
            gpsMess->layer=layer;
            gpsMess->fromNodeID=*nodeData[i].id;
            if(!client.sendMessage(gpsMess))
            {
                LOG_ERROR("Error sending gps message: " << *gpsMess);
                TRACE_EXIT_RET(EXIT_FAILURE);
                return EXIT_FAILURE;
            }

            LabelMessagePtr labMess(new LabelMessage(nodeData[i].label));
            labMess->layer=layer;
            labMess->expiration=expireHands?loopTime*2000:0; 
            EdgeMessagePtr edgeMess(new EdgeMessage(centerId, *nodeData[i].id, layer, nodeData[i].color, 2));
            edgeMess->middleLabel=labMess;
            edgeMess->expiration=expireHands?loopTime*2000:0;
            if(!client.sendMessage(edgeMess))
            {
                LOG_ERROR("Error sending edge message: " << *edgeMess);
                TRACE_EXIT_RET(EXIT_FAILURE);
                return EXIT_FAILURE;
            }
        }

        if (showHourRing)
        {
            // add nodes at clock face number locations.
            double theta=(2*PI)/12;
            for (unsigned int i=0; i<12; i++, theta+=(2*PI)/12)
            {
                NodeIdentifier thisId=NodeIdentifier::from_string("192.168.2." + lexical_cast<string>(i+1));
                GPSMessagePtr gpsMess(new GPSMessage((sin(theta)*radius)+radius, (cos(theta)*radius)+radius, 0.0)); 
                gpsMess->layer=layer;
                gpsMess->fromNodeID=thisId;
                if(!client.sendMessage(gpsMess))
                {
                    LOG_ERROR("Error sending gps message: " << *gpsMess);
                    TRACE_EXIT_RET(EXIT_FAILURE);
                    return EXIT_FAILURE;
                }
            }
        }

        if (showSecondRing)
        {
            // add nodes at clock face second locations.
            double theta=(2*PI)/60;
            for (unsigned int i=0; i<60; i++, theta+=(2*PI)/60)
            {
                NodeIdentifier thisId=NodeIdentifier::from_string("192.168.3." + lexical_cast<string>(i+1));
                double faceRad=radius*1.15;
                GPSMessagePtr gpsMess(new GPSMessage((sin(theta)*faceRad)+radius, (cos(theta)*faceRad)+radius, 0.0)); 
                gpsMess->layer=layer;
                gpsMess->fromNodeID=thisId;
                if(!client.sendMessage(gpsMess))
                {
                    LOG_ERROR("Error sending gps message: " << *gpsMess);
                    TRACE_EXIT_RET(EXIT_FAILURE);
                    return EXIT_FAILURE;
                }
            }
        }
        sleep(loopTime); 
    }

    client.wait();

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

