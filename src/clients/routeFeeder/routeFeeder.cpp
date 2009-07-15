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

typedef struct Route
{
    unsigned int dst;
    unsigned int nexthop;
    unsigned int mask;
    char iface[16];

    struct Route *next;
} Route;

/*
   Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask       MTU      Window  IRTT
   eth0    7402A8C0        7C02A8C0        0007    0       0       0       FFFFFFFF   00       0
   eth0    7402A8C0        7202A8C0        0007    0       0       0       FFFFFFFF   00       0
   */

static Route *routeRead(unsigned int network, unsigned int netmask)
{
    TRACE_ENTER(); 

    FILE *fil;
    char line[1024];
    Route *list=NULL,*nxt;

    fil=fopen("/proc/net/route","r");
    while(fgets(line,sizeof(line)-1,fil))
    {
        char iface[16];
        int flags,refcnt,use,metric,mtu,window;
        unsigned int dst,nexthop,mask;
        int rc;


        rc=sscanf(line,"%s\t%x\t%x\t%d\t%d\t%d\t%d\t%x\t%o\t%d\n",iface,&dst,&nexthop,&flags,&refcnt,&use,&metric,&mask,&mtu,&window);

        if (rc==10 && ((ntohl(dst) & ntohl(netmask)) == ntohl(network)))
        {
            nxt = (Route*)malloc(sizeof(*nxt));
            assert(nxt!=NULL);
            nxt->dst=htonl(dst);
            nxt->nexthop=htonl(nexthop);
            nxt->mask=ntohl(mask); 
            strncpy(nxt->iface,iface,sizeof(nxt->iface));

            nxt->next=list;
            list=nxt;
        }
    }

    fclose(fil);

    TRACE_EXIT();
    return list;
}

#if DEBUG
static void routeDump(Route *r)
{
    TRACE_ENTER();
    while(r)
    {
        LOG_DEBUG(r->dst << " -> " << r->nexthop << " mask " << r->mask); 
        r=r->next;
    }
    TRACE_EXIT();
}
#endif // DEBUG

static int routeNumber(Route *r)
{
    TRACE_ENTER();
    int count=0;

    while(r)
    {
        count++;
        r=r->next;
    }

    TRACE_EXIT_RET(count); 
    return count;
}

static void routeFree(Route *r)
{
    TRACE_ENTER();
    Route *d;

    while(r)
    {
        d=r;
        r=r->next;
        free(d);
    }
    TRACE_EXIT();
}

static void routeInsert(Route **list, Route *n)
{
    TRACE_ENTER();
    Route *nxt;
    nxt = (Route*)malloc(sizeof(*nxt));
    nxt->dst=n->dst;
    nxt->nexthop=n->nexthop;
    nxt->mask=n->mask;

    nxt->next=*list;
    *list=nxt;
    TRACE_EXIT();
}

/* return first route to in list which has the same nexthop
*/
static Route *routeSearchNext(Route *list, Route *key)
{
    TRACE_ENTER();
    Route *r;

    r=list;
    while(r)
    {
        if (r->nexthop==key->nexthop)
        {
            TRACE_EXIT_RET(r); 
            return r;
        }
        r=r->next;
    }
    TRACE_EXIT_RET("NULL"); 
    return NULL;
}

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

    int useConnectivityMessage;

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

	struct in_addr filterNetwork;
	struct in_addr filterMask;
	struct in_addr localhost;

    int useConnectivityMessage;

} DetectorInit;

