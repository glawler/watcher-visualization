#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "metric.h"
#include "simulation.h"
#include "graphics.h"
#include "routing.h"
#include "node.h"

#include "marshal.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: interim.cpp,v 1.16 2007/04/23 18:51:07 dkindred Exp $";

/* Debug ClusterHead selection
*/
// #define DEBUG_CH
// #define DEBUG_HELLO
// #define DEBUG_LINKSTATE


typedef struct clusteringState
{
	int hellosequencenum;
	int hellocount[MAXLEVEL];              /* number of HELLOs we've sent  */
	int ttl[MAXLEVEL];
	int rootflag;
	int maxTTL;
} clusteringState;


static void nodeHelloTimer(struct manetNode *, void *data);
static void nodeHelloTimeoutTimer(manetNode *, void *data);
static void nodeFindClusterheads(manetNode *us);
void gotHello(manetNode *us, packet *p);

static packet *helloMarshal(manetNode *us, packet_hello *p)
{
	packet *np;
	int len;
	unsigned char *hp;
	int i;

	len=	1 +	/* p->onehopcount */
		1 +	/* p->symcount */
		1 +	/* p->level */
		4 +	/* p->clusterhead */
		4 +	/* p->sequencenum */
		(4 * p->onehopcount)   /* hp->addresslist */;

	np=packetMalloc(us,len);
	hp=(unsigned char *)np->data;
	
	MARSHALBYTE(hp,p->onehopcount);
	MARSHALBYTE(hp,p->symcount);
	MARSHALBYTE(hp,p->level);
	MARSHALLONG(hp,p->clusterhead);
	MARSHALLONG(hp,p->sequencenum);
	for(i=0;i<p->onehopcount;i++)
		MARSHALLONG(hp,p->addresslist[i]);
	return np;
}

static packet_hello *helloUnmarshal(packet *np)
{
	packet_hello *p;
	unsigned char *hp;
	int i;

	p=(packet_hello*)malloc(sizeof(*p));
	hp=(unsigned char *)np->data;
	
	UNMARSHALBYTE(hp,p->onehopcount);
	UNMARSHALBYTE(hp,p->symcount);
	UNMARSHALBYTE(hp,p->level);
	UNMARSHALLONG(hp,p->clusterhead);
	UNMARSHALLONG(hp,p->sequencenum);
	for(i=0;i<p->onehopcount;i++)
		UNMARSHALLONG(hp,p->addresslist[i]);
	return p;
}

/* This is called by simulation code on startup
*/
void nodeInit(manetNode *n)
{
	n->clusterhead=NULL;
	n->neighborlist=NULL;
	n->level=0;
	n->rootflag=1;

	n->cluster=(clusteringState*)malloc(sizeof(*(n->cluster)));

	n->cluster->hellosequencenum=0;
	n->cluster->rootflag=0;
	n->cluster->maxTTL=configSearchInt(n->manet->conf,"interim_maxTTL");
	if (n->cluster->maxTTL<1)
		n->cluster->maxTTL=255;

	memset(n->cluster->hellocount,0,sizeof(n->cluster->hellocount));
	memset(n->packetlist,0,sizeof(n->packetlist));
	n->packetlistpos=0;
	n->packetlistlen=0;
	memset(n->cluster->ttl,0,sizeof(n->cluster->ttl));

	timerSet(n,nodeHelloTimer,TIME_HELLO, NULL);
	nodeHelloTimeoutTimer(n, NULL);

#if 0
	routeInit(n);
	if (n->addr==2)
	{
		timerSet(n,(timercallback*)routeTestSend,10);
	}
#endif
	manetPacketHandlerSet(n, PACKET_INTERIM_HELLO, gotHello);
}

void nodeFree(manetNode *n)
{
}

/* Hello timeout timer
** called every HELLO interval, to remove HEARD and HEARS flags from entries from the neighbor list
** which we havn't heard a HELLO from recently enough
*/
void nodeHelloTimeoutTimer(manetNode *us, void *data)
{
	neighbor *n;

	n=us->neighborlist;
	while(n)
	{
		if ((us->manet->curtime-n->lastheard) > TIME_HELLO_TIMEOUT)
		{
			if (n->flags & NEIGHBOR_HEARD)
#ifdef DEBUG_LINKSTATE
				fprintf(stderr,"node %d: node %d timed out\n",us->addr & 0xFF,n->addr & 0xFF);   /* lost an edge, call the OLSR stuff */
#endif
			n->flags&=~(NEIGHBOR_HEARD | NEIGHBOR_HEARS);
			n->hopcount=MAXHOP;
		}
		n=n->next;
	}
	timerSet(us,nodeHelloTimeoutTimer,TIME_HELLO_TIMEOUT, NULL);
}



