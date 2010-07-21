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
 * @arg <b>-h, --help</b>, Show help message
 */
#include <getopt.h>
#include <string>
#include <math.h>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <libwatcher/client.h>
#include <libwatcher/gpsMessage.h>
#include <libwatcher/labelMessage.h>
#include <libwatcher/edgeMessage.h>
#include <libwatcher/watcherColors.h>
#include <libwatcher/colors.h>
#include "logger.h"
#include <libwatcher/sendMessageHandler.h>

DECLARE_GLOBAL_LOGGER("showClock"); 

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
    fprintf(stderr, "   -t, --latitude              Place the clock at this latitude (def==0).\n"); 
    fprintf(stderr, "   -g, --longitude             Place the clock at this longitude (def==0).\n"); 
    fprintf(stderr, "   -x, --gpsScale              Factor the GPS positions by this much (def==1)\n"); 
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
    bool showSecondRing=true, showHourRing=true;
    double offsetLong=0, offsetLat=0;
    double gpsScale=1;

    while (true) 
    {
        int option_index = 0;
        static struct option long_options[] = {
            {"server", required_argument, 0, 's'},
            {"logProps", required_argument, 0, 'p'},
            {"radius", no_argument, 0, 'r'},
            {"hideSecondRing", no_argument, 0, 'S'},
            {"hideHourRing", no_argument, 0, 'H'},
            {"latitude", required_argument, 0, 't'}, 
            {"longitude", required_argument, 0, 'g'}, 
            {"gpsScale", required_argument, 0, 'x'}, 
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "r:s:p:t:g:x:eSHh?", long_options, &option_index);

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
            case 'g':
                offsetLong=lexical_cast<double>(optarg); 
                break;
            case 't':
                offsetLat=lexical_cast<double>(optarg); 
                break;
            case 'x':
                gpsScale=lexical_cast<double>(optarg);
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

    const GUILayer hourLayer("Hour"); 
    const GUILayer minLayer("Min"); 
    const GUILayer secLayer("Sec"); 
    const GUILayer hourRingLayer("HourRing"); 
    const GUILayer secRingLayer("SecondRing"); 

    const double step=(2*PI)/60;
    struct 
    {
        double theta;
        NodeIdentifier *id;
        const char *label;
        double length;
        const GUILayer layer;
    } nodeData[]=
    {
        { 0,   &hourId, "hour", radius*0.7, hourLayer }, 
        { 0,    &minId,  "min", radius, minLayer }, 
        { 0,    &secId,  "sec", radius, secLayer }, 
    };

    while (true)  // draw everything all the time as we don't know when watcher will start
    {
        vector<MessagePtr> messages; 

        // Draw center node
        GPSMessagePtr gpsMess(new GPSMessage(gpsScale*(offsetLong+radius), gpsScale*(offsetLat+radius), 0));
        gpsMess->layer=hourLayer; // meh.
        gpsMess->fromNodeID=centerId;

        messages.push_back(gpsMess); 
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
                        gpsScale*(offsetLong+(sin(nodeData[i].theta)*nodeData[i].length)+radius), 
                        gpsScale*(offsetLat+(cos(nodeData[i].theta)*nodeData[i].length)+radius), 
                        (double)i));
            gpsMess->layer=nodeData[i].layer;
            gpsMess->fromNodeID=*nodeData[i].id;

            messages.push_back(gpsMess); 

            EdgeMessagePtr edge(new EdgeMessage(centerId, *nodeData[i].id, nodeData[i].layer, 
                        colors::blue, 2.0, false, loopTime*1500, true)); 

            LabelMessagePtr labMess(new LabelMessage(nodeData[i].label));
            labMess->layer=nodeData[i].layer;
            labMess->expiration=loopTime*1500; 
            edge->middleLabel=labMess;

            LabelMessagePtr numLabMess(new LabelMessage);
            if (*nodeData[i].id==hourId)
                numLabMess->label=boost::lexical_cast<string>(now->tm_hour%12); 
            else if(*nodeData[i].id==minId)
                numLabMess->label=boost::lexical_cast<string>(now->tm_min); 
            else if(*nodeData[i].id==secId)
                numLabMess->label=boost::lexical_cast<string>(now->tm_sec); 
            numLabMess->layer=nodeData[i].layer;
            numLabMess->expiration=loopTime*1500; 
            edge->node2Label=numLabMess;

            messages.push_back(edge);
        }

        if (showHourRing)
        {
            // add nodes at clock face number locations.
            double theta=(2*PI)/12;
            for (unsigned int i=0; i<12; i++, theta+=(2*PI)/12)
            {
                NodeIdentifier thisId=NodeIdentifier::from_string("192.168.2." + lexical_cast<string>(i+1));
                GPSMessagePtr gpsMess(new GPSMessage(
                            gpsScale*(offsetLong+((sin(theta)*radius)+radius)), 
                            gpsScale*(offsetLat+((cos(theta)*radius)+radius)), 
                            0.0));
                gpsMess->layer=hourRingLayer;
                gpsMess->fromNodeID=thisId;

                messages.push_back(gpsMess); 
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
                GPSMessagePtr gpsMess(new GPSMessage(
                            gpsScale*(offsetLong+((sin(theta)*faceRad)+radius)), 
                            gpsScale*(offsetLat+((cos(theta)*faceRad)+radius)), 0.0)); 
                gpsMess->layer=secRingLayer;
                gpsMess->fromNodeID=thisId;

                messages.push_back(gpsMess); 
            }
        }
        if (!messages.empty()) { 
            if(!client.sendMessages(messages)) {
                LOG_ERROR("Error sending " << messages.size() << " messages.");
                TRACE_EXIT_RET(EXIT_FAILURE);
                return EXIT_FAILURE;
            }
        }

        sleep(loopTime); 
    }

    client.wait();

    TRACE_EXIT_RET(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

