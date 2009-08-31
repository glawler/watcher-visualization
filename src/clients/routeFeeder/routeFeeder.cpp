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
 * @file routeFeeder.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <stdio.h>
#include <sysexits.h> 	// portablish exit values. 
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <vector>

#include <libwatcher/client.h>                         // we are a client of the watcher.
#include <libwatcher/edgeMessage.h>         // we may send edgeMessages to the watcher. 
#include <libwatcher/connectivityMessage.h> // we may send connectivityMessages to the watcher. 
#include <logger.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;

static const char *rcsid __attribute__ ((unused)) = "$Id: routingfeeder.c,v 1.0 2009/04/28 22:08:47 glawler Exp $";

// #define DEBUG 1

/* This is a feeder which polls the routing table,
 * and draws the routing algorithm's edges on the watcher
 *
 *  Copyright (C) 2006,2007,2008,2009 Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

struct Route
{
    unsigned int dst;
    unsigned int nexthop;
    unsigned int mask;
    char iface[16];

    Route(int d, int n, int m, char i[16]) : dst(d), nexthop(n), mask(m) { memcpy(iface, i, sizeof(iface)); }
    Route(const Route &cpy) { dst=cpy.dst; nexthop=cpy.nexthop; mask=cpy.mask; memcpy(iface, cpy.iface, sizeof(iface)); }
    ~Route() { }
    ostream &operator<<(ostream &out) { return out << "dst: " << dst << " nexthop: " << nexthop << " mask: " << mask << " iface: " << (const char *)iface; }

};

bool operator==(const Route &a, const Route &b) { 
    return a.dst==b.dst && a.nexthop==b.nexthop && a.mask==b.mask && !strncmp(a.iface, b.iface, sizeof(a.iface)); 
}

bool operator<(const Route &a, const Route &b) { 
    return a.dst < b.dst;
}

typedef vector<Route> RouteList; 

/*
   Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask       MTU      Window  IRTT
   eth0    7402A8C0        7C02A8C0        0007    0       0       0       FFFFFFFF   00       0
   eth0    7402A8C0        7202A8C0        0007    0       0       0       FFFFFFFF   00       0
   */

static void routeRead(unsigned int network, unsigned int netmask, RouteList &oneHopRoutes, RouteList &nextHopRoutes)
{
    TRACE_ENTER(); 

    FILE *fil;
    char line[1024];

    fil=fopen("/proc/net/route","r");
    while(fgets(line,sizeof(line)-1,fil))
    {
        char iface[16];
        int flags,refcnt,use,metric,mtu,window;
        unsigned int dst,nexthop,mask;
        int rc;

        rc=sscanf(line,"%s\t%x\t%x\t%d\t%d\t%d\t%d\t%x\t%o\t%d\n",iface,&dst,&nexthop,&flags,&refcnt,&use,&metric,&mask,&mtu,&window);

        if (rc==10 && ((ntohl(dst) & ntohl(netmask)) == ntohl(network))) {
            if (nexthop==0)
                nextHopRoutes.push_back(Route(dst, nexthop, mask, iface));
            else 
                oneHopRoutes.push_back(Route(dst, nexthop, mask, iface));
        }
    }

    fclose(fil);

    TRACE_EXIT();
    return;
}

#if DEBUG
static void routeDump(const RouteList &r)
{
    TRACE_ENTER();

    if (r.size()) {
        for (RouteList::const_iterator i=r.begin(); i!=r.end(); i++) {
            ostringstream out;
            out << *i; 
        }
        LOG_DEBUG("%s", out.str().c_str());
    }
        
    TRACE_EXIT();
}
#endif // DEBUG

typedef struct detector
{
    Client *watcherClientPtr;

    useconds_t reportperiod;		/* frequency to generate reports at */
    int reportnum;
    string iface;
    int routeedgewidth;
    Color onehopcolor;
    Color nexthopcolor;

    string nexthoplayer;
    string onehoplayer;

    int duration;			  /* Time to run (in seconds) or 0 to run until broken */

	struct in_addr filterNetwork;
	struct in_addr filterMask;
	struct in_addr localhost;

    RouteList prevNextHopRoutes;   /* The previous next hop list we saw. */
    RouteList prevOneHopRoutes;    /* the previous one hop list we saw. */

    bool constantUpdates;

} detector;

