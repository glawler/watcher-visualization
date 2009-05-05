#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include <client.h>                     // we are a client of the watcher.
#include <libwatcher/edgeMessage.h>     // we send edgeMessages to the watcher. 

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
	ManetAddr dst;
	ManetAddr nexthop;
	ManetAddr mask;
	char iface[16];

	struct Route *next;
} Route;

/*
Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask       MTU      Window  IRTT
eth0    7402A8C0        7C02A8C0        0007    0       0       0       FFFFFFFF   00       0
eth0    7402A8C0        7202A8C0        0007    0       0       0       FFFFFFFF   00       0
*/

static Route *routeRead(NodeIdentifier manetnetwork,ManetAddr manetmask)
{
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

		if ((rc==10) && ((ntohl(dst) & manetmask) == manetnetwork))
		{
//			fprintf(stderr,"%d.%d.%d.%d (%d.%d.%d.%d) -> %d.%d.%d.%d\n",PRINTADDR(dst),PRINTADDR(mask),PRINTADDR(nexthop));
			nxt = (Route*)malloc(sizeof(*nxt));
			assert(nxt!=NULL);
			nxt->dst=ntohl(dst);
			nxt->nexthop=ntohl(nexthop);
			nxt->mask=ntohl(mask);
			strncpy(nxt->iface,iface,sizeof(nxt->iface));

			nxt->next=list;
			list=nxt;
		}
	}

	fclose(fil);
	return list;
}

static void routeDump(FILE *fil,Route *r)
{
	while(r)
	{
		fprintf(fil,"%d.%d.%d.%d -> %d.%d.%d.%d  mask %d.%d.%d.%d\n",PRINTADDR(r->dst),PRINTADDR(r->nexthop),PRINTADDR(r->mask));
		r=r->next;
	}
}

static int routeNumber(Route *r)
{
	int count=0;

	while(r)
	{
		count++;
		r=r->next;
	}
	return count;
}

static void routeFree(Route *r)
{
	Route *d;

	while(r)
	{
		d=r;
		r=r->next;
		free(d);
	}
}

static void routeInsert(Route **list, Route *n)
{
	Route *nxt;
	nxt = (Route*)malloc(sizeof(*nxt));
	nxt->dst=n->dst;
	nxt->nexthop=n->nexthop;
	nxt->mask=n->mask;

	nxt->next=*list;
	*list=nxt;
}

/* return first route to in list which has the same nexthop
 */
static Route *routeSearchNext(Route *list, Route *key)
{
	Route *r;

	r=list;
	while(r)
	{
		if (r->nexthop==key->nexthop)
			return r;
		r=r->next;
	}
	return NULL;
}

typedef struct detector
{
    Client *watcherClientPtr;

	int reportperiod;		/* frequency to generate reports at */
	int reportnum;
	int tag;
	NodeIdentifier manetaddr;
	NodeIdentifier manetnetwork;
    NodeIdentifier::netmask manetnetmask;
	char *iface;
	int onehopfamily;
	int nexthopfamily;
	int routeedgewidth;
	Color onehopcolor;
	Color nexthopcolor;

	int duration;			  /* Time to run (in seconds) or 0 to run until broken */

	ManetAddr othermouth;

} detector;

typedef struct DetectorInit
{
	int reportperiod;		/* frequency to generate reports at */
	char *iface;
	int onehopfamily;
	int nexthopfamily;
	Color onehopcolor;
	Color nexthopcolor;
	int mouthnum;
	int duration;			  /* Time to run (in seconds) or 0 to run until broken */
	int routeedgewidth; 

    string serverName;
    string serverPort;

} DetectorInit;