/* callback for hello timer went off
**
** Will build all the HELLO packets which this node is to send
**
** A node may send several hello packets, with different TTLs.
**  a level 0 node sends a single hello, at TTL=0.  
**  a level 1 node sends two hellos, a level 0 hello at TTL=0, and a level 1 TTL at some computed TTL
**  a level 2 node sends three, etc. 
*/
void nodeHelloTimer(manetNode *us, void *data)
{
	packet *p;
	packet_hello *hp,realhp;
	neighbor *n;
	unsigned int ncount,nsymneighbors;
	int l;
	int noise;

		/* first, reschedule...  */
	noise=(int)(drand48()*5.0)-2;   /* -2 to +2 */
	timerSet(us,nodeHelloTimer,TIME_HELLO+noise, NULL);

	nodeFindClusterheads(us);

	hp=&realhp;

	for(l=0;l<=us->level;l++)
	{
		hp->level=l;
		hp->sequencenum=us->cluster->hellosequencenum++;
		ncount=0;
		nsymneighbors=0;

#ifdef DEBUG_HELLO
		fprintf(stderr,"node %d: HELLO Level %d  CH= %d seqnum= %d TTL= %d\n",us->addr & 0xFF,l,(us->clusterhead?(us->clusterhead->addr):(us->addr)) & 0xFF,hp->sequencenum,p->ttl);
#endif
		n=us->neighborlist;
		while(n)
		{
			if ((n->level==l) && (n->flags & NEIGHBOR_HEARD) && (ncount<(sizeof(hp->addresslist)/sizeof(hp->addresslist[0]))))
			{
				hp->addresslist[ncount]=n->addr;
				ncount++;
				if (n->flags & NEIGHBOR_HEARS)
					nsymneighbors++;
#ifdef DEBUG_HELLO
				fprintf(stderr,"node %d:    %d\n",us->addr & 0xFF,n->addr & 0xFF);
#endif
			}
			n=n->next;
		}
		hp->onehopcount=ncount;
		hp->symcount=nsymneighbors;
		if (us->cluster->hellocount[l]>=HELLO_CLUSTERHEAD)
		{
			if (l<us->level)
			{
				hp->clusterhead=us->addr;
			}
			else
				hp->clusterhead=(us->clusterhead)?(us->clusterhead->addr):(us->addr);
		}
		else
			hp->clusterhead=NODE_BROADCAST;
		us->cluster->hellocount[l]++;

		p=helloMarshal(us,hp);
		p->type=PACKET_INTERIM_HELLO;
		p->dst=NODE_BROADCAST;
#if 1
		p->ttl=(l>0)?us->cluster->ttl[l]:0;   /* insist that level 0 is with a ttl of 0 */
#else
		p->ttl=(l==0)?0:(l*2);
#endif
		packetSend(us,p, PACKET_ORIGIN);
		packetFree(p);
	}

}


/* Called by simulation code when a packet arrives at a node
*/
void gotHello(manetNode *us, packet *p)
{
	packet *np;

#ifdef DEBUG_INTERIM
	fprintf(stderr,"node %d got a packet from %d to %d at %lld ttl= %d\n",us->addr & 0xFF,p->src & 0xFF,p->dst & 0xFF ,us->manet->curtime,p->ttl);
#endif

	/* Should this be in the des code, or in the simulation?  I'm going to put it here for now...
	*/

	if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
	{
		return;
	}

	if (nodePacketSearch(us,p)!=NULL)    /* if we've already seen this packet, ignore it */
		return;
	nodePacketInsert(us,p);

/* Enable repeating of HELLO packets...  */
#if 1
	if ((p->ttl>0) && (p->type==PACKET_INTERIM_HELLO))
	{
#ifdef DEBUG_INTERIM
		fprintf(stderr,"node %d forwarding packet from %d at %lld ttl= %d\n",us->addr & 0xFF,p->src & 0xFF,us->manet->curtime,p->ttl);
#endif
		np=packetCopy(p,0);
		np->hopcount++;                              /* then make a copy, update the ttl, and repeat it */
		np->ttl--;
		packetSend(us,np, PACKET_REPEAT);
		packetFree(np);
	}
#endif

	switch(p->type)
	{
		case PACKET_INTERIM_HELLO:             /* we received a HELLO packet */
		{
			neighbor *n;
			packet_hello *hp=helloUnmarshal(p);
			int i;

			if (hp->level>us->level)     /* its for a level above us... we don't care */
			{
				free(hp);
				break; 
			}

#ifdef DEBUG_INTERIM
			fprintf(stderr,"node %d: got a level %d HELLO packet from %d  CH= %d\n",us->addr & 0xFF,hp->level,p->src & 0xFF,hp->clusterhead & 0xFF);
#endif

			n=neighborSearch(us,p->src,hp->level);
			if (n==NULL)
				n=neighborInsert(us,p->src,hp->level);
			n->lastheard=us->manet->curtime;
			n->clusterhead=hp->clusterhead;
			if (n->clusterhead==us->addr)
				n->flags|=NEIGHBOR_CHILD;
			else
				n->flags&=~NEIGHBOR_CHILD;
			n->level=hp->level;
			n->flags|=NEIGHBOR_HEARD;    /* we heard them...  */

			n->hopcount=p->hopcount;
			assert(p->hopcount>0);
			assert(n->hopcount>0);

			n->onehopdegree=hp->symcount;

			for(i=0;i<hp->onehopcount;i++)
			{
				if (hp->addresslist[i]==us->addr)
				{
					if (!(n->flags & NEIGHBOR_HEARS))
					{
#ifdef DEBUG_LINKSTATE
						fprintf(stderr,"node %d: just got a new symmetric link %d.\n",us->addr & 0xFF, n->addr & 0xFF);    /* got a new symmetric link... call the OLSR stuff */
#endif
						n->flags|=NEIGHBOR_HEARS;   /* they listed us, so they hear us  */
						n->firstheard=us->manet->curtime;
					}
				}
			}
			free(hp);
		}
		break;
		default:
			fprintf(stderr,"node %d: got an unknown packet type 0x%x from %d\n",us->addr & 0xFF,p->type,p->src & 0xFF);
			exit(1);
		break;
	}
}

