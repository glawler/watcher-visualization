#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>

#include "idsCommunications.h"
#include "apisupport.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: routingdetector.c,v 1.29 2007/06/27 22:08:47 mheyman Exp $";

#define DEBUG 1

/* This is a debugging detector, which polls the routing table,
 * and draws the routing algorithm's edges on the watcher
 *
 *  Copyright (C) 2006,2007 Sparta Inc.  Written by the NIP group, SRD, ISSO
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

static Route *routeRead(ManetAddr manetnetwork,ManetAddr manetmask)
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
	CommunicationsStatePtr cs;
	int reportperiod;		/* frequency to generate reports at */
	int reportnum;
	int tag;
	ManetAddr manetaddr;
	ManetAddr manetnetwork;
	ManetAddr manetnetmask;
	char *iface;
	int onehopfamily;
	int nexthopfamily;
	int rootedgefamily;
	int rootedgeenable;
	int rootdistancefamily;
	int rootdistanceenable;
	int rootgroupenable;
	int routeedgewidth;
	unsigned char onehopcolor[4];
	unsigned char nexthopcolor[4];
	unsigned char lostroutecolor[4];
	unsigned char rootedgecolor[4];
	unsigned char rootdistancecolor[4];

	int count;                        /* number of expected routes on the manet network (or 0 to disable counting) */
	int duration;			  /* Time to run (in seconds) or 0 to run until broken */
	ManetAddr nodeFirst;
	ManetAddr nodeLast;

	ManetAddr othermouth;

	ManetAddr root;                   /* our root node (may be NODE_LOCAL, or NODE_BROADCAST for none) */
	int rootgroupflag;                /* are we in the root group? */
} detector;

typedef struct DetectorInit
{
	int reportperiod;		/* frequency to generate reports at */
	char *iface;
	int onehopfamily;
	int nexthopfamily;
	int rootedgefamily;
	int rootdistancefamily;
	int rootedgeenable;
	int rootdistanceenable;
	int rootgroupenable;
	unsigned char onehopcolor[4];
	unsigned char nexthopcolor[4];
	unsigned char lostroutecolor[4];
	unsigned char rootedgecolor[4];
	unsigned char rootdistancecolor[4];
	ManetAddr mouth[2];
	int mouthnum;
	int count;                        /* number of expected routes on the manet network (or 0 to disable counting) */
	int duration;			  /* Time to run (in seconds) or 0 to run until broken */
	ManetAddr nodeFirst;
	ManetAddr nodeLast;
	int routeedgewidth; 
} DetectorInit;