static void updateRoutes(detector *st, Route *list)
{
	Route *r,*tmpNextHop=NULL;
	Route *tmpOneHop=NULL;
	int p;

/*
	for each edge in newlist,
		if there is not an edge in oldList or tmpList to nexthop, add it, and add to tmpList
*/
	for(r=list;r;r=r->next)
	{

		if ((st->iface) && (strcmp(st->iface,r->iface)!=0))    /* if we're checking interfaces, and this route is on the wrong one...  */
			continue;
	
#if 1
		fprintf(stderr,"nexthop inserting us -> %d\n",r->nexthop & 0xFF);
#endif

		if ((r->nexthop!=0) && (routeSearchNext(tmpNextHop,r)==NULL))   /* if its multi-hop, and not on the next hop list */
		{
		    NodeEdge edge;
			edge.head=NODE_LOCAL;
			edge.tail=r->nexthop;
			edge.family=st->nexthopfamily;
			edge.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
			edge.tag=st->tag;
			edge.width=st->routeedgewidth;
			if (edge.tail==st->othermouth)
				edge.width=70;
			edge.expiration=st->reportperiod*1.5;
			memcpy(edge.color,st->nexthopcolor,sizeof(st->nexthopcolor));
			edge.labelHead.text=NULL;
			edge.labelMiddle.text=NULL;
			edge.labelTail.text=NULL;
			communicationsWatcherEdge(st->cs,&edge);

			routeInsert(&tmpNextHop,r);
		}

		if (r->nexthop==0)   /* if its a one-hop...  */
		{
			NodeEdge edge;
#if 1
			fprintf(stderr,"onehop inserting us -> %d\n",r->dst & 0xFF);
#endif
			edge.head=NODE_LOCAL;
			edge.tail=r->dst;
			edge.family=st->onehopfamily;
			edge.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
			edge.tag=st->tag+1;
			edge.width=st->routeedgewidth;
			if (edge.tail==st->othermouth)
				edge.width=40;
			edge.expiration=st->reportperiod*2;
			memcpy(edge.color,st->onehopcolor,sizeof(st->onehopcolor));
			edge.labelHead.text=NULL;
			edge.labelMiddle.text=NULL;
			edge.labelTail.text=NULL;
			communicationsWatcherEdge(st->cs,&edge);

			routeInsert(&tmpOneHop,r);
		}
	}

	routeFree(tmpNextHop);
	routeFree(tmpOneHop);
}


/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void detectorSend(detector *st)
{
	Route *list;
	destime curtime;

	list=routeRead(st->manetnetwork,st->manetnetmask);

	curtime=getMilliTime();
	printf("node= %d time= %lld numroutes= %d\n",st->manetaddr & 0xFF, curtime,routeNumber(list));

#ifdef DEBUG
#if 0
	fprintf(stderr,"old:\n");
	routeDump(stderr,st->oldList);
#endif
	fprintf(stderr,"new:\n");
	routeDump(stderr,list);
	fprintf(stderr,"\n");
#endif

	st->reportnum++;

	updateRoutes(st,list);

	routeFree(list);
}

