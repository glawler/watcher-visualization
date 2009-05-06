#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include <string>

#include <client.h>                     // we are a client of the watcher.
#include <libwatcher/edgeMessage.h>     // we send edgeMessages to the watcher. 
#include <logger.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;

static const char *rcsid __attribute__ ((unused)) = "$Id: routingfeeder.c,v 1.0 2009/04/28 22:08:47 glawler Exp $";

#define DEBUG 1

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

static Route *routeRead()
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

        if (rc==10)
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
    unsigned int localaddr;
    string iface;
    int routeedgewidth;
    Color onehopcolor;
    Color nexthopcolor;

    string nexthoplayer;
    string onehoplayer;

    int duration;			  /* Time to run (in seconds) or 0 to run until broken */
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
    for(r=list;r;r=r->next)
    {

        if (st->iface != string(r->iface))    /* if we're checking interfaces, and this route is on the wrong one...  */
            continue;

#if 1
        LOG_DEBUG("nexthop inserting us -> " << r->nexthop); 
#endif

        if ((r->nexthop!=0) && (routeSearchNext(tmpNextHop,r)==NULL))   /* if its multi-hop, and not on the next hop list */
        {
            EdgeMessagePtr em=EdgeMessagePtr(new EdgeMessage); 
            em->node1=boost::asio::ip::address_v4(st->localaddr); 
            em->node2=boost::asio::ip::address_v4(r->nexthop);
            em->edgeColor=st->nexthopcolor;
            em->expiration=st->reportperiod*1.5;
            em->width=st->routeedgewidth;
            em->layer=ROUTING_LAYER; // GTL st->nexthoplayer;
            em->addEdge=true;
            em->bidirectional=false;
            st->watcherClientPtr->sendMessage(em); 

            routeInsert(&tmpNextHop,r);
        }

        if (r->nexthop==0)   /* if its a one-hop... */
        {
#if 1
            LOG_DEBUG("onehop inserting us -> " << r->dst); 
#endif
            EdgeMessagePtr em=EdgeMessagePtr(new EdgeMessage); 
            em->node1=boost::asio::ip::address_v4(st->localaddr); 
            em->node2=boost::asio::ip::address_v4(r->dst); 
            em->edgeColor=st->onehopcolor;
            em->expiration=st->reportperiod*1.5;
            em->width=st->routeedgewidth;
            em->layer=ROUTING_LAYER; // GTL st->onehoplayer;
            em->addEdge=true;
            em->bidirectional=false;
            st->watcherClientPtr->sendMessage(em); 

            routeInsert(&tmpOneHop,r);
        }
    }

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

    list=routeRead();  // read all routes.

    long long int curtime;
    struct timeval tp;
    gettimeofday(&tp, NULL); 
    curtime=(long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;

    LOG_DEBUG("node= " << st->localaddr << " time= " << curtime << " numroutes= " << routeNumber(list)); 

#ifdef DEBUG
#if 0
    LOG_DEBUG("old:"); 
    routeDump(st->oldList);
#endif
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

    st->localaddr=0x7f000001;  // 127.0.0.1

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

    while ((ch = getopt(argc, argv, "w:t:d:o:n:a:b:i:p:l:?")) != -1)
        switch (ch)
        {
            case 'o':
                detinit.onehopcolor.fromString(optarg);
                break;
            case 'n':
                detinit.nexthopcolor.fromString(optarg); 
                break;
            case 't':
                sscanf(optarg,"%d",&detinit.duration);
                break;
            case 'a':
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
            case '?':
            default:
                fprintf(stderr,"routingdetector - poll the linux routing table, and draw the routes in the watcher\n"
                        "-d ipaddr/hostname - specify watcherd to connect to (duck)\n"
                        "-p milliseconds - specify the period to poll and report\n"
                        "-i interface - display only routes through this ethernet interface (or any if unspecified)\n"
                        "-o color - onehop edge color\n"
                        "-a layer - onehop layer name\n"
                        "-n color - nexthop edge color\n"
                        "-b layer - nexthop layer name\n"
                        "-t seconds - Run for this long, then exit\n"
                        "-w width - make edges width pixels wide (default 15)\n"
                       );
                exit(1);
                break;
        }

    // init the logging system
    LOAD_LOG_PROPS(logPropsFile); 
    LOG_INFO("Logger initialized from file \"" << logPropsFile << "\"");

    if(detinit.iface=="")
    {
        LOG_FATAL("You must specify an interface"); 
        exit(EXIT_FAILURE); 
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