static void updateRoutes(detector *st, Route *list)
{
	Route *r,*tmpNextHop=NULL;
	Route *tmpOneHop=NULL;
	int count=0;
	int p;
	int routeflag[st->count+1];

	memset(routeflag,0,sizeof(routeflag));

/*
	for each edge in newlist,
		if there is not an edge in oldList or tmpList to nexthop, add it, and add to tmpList
*/
	for(r=list;r;r=r->next)
	{

		if ((st->iface) && (strcmp(st->iface,r->iface)!=0))    /* if we're checking interfaces, and this route is on the wrong one...  */
			continue;
	
		count++;                           /* missing route detection  */
		p=r->dst - st->nodeFirst;
		if ((p>=0) && (p<=st->count))
			routeflag[p]=1;
		else
			fprintf(stderr,"suprised to see route to address %d.%d.%d.%d  p= %d\n",PRINTADDR(r->dst),p);

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

	if (st->count)    /* if missing route counting is enabled...  */
	{
		NodeLabel lab;
		char strng[1024];
		ManetAddr nod;

		lab.node=NODE_LOCAL;
		lab.bgcolor[0]=255;
		lab.bgcolor[1]=255;
		lab.bgcolor[2]=255;
		lab.bgcolor[3]=255;
		lab.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
		lab.family=st->onehopfamily;
		lab.tag=st->tag;
		lab.text=NULL;
		lab.expiration=st->reportperiod*1.5;

		if (count!=st->count)
		{
			lab.fgcolor[0]=255;
			lab.fgcolor[1]=0;
			lab.fgcolor[2]=0;
			lab.fgcolor[3]=255;
		}
		else
		{
			lab.fgcolor[0]=0;
			lab.fgcolor[1]=0;
			lab.fgcolor[2]=255;
			lab.fgcolor[3]=255;
		}
		lab.text=strng;
		sprintf(strng,"missingroutes= %d",st->count-count);
		communicationsWatcherLabel(st->cs,&lab);

		for(nod=st->nodeFirst;nod<=st->nodeLast;nod++)
		{
			fprintf(stderr,"checking node %d.%d.%d.%d\n",PRINTADDR(nod));
			if (routeflag[nod-st->nodeFirst]==0)
			{
				fprintf(stderr,"missing route to %d.%d.%d.%d\n",PRINTADDR(nod));
				NodeEdge edge;
				edge.head=NODE_LOCAL;
				edge.tail=nod;
				edge.family=0;
				edge.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
				edge.tag=st->tag+2;
				edge.width=st->routeedgewidth;
				edge.expiration=st->reportperiod*1.5;
				memcpy(edge.color,st->lostroutecolor,sizeof(st->lostroutecolor));
				edge.labelHead.text=NULL;
				edge.labelMiddle.text=NULL;
				edge.labelTail.text=NULL;
				communicationsWatcherEdge(st->cs,&edge);
			}
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

static void setColor(detector *st)
{
	unsigned char rootColor[4] = {0,128,0,128}; /* dk green */
	unsigned char rgColor[4] = {160,32,240,128}; /* purple */
	if (st->rootgroupenable)
	{
		communicationsWatcherColor(st->cs,
					   NODE_LOCAL,
					   st->root == NODE_LOCAL
					   ? rootColor
					   : (st->rootgroupflag
					      ? rgColor
					      : NULL));
	}
}

static void setRootness(detector *st, ManetAddr addr, int distance, int isRoot)
{
	NodeLabel label;
	NodeEdge edge;

	/* set distance label params */
	memset(&label,0,sizeof(label));
	/* bg white */
	label.bgcolor[0]=255;
	label.bgcolor[1]=255;
	label.bgcolor[2]=255;
	label.bgcolor[3]=255;
	memcpy(label.fgcolor,st->rootdistancecolor,sizeof(st->rootdistancecolor));
	label.text=NULL;
	label.node=NODE_LOCAL;
	label.family=st->rootdistancefamily;
	label.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
	label.tag=0;
	label.expiration=0; /* never expire */

	/* set edge-to-root params */
	memset(&edge,0,sizeof(edge));
	edge.head=addr;
	edge.tail=NODE_LOCAL;
	edge.family=st->rootedgefamily;
	edge.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
	edge.tag=0;
	edge.width=st->routeedgewidth;
	edge.expiration=0; /* never expire */
	memcpy(edge.color,st->rootedgecolor,sizeof(st->rootedgecolor));
	edge.labelHead.text=NULL;
	edge.labelMiddle.text=NULL;
	edge.labelTail.text=NULL;

	if (isRoot)
	{
		/* set our distance-to-root label */
		char buf[80];
		sprintf(buf, "%d", distance);
		label.text=buf;
		if (st->rootdistanceenable)
		{
#ifdef DEBUG
			fprintf(stderr, "%s(): setting root-distance label \"%s\"\n",
				__func__, label.text);
#endif
			communicationsWatcherLabel(st->cs,&label);
		}
		
		/* create an edge from local node to the root */
		if (st->rootedgeenable && addr != NODE_LOCAL)
		{
#ifdef DEBUG
			fprintf(stderr, "%s(): setting root edge to %d\n",
				__func__, addr & 0xFF);
#endif
			communicationsWatcherEdge(st->cs,&edge);
		}
		st->root=addr; 
	}
	else
	{
		/* node is not our root */
		if (st->rootdistanceenable && st->root == addr) 
		{
			st->root=NODE_BROADCAST;
			/* remove distance label */
#ifdef DEBUG
			fprintf(stderr, "%s(): clearing root distance labels\n",
				__func__);
#endif
			communicationsWatcherLabelRemove(
				st->cs,
				(COMMUNICATIONS_LABEL_REMOVE_FAMILY
				 | COMMUNICATIONS_LABEL_REMOVE_PRIORITY
				 | COMMUNICATIONS_LABEL_REMOVE_TAG
				 | COMMUNICATIONS_LABEL_REMOVE_NODE),
				&label);
		}
		if (st->rootedgeenable && addr != NODE_LOCAL)
		{
#ifdef DEBUG
			fprintf(stderr, "%s(): clearing any root edges to %u\n",
				__func__, addr);
#endif
			/* remove edge to this node, if any */
			communicationsWatcherEdgeRemove
				(st->cs,
				 (COMMUNICATIONS_EDGE_REMOVE_HEAD
				  | COMMUNICATIONS_EDGE_REMOVE_TAIL
				  | COMMUNICATIONS_EDGE_REMOVE_FAMILY
				  | COMMUNICATIONS_EDGE_REMOVE_PRIORITY
				  | COMMUNICATIONS_EDGE_REMOVE_TAG),
				 edge.head,
				 edge.tail,
				 edge.family,
				 edge.priority,
				 edge.tag);
		}
	}
}

static void positionUpdate(void *data, IDSPositionType position, IDSPositionStatus positionStatus)
{
	detector *st = (detector *) data;
#ifdef DEBUG
	fprintf(stderr, "%s(%s, %s)\n",
		__func__,
		idsPosition2Str(position),
		idsPositionStatus2Str(positionStatus));
#endif
	if (position == COORDINATOR_ROOT)
	{
		setRootness(st, NODE_LOCAL, 0, 
			    (positionStatus == IDSPOSITION_ACTIVE));
	}
	if (position == COORDINATOR_ROOTGROUP)
	{
		st->rootgroupflag = (positionStatus == IDSPOSITION_ACTIVE);
		setColor(st);
	}
}

static void neighborUpdate(
    void *data, 
    CommunicationsNeighbor *communicationsNeighbor)
{
	detector *st = (detector *) data;
#ifdef DEBUG
	const char *stateStr = 
		communicationsNeighborState2Str(communicationsNeighbor->state);

	char typeStr[1024];
	strcpy(typeStr,"");
	if (communicationsNeighbor->type & COMMUNICATIONSNEIGHBOR_PARENT)
	{
		strcat(typeStr," PARENT");
	}
	if (communicationsNeighbor->type & COMMUNICATIONSNEIGHBOR_CHILD)
	{
		strcat(typeStr," CHILD");
	}
	if (communicationsNeighbor->type & COMMUNICATIONSNEIGHBOR_ROOT)
	{
		strcat(typeStr," ROOT");
	}
	fprintf(stderr, "%s(addr=%u.%u.%u.%u, type=%s, state=%s)\n",
		__func__, PRINTADDR(communicationsNeighbor->addr), typeStr, stateStr);
#endif
	setRootness(st, communicationsNeighbor->addr, communicationsNeighbor->distance,
		    (communicationsNeighbor->type & COMMUNICATIONSNEIGHBOR_ROOT));
}

static detector *detectorInit(ManetAddr us, DetectorInit *detinit)
{
	detector *st;
	CommunicationsNeighbor *neighbors;

	st=(detector*)malloc(sizeof(*st));
	st->cs=communicationsInit(us);
	communicationsNameSet(st->cs,"routingdetector","");

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

	st->onehopfamily=detinit->onehopfamily;
	memcpy(st->onehopcolor,detinit->onehopcolor,sizeof(st->onehopcolor));

	st->nexthopfamily=detinit->nexthopfamily;
	memcpy(st->nexthopcolor,detinit->nexthopcolor,sizeof(st->nexthopcolor));

	st->rootedgefamily=detinit->rootedgefamily;
	memcpy(st->rootedgecolor,detinit->rootedgecolor,sizeof(st->rootedgecolor));
	st->rootedgeenable=detinit->rootedgeenable;

	st->rootdistancefamily=detinit->rootdistancefamily;
	memcpy(st->rootdistancecolor,detinit->rootdistancecolor,sizeof(st->rootdistancecolor));
	st->rootdistanceenable=detinit->rootdistanceenable;

	st->rootgroupenable=detinit->rootgroupenable;

	st->routeedgewidth=detinit->routeedgewidth;

	st->manetnetmask=communicationsNodeMask(st->cs);
	st->manetaddr=communicationsNodeAddress(st->cs);
	st->manetnetwork=st->manetaddr  & st->manetnetmask;
	st->tag=(communicationsNodeAddress(st->cs) & 0xFF ) | 0x12345600;

	if (detinit->mouth[0]==communicationsNodeAddress(st->cs))
		st->othermouth=detinit->mouth[1];
	if (detinit->mouth[1]==communicationsNodeAddress(st->cs))
		st->othermouth=detinit->mouth[0];

	st->count=detinit->count;
	st->nodeFirst=detinit->nodeFirst;
	st->nodeLast=detinit->nodeLast;
	memcpy(st->lostroutecolor,detinit->lostroutecolor,sizeof(st->lostroutecolor));

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
	detinit.onehopcolor[0]=120;
	detinit.onehopcolor[1]=200;
	detinit.onehopcolor[2]=0;
	detinit.onehopcolor[3]=128;
	detinit.onehopfamily=COMMUNICATIONS_LABEL_FAMILY_ROUTING_ONEHOP;
	detinit.nexthopcolor[0]=0;
	detinit.nexthopcolor[1]=255;
	detinit.nexthopcolor[2]=0;
	detinit.nexthopcolor[3]=128;
	detinit.nexthopfamily=COMMUNICATIONS_LABEL_FAMILY_ROUTING;
	detinit.rootedgecolor[0]=69; /* aquamarine4 */
	detinit.rootedgecolor[1]=139;
	detinit.rootedgecolor[2]=116;
	detinit.rootedgecolor[3]=128;
	detinit.rootedgefamily=COMMUNICATIONS_LABEL_FAMILY_UNDEFINED;
	detinit.rootedgeenable=0;
	detinit.rootdistancecolor[0]=139;	/* sienna4 */
	detinit.rootdistancecolor[1]=71;
	detinit.rootdistancecolor[2]=38;
	detinit.rootdistancecolor[3]=128;
	detinit.rootdistancefamily=COMMUNICATIONS_LABEL_FAMILY_UNDEFINED;
	detinit.rootdistanceenable=0;
	detinit.rootgroupenable=0;
	detinit.mouth[0]=0;
	detinit.mouth[1]=0;
	detinit.mouthnum=0;
	detinit.reportperiod=2000;
	detinit.count=0;
	detinit.lostroutecolor[0]=255;
	detinit.lostroutecolor[1]=0;
	detinit.lostroutecolor[2]=255;
	detinit.lostroutecolor[3]=128;
	detinit.duration=0;
	detinit.routeedgewidth=15; 

	while ((ch = getopt(argc, argv, "w:t:d:o:n:a:b:i:m:p:c:f:l:r:?R")) != -1)
	switch (ch)
	{
		case 'o':
		{
			int tmpcolor;
			tmpcolor=ntohl(inet_addr(optarg));
			detinit.onehopcolor[0]=tmpcolor >> 24;
			detinit.onehopcolor[1]=tmpcolor >> 16;
			detinit.onehopcolor[2]=tmpcolor >> 8;
			detinit.onehopcolor[3]=tmpcolor;
		}
		break;
		case 'n':
		{
			int tmpcolor;
			tmpcolor=ntohl(inet_addr(optarg));
			detinit.nexthopcolor[0]=tmpcolor >> 24;
			detinit.nexthopcolor[1]=tmpcolor >> 16;
			detinit.nexthopcolor[2]=tmpcolor >> 8;
			detinit.nexthopcolor[3]=tmpcolor;
		}
		break;
		case 'r':
		{
			int tmpcolor;
			tmpcolor=ntohl(inet_addr(optarg));
			detinit.lostroutecolor[0]=tmpcolor >> 24;
			detinit.lostroutecolor[1]=tmpcolor >> 16;
			detinit.lostroutecolor[2]=tmpcolor >> 8;
			detinit.lostroutecolor[3]=tmpcolor;
		}
		break;
		case 't':
			sscanf(optarg,"%d",&detinit.duration);
		break;
		case 'a':
			sscanf(optarg,"%d",&detinit.onehopfamily);
		break;
		case 'b':
			sscanf(optarg,"%d",&detinit.nexthopfamily);
		break;
		case 'c':
			sscanf(optarg,"%d",&detinit.count);
		break;
		case 'f':
			detinit.nodeFirst=communicationsHostnameLookup(optarg);
			detinit.count=detinit.nodeLast-detinit.nodeFirst+1;
		break;
		case 'l':
			detinit.nodeLast=communicationsHostnameLookup(optarg);
			detinit.count=detinit.nodeLast-detinit.nodeFirst+1;
		break;


		case 'i':
			detinit.iface=strdup(optarg);
		break;
		case 'd':
			us=communicationsHostnameLookup(optarg);
		break;
		case 'm':
			if (detinit.mouthnum <= (int)(sizeof(detinit.mouth)/sizeof(detinit.mouth[0])))
				detinit.mouth[detinit.mouthnum++]=communicationsHostnameLookup(optarg);
		break;
		case 'p':
			sscanf(optarg,"%d",&detinit.reportperiod);
		break;
		case 'R':
			detinit.rootedgeenable=1;
			detinit.rootdistanceenable=1;
			detinit.rootgroupenable=1;
		break;
		case 'w':
			sscanf(optarg, "%d", &detinit.routeedgewidth); 
		break;
		case '?':
		default:
			fprintf(stderr,"routingdetector - poll the linux routing table, and draw the routes in the watcher\n"
				"-d ipaddr - specify node to connect to (duck) (default is localhost)\n"
				"-p milliseconds - specify the period to poll and report\n"
				"-m ipaddr -  specify a wormhole mouth.  Must be used twice, to specify both\n"
				"-i interface - display only routes through this ethernet interface (or any if unspecified)\n"
				"-o num.num.num.num - onehop edge color\n"
				"-a num - onehop family (layer)\n"
				"-n num.num.num.num - nexthop edge color\n"
				"-b num - nexthop family (layer)\n"
				"-c num - enable counting routes, the number being the expected number of routes\n"
				"-f ipaddr - specify first node IP address (for -c option, which isn't needed if -f is used)\n"
				"-l ipaddr - specify last node IP address (for -c option, which isn't needed if -l is used)\n"
				"-r num.num.num.num - lost route edge color\n"
				"-t seconds - Run for this long, then exit\n"
				"-R - draw edge to root node and make root-distance labels\n"
				"-w width - make edges width pixels wide (default 15)\n"
				);
			exit(1);
		break;
	}

	if (detinit.count<0)
	{
		fprintf(stderr,"illegal values for first or last node address.\n");
		exit(1);
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