typedef struct DetectorInit
{
    useconds_t reportperiod;		/* frequency to generate reports at */
    string iface;
    int onehopfamily;
    Color onehopcolor;
    Color nexthopcolor;
    int mouthnum;
    int duration;			  /* Time to run (in seconds) or 0 to run until broken */
    int routeedgewidth; 

    string serverName;

    string nexthoplayer;
    string onehoplayer;

    bool constantUpdates;

	struct in_addr filterNetwork;
	struct in_addr filterMask;
	struct in_addr localhost;

} DetectorInit;

static void updateRoutes(detector *st, RouteList &oneHopRoutes, RouteList &nextHopRoutes)
{
    TRACE_ENTER();

    static vector<MessagePtr> messages;
    if(!messages.empty())
        messages.clear(); 

    bool sendMessage=false;

    /*
     * If constant updates is true, always send route list. Else only send when the new list differs from the old.
     */
    if (st->constantUpdates || oneHopRoutes!=st->prevOneHopRoutes) { 
        ConnectivityMessagePtr message(new ConnectivityMessage);
        message->layer=ONE_HOP_ROUTING_LAYER;
        for (RouteList::const_iterator i=oneHopRoutes.begin(); i!=oneHopRoutes.end(); i++) 
            if (st->iface != string(i->iface))
                message->neighbors.push_back(boost::asio::ip::address_v4(i->dst));
        st->prevOneHopRoutes=oneHopRoutes;
        messages.push_back(message); 
        sendMessage=true;
    }
    if (st->constantUpdates || nextHopRoutes!=st->prevNextHopRoutes) { 
        ConnectivityMessagePtr message(new ConnectivityMessage);
        message->layer=ROUTING_LAYER;
        for (RouteList::const_iterator i=nextHopRoutes.begin(); i!=nextHopRoutes.end(); i++) 
            if (st->iface != string(i->iface))
                message->neighbors.push_back(boost::asio::ip::address_v4(i->nexthop));
        st->prevNextHopRoutes=nextHopRoutes;
        messages.push_back(message); 
        sendMessage=true;
    }

    if (sendMessage)
        st->watcherClientPtr->sendMessages(messages); 

    TRACE_EXIT();
}


/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void detectorSend(detector *st)
{
    TRACE_ENTER();

    RouteList oneHopRoutes, nextHopRoutes;

    routeRead(st->filterNetwork.s_addr, st->filterMask.s_addr, oneHopRoutes, nextHopRoutes);

    long long int curtime;
    struct timeval tp;
    gettimeofday(&tp, NULL); 
    curtime=(long long int)tp.tv_sec*1000 + (long long int)tp.tv_usec/1000;

    LOG_DEBUG("node= " << st->localhost.s_addr << " time= " << curtime << " routes= " << nextHopRoutes.size() << " onehoproutes= " << oneHopRoutes.size() << " totalroutes= " << nextHopRoutes.size()+oneHopRoutes.size());

    st->reportnum++;

    updateRoutes(st,oneHopRoutes, nextHopRoutes);

    TRACE_EXIT();
}

static detector *detectorInit(DetectorInit *detinit)
{
    TRACE_ENTER();
    detector *st;

    st=new detector;
    st->iface="";

    st->watcherClientPtr=new Client(detinit->serverName); 
    if(st->watcherClientPtr==NULL)
        return NULL;

    st->duration=detinit->duration;
    st->constantUpdates=detinit->constantUpdates;
    st->reportperiod=detinit->reportperiod;
    st->reportnum=0;
    st->iface=detinit->iface;

    st->onehoplayer=detinit->onehoplayer;
    st->onehopcolor=detinit->onehopcolor; 

    st->nexthoplayer=detinit->nexthoplayer;
    st->nexthopcolor=st->nexthopcolor;

    st->routeedgewidth=detinit->routeedgewidth;

	st->filterNetwork.s_addr=detinit->filterNetwork.s_addr;
	st->filterMask.s_addr=detinit->filterMask.s_addr;
	st->localhost.s_addr=detinit->localhost.s_addr;

    TRACE_EXIT();
    return st;
}

/* 
 * Wait around until we are ready to generate a message, then do it.
 */
static void selectLoop(detector *dt)
{
    TRACE_ENTER();

    time_t startTime=time(NULL); 

    while(1)
    {
        usleep(dt->reportperiod*(useconds_t)1000.0); 
        detectorSend(dt);

        if (dt->duration) 
            if (time(NULL)-startTime>dt->duration)
                break;
    }

    dt->watcherClientPtr->wait(); 

    TRACE_EXIT();
}