static detector *detectorInit(ManetAddr us, DetectorInit *detinit)
{
	detector *st;
	CommunicationsNeighbor *neighbors;

	st=(detector*)malloc(sizeof(*st));

	// st->cs=communicationsInit(us);
	// communicationsNameSet(st->cs,"routingdetector","");

    st->watcherClientPtr=new Client(detinit->serverName, detinit->serverPort);
    if(st->watcherClientPtr==NULL)
        return NULL;

	if (st->cs==NULL)
		return NULL;

	if (0==idsPositionRegister(st->cs, COORDINATOR_ROOT, IDSPOSITION_INFORM, positionUpdate, st))
	{
		fprintf(stderr, "%s(): idsPositionRegister(COORDINATOR_ROOT) failed\n",
			__func__);
	}
	if (0==idsPositionRegister(st->cs, COORDINATOR_ROOTGROUP, IDSPOSITION_INFORM, positionUpdate, st))
	{
		fprintf(stderr, "%s(): idsPositionRegister(COORDINATOR_ROOTGROUP) failed\n",
			__func__);
	}

	st->duration=detinit->duration;
	st->reportperiod=detinit->reportperiod;
	st->reportnum=0;
	if (detinit->iface)
		st->iface=strdup(detinit->iface);
	else
		st->iface=NULL;

	st->onehoplabel=detinit->onehoplabel;
	st->onehopcolor=detinit->onehopcolor; 

	st->nexthoplayer=detinit->nexthoplayer;
	st->nexthopcolor=st->nexthopcolor;

	st->routeedgewidth=detinit->routeedgewidth;

	// st->manetnetmask=communicationsNodeMask(st->cs);
	st->manetaddr=communicationsNodeAddress(st->cs);
	st->manetnetwork=st->manetaddr  & st->manetnetmask;
	st->tag=(communicationsNodeAddress(st->cs) & 0xFF ) | 0x12345600;

	st->root=NODE_BROADCAST;

	/* XXX should erase existing labels/edges here */

	communicationsNeighborRegister(st->cs, neighborUpdate, st);

	neighbors = communicationsNeighborList(st->cs);
	while (neighbors != NULL)
	{
		neighborUpdate(st, neighbors);
		neighbors = neighbors->next;
	}

#ifdef DEBUG
	fprintf(stderr,"tag= %d\n",st->tag);
#endif
	return st;
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and break out every 2 seconds to
 * send messages.
 *
 * The API is not threadsafe!
 */
static void selectLoop(detector *dt)
{
    fd_set readfds,writefds;
    int maxfd;
    int rc;
    int apifd;
    struct timeval nextreport,curtime;
    struct timeval endtime;
    struct timeval timeout;

    gettimeofday(&nextreport,NULL);
    memcpy(&endtime,&nextreport,sizeof(endtime));

    timeout.tv_sec=dt->reportperiod/1000;
    timeout.tv_usec=(dt->reportperiod % 1000) * 1000;
    timeradd(&curtime,&timeout,&nextreport);

    endtime.tv_sec+=dt->duration;

    while(1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        maxfd=-1;

        apifd=communicationsReturnFD(dt->cs);
        if (apifd>0)
        {
            FD_SET(apifd,&readfds);
            GETMAXFD(maxfd,apifd);
        }

        gettimeofday(&curtime,NULL);

        if ((dt->duration>0) && (timercmp(&curtime,&endtime,>)))
        {
            return;
        }

        if (timercmp(&curtime,&nextreport,>))
        {
            if (apifd<0)
                communicationsReadReady(dt->cs);
            detectorSend(dt);
            timeout.tv_sec=dt->reportperiod/1000;
            timeout.tv_usec=(dt->reportperiod % 1000) * 1000;
            timeradd(&curtime,&timeout,&nextreport);
        }
        timersub(&nextreport,&curtime,&timeout);
#if 0
        fprintf(stderr,"entering select.  timeout= %d\n",timeout.tv_sec);
#endif
        rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

        if (rc>0)
        {
            if ((apifd>0) && (FD_ISSET(apifd,&readfds)))
            {
#if 0
                fprintf(stderr,"API fd readable\n");
#endif
                communicationsReadReady(dt->cs);
            }
        }
    }
}

int main(int argc, char *argv[])
{
	detector *dt;
	unsigned int us=0;
	int ch;

	DetectorInit detinit;
	detinit.iface=NULL;
	detinit.onehopcolor=Color::green;
	detinit.onehoplayer="One Hop Routing";
	detinit.nexthopcolor=Color::blue;
	detinit.nexthoplayer="Network Routing"; 
	detinit.reportperiod=2000;
	detinit.duration=0;
	detinit.routeedgewidth=15; 

	while ((ch = getopt(argc, argv, "w:t:d:o:n:a:b:i:p:?")) != -1)
	switch (ch)
    {
        case 'o':
            detinit.onehopcolor=Color::fromString(optarg);
            break;
        case 'n':
            detinit.nexthopcolor=Color::fromString(optarg); 
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
            us=string(optarg);
            break;
        case 'p':
            sscanf(optarg,"%d",&detinit.reportperiod);
            break;
        case 'w':
            sscanf(optarg, "%d", &detinit.routeedgewidth); 
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


	dt=detectorInit(us,&detinit);

	if (dt==NULL)
	{
		fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
		exit(1);
	}
	printf("%s: starting\n",argv[0]);
	selectLoop(dt);

	return 0;
}