/* Called when a node wants to send a hello packet.  Decides which neighbors to use for
** clusterheads.
*/
static void nodeFindClusterheads(manetNode *us)
{
	int maxdegree;
	neighbor *maxdegreeneighbor;
	neighbor *remotechild;
	neighbor *remoteclusterhead;
	neighbor *n;
	int l=0;
	int maxhopcount;
	int lastheard;

/* The pitbull rule:  when we find a clusterhead, use it till we can no longer contact it
*/
#if 0
	if (us->clusterhead!=NULL)    /* pitbull mode: only let go of clusterhead if we lose contact */
	{
		n=neighborSearch(us,us->clusterhead->addr,us->level);
		if ((n->flags & NEIGHBOR_HEARD) && (n->flags & NEIGHBOR_HEARS))
			return;
	}
#endif

	maxdegreeneighbor=NULL;
	us->cluster->rootflag=0;
	while((!us->cluster->rootflag) && (maxdegreeneighbor==NULL) && (us->cluster->hellocount[l]>=HELLO_CLUSTERHEAD))
	{
		remotechild=NULL;
		remoteclusterhead=NULL;

		maxhopcount=0;
		maxdegree=0;
		maxdegreeneighbor=NULL;
		n=us->neighborlist;
		while(n)
		{
			if ((n->level==l) && (n->flags & NEIGHBOR_HEARD) && (n->flags & NEIGHBOR_HEARS))
			{
				lastheard=n->lastheard;
				maxdegree++;
#ifdef DEBUG_CH
				fprintf(stderr,"node %d: level %d neighbor %d "
                               "degree= %d CH= %d  hopcount= %d "
                               "lastheard= %lld\n",
					us->addr & 0xFF,
					l,
					n->addr & 0xFF,
					n->onehopdegree,
					n->clusterhead & 0xFF,
					n->hopcount,
					n->lastheard);
#endif
			}
			n=n->next;
		}
#ifdef DEBUG_CH
		fprintf(stderr,"node %d: at level %d my degree is %d\n",us->addr & 0xFF,l,maxdegree);
#endif

		if ((l==0) && (maxdegree==0))
		{
			us->rootflag=1;
			us->level=0;
			us->clusterhead=NULL;
			return;
		}
		n=us->neighborlist;
		while(n)
		{
			if ((n->level==l) && (n->flags & NEIGHBOR_HEARD) && (n->flags & NEIGHBOR_HEARS))
			{
#if 1
				if ((n->onehopdegree>maxdegree) ||
					((maxdegreeneighbor!=NULL) && (n->onehopdegree==maxdegree) && (n->addr < maxdegreeneighbor->addr)) ||
					((maxdegreeneighbor==NULL) && (n->onehopdegree==maxdegree) && (n->addr < us->addr))
					)
				{
					maxdegree=n->onehopdegree;
					maxdegreeneighbor=n;
				}
#else
				if ((n->lastheard < lastheard) || 
					((maxdegreeneighbor != NULL) && (n->lastheard == lastheard) && (n->addr < maxdegreeneighbor->addr))
					)
				{
						maxdegreeneighbor=n;
						lastheard=n->lastheard;
				}
#endif
				if (n->clusterhead==n->addr)
				{
					if ((remoteclusterhead==NULL) ||
						((remoteclusterhead!=NULL) && (n->onehopdegree==remoteclusterhead->onehopdegree) && (n->addr < remoteclusterhead->addr)) ||
						((remoteclusterhead!=NULL) && (n->onehopdegree > remoteclusterhead->onehopdegree))
						)
						remoteclusterhead=n;
				}

				if (n->clusterhead==us->addr)
				{
#ifdef DEBUG_CH
					fprintf(stderr,"node %d: level %d I am CH for %d\n",us->addr & 0xFF,l,n->addr & 0xFF);
#endif
					remotechild=n;
				}
			}
			if (n->hopcount>maxhopcount)
				maxhopcount=n->hopcount;
			n=n->next;
		}

/* enable remoteclusterhead rule  */
#if 1
		if (remoteclusterhead)
		{
			maxdegreeneighbor=remoteclusterhead;
//			maxdegree=10000;
		}
#endif

/* enable remotechild rule */
#if 1
		if (remotechild)
		{
#ifdef DEBUG_CH
			fprintf(stderr,"node %d: child is %d. being CH\n",us->addr & 0xFF,remotechild->addr & 0xFF);
#endif
			maxdegreeneighbor=NULL;
//			maxdegree=10000;
		}
#endif

		/* node of max degree is now pointed to by maxdegreeneighbor  (or NULL if its us)  */
		if ((maxdegreeneighbor==NULL) && (maxdegree>0))    /* we can only "win" if we have peers  */
			l++;

		if ((maxdegree==0) && (remoteclusterhead==NULL) && (remotechild==NULL))              /* insufficient peers found, look in a wider area */
		{
			if (us->cluster->ttl[l]>=us->cluster->maxTTL)     /* there is no wider area...  we are root!  */
			{
				us->cluster->ttl[l]=us->cluster->maxTTL;
				if ((us->cluster->hellocount[l]>=HELLO_CLUSTERHEAD) && (l>0))
				{
#ifdef DEBUG_CH
					fprintf(stderr,"node %d: we're root\n",us->addr & 0xFF);
#endif
					us->cluster->rootflag=1;
				}
			}
			else
			{
				us->cluster->ttl[l]=us->cluster->ttl[l]*2;
				if (us->cluster->ttl[l]>=us->cluster->maxTTL)
					us->cluster->ttl[l]=us->cluster->maxTTL;

#if 1
				if (us->cluster->ttl[l] < maxhopcount)
					us->cluster->ttl[l]=maxhopcount;
#endif

				us->cluster->hellocount[l]=0;    /* reset hello count, so as not to increment until after 2 packets are sent */
			}
			break;
		}
/* TTL management thing...  does not work
**
** The plan is for a node to figure out how many neighbors it wants to have,
** then sort its known neighbors by their hop count, and use use a ttl just large enough to get
** to the most n'th most distant one (where n is the number of neighbors it wants to have)
**
** The problem is that there will exist a node which due to the shape of the network can only
** get to nodes, where is is n+m'th most distant node (where m is 1 or greater).  That node then
** ignores it, and the remote node will eventually decide that it is root, since it is not getting
** any responses at its level.
*/
#if 0
		else    /* we have sufficient neighbors...  do the ttl management thing */
		{
			int p,j;

			for(j=0;j<5;j++)
				minttl[j]=maxhopcount;

			n=us->neighborlist;
			while(n)                    /* find 5 least hopcounts */
			{
				if ((n->level==l) && (n->flags & NEIGHBOR_HEARD) && (n->flags&NEIGHBOR_HEARS))
				{
					j=5;
					while((j>0) && (n->hopcount < minttl[j-1]))
						j--;
					if (j<5)
					{
						for(p=4;p>j;p--)
							minttl[p]=minttl[p-1];
						minttl[j]=n->hopcount;
					}
				}
				n=n->next;
			}
			us->cluster->ttl[l]=minttl[4];
		}
#endif
	}

	us->clusterhead=maxdegreeneighbor;
	us->level=l;
	if (us->cluster->rootflag)
	{
		us->rootflag=1;
		us->cluster->ttl[l]=us->cluster->maxTTL;
	}
	else
		us->rootflag=0;

	for(l=us->level+1;l<MAXLEVEL;l++)
	{
		us->cluster->hellocount[l]=0;
		us->cluster->ttl[l]=l*2;
	}

#ifdef DEBUG_CH
	fprintf(stderr,"node %d: level= %d my clusterhead is %d  ttl= %d %d %d %d %d %d\n",us->addr & 0xFF,us->level,(us->clusterhead==NULL?us->addr:us->clusterhead->addr) & 0xFF,us->cluster->ttl[0],us->cluster->ttl[1],us->cluster->ttl[2],us->cluster->ttl[3],us->cluster->ttl[4],us->cluster->ttl[5]);
#endif

	for(n=us->neighborlist;n;n=n->next)
		n->flags&=~NEIGHBOR_PARENT;

	if (us->clusterhead)
		us->clusterhead->flags|=NEIGHBOR_PARENT;
}