int main(int argc, char *argv[])
{
    TRACE_ENTER();
    detector *dt;
    int ch;
    string logPropsFile("routeFeeder.log.properties"); 

    DetectorInit detinit;
    detinit.iface="";
    detinit.onehopcolor=Color::green;
    detinit.onehoplayer="One Hop Routing";
    detinit.nexthopcolor=Color::blue;
    detinit.nexthoplayer="Network Routing"; 
    detinit.reportperiod=2000;
    detinit.duration=0;
    detinit.routeedgewidth=15; 

	detinit.filterNetwork.s_addr=0;
	detinit.filterMask.s_addr=0;
	detinit.localhost.s_addr=0;

    while ((ch = getopt(argc, argv, "m:n:h:o:x:t:e:b:i:d:p:w:l:c?")) != -1)
        switch (ch)
        {
            case 'm': if(-1 == inet_pton(AF_INET, optarg, &detinit.filterMask))
                      {
                          fprintf(stderr, "Error parsing filter mask: %s\n", optarg); 
                          exit(EX_USAGE); 
                      }
                      break;
            case 'n': if(-1 == inet_pton(AF_INET, optarg, &detinit.filterNetwork)) 
                      {
                          fprintf(stderr, "Error parsing filter network: %s\n", optarg); 
                          exit(EX_USAGE); 
                      }
                      break;
            case 'h': if(-1 == inet_pton(AF_INET, optarg, &detinit.localhost)) 
                      {
                          fprintf(stderr, "Error parsing localhost address: %s\n", optarg); 
                          exit(EX_USAGE); 
                      }
                      break;
            case 'o':
                      detinit.onehopcolor.fromString(optarg);
                      break;
            case 'x':
                      detinit.nexthopcolor.fromString(optarg); 
                      break;
            case 't':
                      sscanf(optarg,"%d",&detinit.duration);
                      break;
            case 'e':
                      detinit.onehoplayer=string(optarg); 
                      break;
            case 'b':
                      detinit.nexthoplayer=string(optarg); 
                      break;
            case 'i':
                      detinit.iface=string(optarg);
                      break;
            case 'd':
                      detinit.serverName=string(optarg);
                      break;
            case 'p':
                      sscanf(optarg,"%d",&detinit.reportperiod);
                      break;
            case 'w':
                      sscanf(optarg, "%d", &detinit.routeedgewidth); 
                      break;
            case 'l':
                      logPropsFile=string(optarg);
                      break;
            case 'u': 
                      detinit.constantUpdates=true;
                      break;
            case '?':
            default:
                      fprintf(stderr,"%s - poll the linux routing table, and draw the routes in the watcher\n"
                              "-d ipaddr/hostname - specify watcherd address to connect to (duck)\n"
                              "-h ipaddr - this host's address\n"
                              "-m netmask - the mask used to filter the routes (ex. 255.255.255.0)\n"
                              "-n network - the network used to filter the routes (ex. 192.168.1.0)\n" 
                              "-p milliseconds - specify the period to poll and report\n"
                              "-i interface - display only routes through this ethernet interface (or any if unspecified)\n"
                              "-e layer - onehop layer name\n"
                              "-b layer - nexthop layer name\n"
                              "-t seconds - Run for this long, then exit\n"
                              "-u - send updates every period. If not set, only send updates when the routes change.\n",
                              // "-c - use connectivity messages instead of edge messages (this disables width and color settings)\n"
                              basename(argv[0])
                             );
                      exit(1);
                      break;
        }

    // init the logging system
    LOAD_LOG_PROPS(logPropsFile); 
    LOG_INFO("Logger initialized from file \"" << logPropsFile << "\"");

	// check args, errors, etc before doing real work
	if(detinit.localhost.s_addr == 0) 
    {
        fprintf(stderr, "localhost address cannot be blank\n"); 
        exit(EX_USAGE);
    }
	if(detinit.filterNetwork.s_addr == 0) 
	{ 
        fprintf(stderr, "filter network cannot be blank\n"); 
        exit(EX_USAGE);
	}
	if(detinit.filterMask.s_addr == 0) 
	{ 
        fprintf(stderr, "filter mask cannot be blank\n"); 
        exit(EX_USAGE);
	}
	if(detinit.serverName.empty())
	{ 
        fprintf(stderr, "watcherd hostname cannot be blank\n"); 
        exit(EX_USAGE);
	}

    dt=detectorInit(&detinit);

    if (dt==NULL)
    {
        fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
        exit(1);
    }
    printf("%s: starting\n",argv[0]);
    selectLoop(dt);

    TRACE_EXIT(); 
    return 0;
}