static void updateRoutes(detector *st, Route *list)
{
    TRACE_ENTER();
    Route *r,*tmpNextHop=NULL;
    Route *tmpOneHop=NULL;

    /*
     * 	for each edge in newlist,
     * 	   	if there is not an edge in oldList or tmpList to nexthop, add it, and add to tmpList
     */
    static vector<MessagePtr> messages;
    ConnectivityMessagePtr nextHopMessage(new ConnectivityMessage);
    ConnectivityMessagePtr oneHopMessage(new ConnectivityMessage); 
    nextHopMessage->layer=ROUTING_LAYER;
    oneHopMessage->layer=ONE_HOP_ROUTING_LAYER;

    if(!messages.empty())
        messages.clear(); 

    for(r=list;r;r=r->next)
    {
        if (!st->iface.empty()) 
            if (st->iface != string(r->iface))    /* if we're checking interfaces, and this route is on the wrong one...  */
                continue;

#if 1
        LOG_DEBUG("nexthop inserting us -> " << r->nexthop); 
#endif

        if ((r->nexthop!=0) && (routeSearchNext(tmpNextHop,r)==NULL))   /* if its multi-hop, and not on the next hop list */
        {
            if (!st->useConnectivityMessage)
            {
                EdgeMessagePtr em=EdgeMessagePtr(new EdgeMessage); 
                em->node1=boost::asio::ip::address_v4(htonl(st->localhost.s_addr)); 
                em->node2=boost::asio::ip::address_v4(r->nexthop);
                em->edgeColor=st->nexthopcolor;
                em->expiration=Timestamp(st->reportperiod*1.5); 
                em->width=st->routeedgewidth;
                em->layer=ROUTING_LAYER; // GTL st->nexthoplayer;
                em->addEdge=true;
                em->bidirectional=false;
                messages.push_back(em);
            }
            else
                nextHopMessage->neighbors.push_back(boost::asio::ip::address_v4(r->nexthop));

            routeInsert(&tmpNextHop,r);
        }

        if (r->nexthop==0)   /* if its a one-hop... */
        {
#if 1
            LOG_DEBUG("onehop inserting us -> " << r->dst); 
#endif
            if (!st->useConnectivityMessage)
            {
                EdgeMessagePtr em=EdgeMessagePtr(new EdgeMessage); 
                em->node1=boost::asio::ip::address_v4(htonl(st->localhost.s_addr)); 
                em->node2=boost::asio::ip::address_v4(r->dst); 
                em->edgeColor=st->onehopcolor;
                em->expiration=Timestamp(st->reportperiod*1.5); 
                em->width=st->routeedgewidth;
                em->layer=ROUTING_LAYER; // GTL st->onehoplayer;
                em->addEdge=true;
                em->bidirectional=false;
                messages.push_back(em);
            }
            else
                oneHopMessage->neighbors.push_back(boost::asio::ip::address_v4(r->dst));

            routeInsert(&tmpOneHop,r);
        }
    }

    if (st->useConnectivityMessage)
    {
        if (nextHopMessage->neighbors.size())
            messages.push_back(nextHopMessage); 
        if (oneHopMessage->neighbors.size())
            messages.push_back(oneHopMessage); 
    }

    if (!messages.empty())
        st->watcherClientPtr->sendMessages(messages); 
    else
        LOG_DEBUG("No routes found so no messages sent!"); 

    routeFree(tmpNextHop);
    routeFree(tmpOneHop);

    TRACE_EXIT();
}


/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void detectorSend(detector *st)
{
    TRACE_ENTER();

    Route *list;

    list=routeRead(st->filterNetwork.s_addr, st->filterMask.s_addr);

    long long int curtime;
    struct timeval tp;
    gettimeofday(&tp, NULL); 
    curtime=(long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;

    LOG_DEBUG("node= " << st->localhost.s_addr << " time= " << curtime << " numroutes= " << routeNumber(list)); 

#ifdef DEBUG
    LOG_DEBUG("old:"); 
    routeDump(st->oldList);

    LOG_DEBUG("new:");
    routeDump(list);
#endif

    st->reportnum++;

    updateRoutes(st,list);

    routeFree(list);

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

    st->useConnectivityMessage=detinit->useConnectivityMessage;

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

    detinit.useConnectivityMessage=0; 

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
            case 'c':
                    detinit.useConnectivityMessage=1;
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
                              "-o color - onehop edge color\n"
                              "-e layer - onehop layer name\n"
                              "-x color - nexthop edge color\n"
                              "-b layer - nexthop layer name\n"
                              "-t seconds - Run for this long, then exit\n"
                              "-w width - make edges width pixels wide (default 15)\n" 
                              "-c - send connectivity messages - let the GUI decide how to display the routes\n",
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
