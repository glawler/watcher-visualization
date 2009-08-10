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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <errno.h>

#include "des.h"
#include "mobility.h"
#include "data.h"
#include "routing.h"
#include "flood.h"
#include "testtraffic.h"
#include "hello.h"
#include "packetapi.h"

/* needed because some simulation-only stuff got into manetDraw 
*/
#include "simulation.h"
#include <sys/time.h>

#define MIN(a,b)  (((a)>(b))?(b):(a))

#ifdef DEBUG_PACKET
static packet *debuglist=NULL;
#endif

/* allocates a packet with len data bytes
**
** only really does one malloc, so one free is sufficient.
** note the reference counting stuff.  This is to avoid copying packets
** when they are going to be received by multiple nodes.
*/
packet *packetMallocInt(manetNode *us, int len, char *, int )
{
	packet *p;

	p=(packet*)malloc(sizeof(*p)+len);
	if (p==NULL)
	{	
		fprintf(stderr,"packetMallocInt: malloc %d bytes failed!\n",sizeof(*p)+len);
		abort();
	}
	p->src=us->addr;
	p->dst=0;
	p->type=0;
	p->len=len;
	p->ttl=1;
	p->hopcount=1;
	p->data=((char*)p)+sizeof(*p);

	p->refcount=1;
	p->next=NULL;

#ifdef DEBUG_PACKET
	p->originnode=us;
	p->mallocfile=fil;
	p->mallocline=lin;
	p->malloctime=us->manet->curtime;
	p->dupfile="unduped";
	p->dupline=lin;
	p->duptime=-1;

	if (debuglist)
		assert(debuglist->debugprev==NULL);

	p->debugnext=debuglist;
	if (p->debugnext)
		p->debugnext->debugprev=p;
	p->debugprev=NULL;

	debuglist=p;
#endif

	return p;
}

/* Shrink/grow a packet structure, to the new len.
 * Call this only after a packetMalloc.
 * the pointer may be changed
 */
packet *packetRemalloc(manetNode *, int len,packet *oldp)
{
	packet *newp;

	newp=(packet*)realloc(oldp,sizeof(*newp)+len);
	if (newp==NULL)
	{	
		fprintf(stderr,"%s: realloc %d bytes failed!\n",__func__,sizeof(*newp)+len);
		abort();
	}
	newp->data=(char*)(newp+1);

#ifdef DEBUG_PACKET
#error implement walking the linked list looking for the old address, and updating it
#endif

	return newp;
}

#ifdef DEBUG_PACKET
void packetDumpDebug(void)
{
	packet *p;

	fprintf(stderr,"packet  type  refcount  time  mallocfilename, mallocline dupfilename, dupline\n");
	for(p=debuglist;p!=NULL;p=p->debugnext)
		fprintf(stderr,"%8p  %2x  %3d %10lld %20s %4d  %20s %4d\n",p,p->type,p->refcount,p->malloctime,p->mallocfile,p->mallocline,p->dupfile, p->dupline);
}
#endif

/* Duplicate a packet.  
**
** This is actually just a reference count operation
** Thus, a duplicated packet may not be modified.
*/
packet *packetDupInt(packet *p, char *, int)
{
#ifdef DEBUG_PACKET
	if (fil!=NULL)
	{
		p->dupfile=fil;
		p->dupline=lin;
		p->duptime=-1;
	}
#endif
	p->refcount++;
	return p;
}

/* Copy a packet.
**
** A copy may be modified.  Things like subtracting one from the TTL on repeat
** count as a modification.
*/
packet *packetCopyInt(packet const *o, int payloaddelta, char *, int)
{
	packet *p;

	p=(packet*)malloc(sizeof(*p)+o->len+payloaddelta);
	memcpy(p,o,sizeof(*p));
	p->next=NULL;

#ifdef DEBUG_PACKET
	p->mallocfile=fil;
	p->mallocline=lin;
	p->malloctime=p->originnode->manet->curtime;

	if (debuglist)
		assert(debuglist->debugprev==NULL);

	p->debugnext=debuglist;
	if (p->debugnext)
		p->debugnext->debugprev=p;
	p->debugprev=NULL;

	debuglist=p;
#endif

	p->len+=payloaddelta;
	p->data=((char*)p)+sizeof(*p);
	memcpy(p->data,o->data,MIN(p->len,o->len));
	p->refcount=1;
	return p;
}

/* Free a packet.  Dosn't matter how you got it, Malloc, Dup, or Copy
*/
void packetFree(packet *p)
{
	p->refcount--;
	if (p->refcount<0)
	{
		fprintf(stderr,"Refcount Error!  refcount= %d  src= %d dst= %d type= %x len= %d\n",p->refcount,p->src & 0xFF, p->dst & 0xFF, p->type,p->len);
#ifdef DEBUG_PACKET
		fprintf(stderr,"%8p  %2x  %3d %10lld %20s %4d\n",p,p->type,p->refcount,p->malloctime,p->mallocfile,p->mallocline);
		packetDumpDebug();
#endif
	}
	if (p->refcount==0)
	{
#ifdef DEBUG_PACKET
		if (debuglist==p)
		{
			assert(debuglist->debugprev==NULL);
			debuglist=debuglist->debugnext;
			if (debuglist)
				debuglist->debugprev=NULL;
		}
		else
		{
			if (p->debugprev)
				p->debugprev->debugnext=p->debugnext;
			if (p->debugnext)
				p->debugnext->debugprev=p->debugprev;
		}
#endif
		free(p);
	}
}


/* DES event list code
**
**  There is a free list for event list nodes.
**  It is in the global variable eventlistfree.
**  
**  The event list is almost but not quite a priority queuue
**  The sort is not stable, events scheduled for the same time will happen in
**  an undefined order.  
**
**  The event list assumes that there will be large numbers of events at the same
**  time.  
*/

#define EVENTLISTBUCKET 4096
eventnode *eventlistfree=NULL;
eventbucket *eventbucketfree=NULL;

#if 0
static void eventnodeFreeWalk(void)
{
	eventnode *en;

	en=eventlistfree;
	while(en)
	{
		assert(en->magic==0xfeedcfef);
		assert(en->prev==NULL);
		en=en->next;
	}
}
#endif

static eventnode *eventnodeMalloc(void)
{
	eventnode *pn;

	if (eventlistfree==NULL)	/* if free list is empty...  */
	{
		int i;			/* malloc another chunk, and dice it  */

		eventlistfree=(eventnode*)malloc(EVENTLISTBUCKET*sizeof(*eventlistfree));    /* should be on a free list */
		if (eventlistfree==NULL)
		{
			fprintf(stderr,"malloc failed!\n");
			abort();
		}

		for(i=0;i<EVENTLISTBUCKET-1;i++)
			eventlistfree[i].next=&(eventlistfree[i+1]);

		eventlistfree[EVENTLISTBUCKET-1].next=NULL;

#if 0
		for(i=0;i<EVENTLISTBUCKET;i++)
		{
			eventlistfree[i].magic=0xfeedcfef;
			eventlistfree[i].xmittime=0;
			eventlistfree[i].rectime=0;
			eventlistfree[i].prev=NULL;
			VALGRIND_SET_WATCHPOINT(&(eventlistfree[i].next),sizeof(eventlistfree[i].next));
		}
#endif
	}

	pn=eventlistfree;
	eventlistfree=eventlistfree->next;

	pn->type=0;
	pn->fd=-1;
	pn->data.undef=NULL;
	pn->callback=NULL;
	pn->nodeaddr=NULL;
	pn->next=NULL;
	pn->prev=NULL;
	pn->bucket=NULL;
	return pn;
}

static eventbucket *eventbucketMalloc(void)
{
	eventbucket *pn;

	if (eventbucketfree==NULL)
	{
		int i;

		eventbucketfree=(eventbucket*)malloc(EVENTLISTBUCKET*sizeof(*eventbucketfree));    /* should be on a free list */

		for(i=0;i<EVENTLISTBUCKET-1;i++)
			eventbucketfree[i].next=&(eventbucketfree[i+1]);

		eventbucketfree[EVENTLISTBUCKET-1].next=NULL;
	}

	pn=eventbucketfree;
	eventbucketfree=eventbucketfree->next;

	pn->next=NULL;
	pn->prev=NULL;
	pn->bucket=NULL;
	return pn;
}

void eventnodeFree(eventnode *en)
{
	en->type=0;
	en->fd=-1;
	en->data.undef=NULL;
	en->callback=NULL;
	en->nodeaddr=NULL;
	en->prev=NULL;
	en->bucket=NULL;
	en->next=eventlistfree;
	eventlistfree=en;
}

static void eventbucketFree(eventbucket *en)
{
	en->next=eventbucketfree;
	eventbucketfree=en;
}

/* insertion sort of a new eventlistnode into the event list
*/
static void eventnodeEnqueue(manet *m, eventnode *en)
{
	eventbucket *e,*pe,*nb;

	switch(en->type)
	{
		case EVENT_TICK:
		{
			en->next=m->ticklist;
			m->ticklist=en;
			en->bucket=NULL;
			return;
		}
		break;
		case EVENT_FDREAD:
		case EVENT_FDWRITE:
		{
			en->next=m->fdlist;
			m->fdlist=en;
			en->bucket=NULL;
			return;
		}
		case EVENT_TIMER:
		case EVENT_PACKET:
		case EVENT_REPACKET:
		break;
		default:
			fprintf(stderr,"illegal event type %d.\n",en->type);
			abort();
		break;
	}

	e=m->eventlist;
	pe=NULL;

	while((e) && (e->rectime < en->rectime))    /* walk buckets looking for time...  */
	{
		pe=e;
		e=e->next;
	}

	if ((e) && (e->rectime==en->rectime))
	{
		en->next=e->bucket;       /* found exact bucket...  put new node into [unodered] bucket */
		en->prev=NULL;
		if (en->next)
			en->next->prev=en;
		e->bucket=en;

		en->bucket=e;            /* event node points to the bucket its in  */
		return;
	}

	assert((e!=NULL)?(e->rectime > en->rectime):1);
	
	nb=eventbucketMalloc();         /* otherwise, make a new bucket  */
	nb->rectime=en->rectime;
	nb->bucket=en;
	en->bucket=nb;
	en->next=NULL;
	en->prev=NULL;

	if (pe)                              /* and insert-sort it into the list  */
	{
		nb->next=e;
		nb->prev=pe;
		if (nb->next)
			nb->next->prev=nb;
		pe->next=nb;
	}
	else
	{
		assert(m->eventlist==e);
		nb->next=e;
		nb->prev=NULL;
		if (e)
			e->prev=nb;
		m->eventlist=nb;
	}
}

/* enqueue packet p to be received by node us
**
**  This is an internal function, but needs to be visibe in livenetwork.cpp and main.cpp
*/
eventnode *packetEnqueue(manetNode *us,packet *p,int delay)
{
	eventnode *en;

	en=eventnodeMalloc();

	en->type=EVENT_PACKET;
	en->data.pkt=(packet*)packetDup(p);
	en->callback=NULL;
	en->xmittime=us->manet->curtime;
	en->rectime=en->xmittime+delay;         /* XXX  Propigation time  should be modeled, or configurable.  But not here: see packetReReceive */
	en->nodeaddr=us;

	eventnodeEnqueue(us->manet,en);
	return en;
}

/*
**  This is used for decapsulating packets...
*/
void packetReReceive(manetNode *us, packet *p)
{
	eventnode *en;
	if (!((p->dst==us->addr) || (p->dst==NODE_BROADCAST) || NODE_IS_MULTICAST(p->dst)))
	{
		fprintf(stderr,"packetReReceive: the requeued packet was not local.\n");
		abort();
	}
	en=packetEnqueue(us,p,0);
	en->type=EVENT_REPACKET;
#ifdef DEBUG_SELECT
	fprintf(stderr,"node %d: rereceived packet from %d to %d\n",us->addr & 0xFF, p->src & 0xFF, p->dst & 0xFF);
#endif
}

/* Called by support code to get next event
*/
static  eventnode *eventnodeDequeue(manet *m)
{
	eventnode *en;
	eventbucket *eb;

	if (m->eventlist)
	{
		en=m->eventlist->bucket;
		m->eventlist->bucket=m->eventlist->bucket->next;

		if (m->eventlist->bucket==NULL)
		{
			eb=m->eventlist;
			m->eventlist=m->eventlist->next;
			m->eventlist->prev=NULL;
			eventbucketFree(eb);
		}
		else
			m->eventlist->bucket->prev=NULL;
		assert(en!=NULL);
		return en;
	}

	return NULL;
}

/* returns the next event, but does not dequeue it
*/
static eventnode *eventnodeHead(manet *m)
{
	if (m->eventlist)
	{
		return m->eventlist->bucket;
	}
	return NULL;
}

eventnode *eventnodeNextTimer(manet *m)
{
	eventnode *en;
	eventbucket *eb;

	eb=m->eventlist;
	while(eb)
	{
		en=eb->bucket;
		while(en)
		{
			if (en->type==EVENT_TIMER)
			{
				return en;
			}
			en=en->next;
		}
		eb=eb->next;
	}

	return NULL;
}

eventnode *eventnodeNextPacket(manet *m)
{
	eventnode *en;
	eventbucket *eb;

	eb=m->eventlist;
	while(eb)
	{
		en=eb->bucket;
		while(en)
		{
			if (en->type==EVENT_PACKET)
			{
				return en;
			}
			en=en->next;
		}
		eb=eb->next;
	}

	return NULL;
}

/* walk the event list, looking for rereceived packets, and receive them directly
*/
void eventnodeWalkReReceive(manet *m)
{
	eventnode *en,*d;
	eventbucket *eb;
	packet *pkt;

	eb=m->eventlist;
	while(eb)
	{
		en=eb->bucket;
		while(en)
		{
			d=en;
			en=en->next;

			pkt=(packet*)d->data.pkt;
			if ((d->type==EVENT_REPACKET) && ((pkt->dst==m->nlist[0].addr) || (pkt->dst==NODE_BROADCAST)))
			{
				desGotPacket(&(m->nlist[0]),pkt);
				packetFree(pkt);
				eventnodeDelete(m,d);
			}
		}
		eb=eb->next;
	}
}

void eventnodeDelete(manet *m, eventnode *en)
{
	if (en->prev)
		en->prev->next=en->next;
	if (en->next)
		en->next->prev=en->prev;

	if (en->bucket->bucket==en)
		en->bucket->bucket=en->bucket->bucket->next;

	if (en->bucket->bucket==NULL)        /* is current bucket empty?  */
	{
		if (en->bucket->next)
			en->bucket->next->prev=en->bucket->prev;
		if (en->bucket->prev)
			en->bucket->prev->next=en->bucket->next;

		if (m->eventlist==en->bucket)
			m->eventlist=m->eventlist->next;
		eventbucketFree(en->bucket);
	}
	eventnodeFree(en);
}

/* Returns the number of packets in the eventlist
 */
int eventnodeNumPackets(manet *m)
{
	int c=0;
	eventnode *en;
	eventbucket *eb;

	eb=m->eventlist;
	while(eb)
	{
		en=eb->bucket;
		while(en)
		{
			switch(en->type)
			{
				case EVENT_TICK:
				case EVENT_TIMER:
				case EVENT_FDREAD:
				case EVENT_FDWRITE:
					break;
				case EVENT_PACKET:
				case EVENT_REPACKET:
					c++;
			}
			c++;	// bug? should this be deleted? -dkindred
			en=en->next;
		}
		eb=eb->next;
	}

	return c;
}


/* RF propagation model:
** returns true if node a can get a packet to node b
*/
static int nodeReachable(manetNode *a, manetNode *b)
{
	if (hypot(a->x-b->x, a->y-b->y )<a->aradius)   /* if the other node is within our antenna radius (need to get z coord in there...) */
		return 1;
	return 0;
}

static void manetUpdateLinklayer(manet *m)
{
	int src,dst;
	for(src=0;src<m->numnodes;src++)
		for(dst=0;dst<m->numnodes;dst++)
		{
			m->linklayergraph[src*m->numnodes+dst]=nodeReachable(&(m->nlist[src]),&(m->nlist[dst]));
		}
}

eventnode *timerSet(manetNode *us, eventCallback *cb, int delay, void *rawptr)
{
	eventnode *en;

	en=eventnodeMalloc();
	en->type=EVENT_TIMER;
	en->data.undef=rawptr;
	en->callback=cb;
	en->xmittime=us->manet->curtime;
	en->rectime=en->xmittime+delay;
	en->nodeaddr=us;

	eventnodeEnqueue(us->manet,en);
	return en;
}

eventnode *tickSet(manetNode *us, eventCallback *cb,void *rawptr)
{
	eventnode *en;

	en=eventnodeMalloc();
	en->type=EVENT_TICK;
	en->data.undef=rawptr;
	en->callback=cb;
	en->xmittime=us->manet->curtime;
	en->rectime=en->xmittime;
	en->nodeaddr=us;

	eventnodeEnqueue(us->manet,en);
	return en;
}

eventnode *eventFileRead(manetNode *us, int fd, eventCallback *cd, void *rawptr)
{
	eventnode *en;

/* Make sure there isn't already a read event?  */

	en=eventnodeMalloc();
	en->type=EVENT_FDREAD;
	en->fd=fd;
	en->data.undef=rawptr;
	en->callback=cd;
	en->xmittime=us->manet->curtime;
	en->rectime=en->xmittime;
	en->nodeaddr=us;

	eventnodeEnqueue(us->manet,en);
	return en;
}

eventnode *eventFileWrite(manetNode *us, int fd, eventCallback *cd, void *rawptr)
{
	eventnode *en;
	en=eventnodeMalloc();
	en->type=EVENT_FDWRITE;
	en->fd=fd;
	en->data.undef=rawptr;
	en->callback=cd;
	en->xmittime=us->manet->curtime;
	en->rectime=en->xmittime;
	en->nodeaddr=us;

	eventnodeEnqueue(us->manet,en);
	return en;
}

void eventFileClose(manetNode *us, int fd)
{
	eventnode *en, *preven=NULL, *d;

	en=us->manet->fdlist;
	
	while(en)
	{
		if (en->fd==fd)
		{
			if (preven)
				preven->next=en->next;
			else
				us->manet->fdlist=en->next;

			d=en;
			en=en->next;
			eventnodeFree(d);
		}
		else
		{
			preven=en;
			en=en->next;
		}
	}
}


manet *manetInit(Config *conf,long long int starttime)
{
	manet *m;
	int i,j;

	m=(manet*)malloc(sizeof(*m));

	m->conf=conf;
	m->packetProtection = NULL; // filled in later if needed

	m->numnodes=0;
	m->numnodes=configSearchInt(m->conf,"numnodes");
	if (m->numnodes<1)
		m->numnodes=1;
	m->nlist=(manetNode*)malloc(sizeof(*m->nlist)*m->numnodes);
	m->starttime=starttime;
	m->curtime=starttime;
	m->eventlist=NULL;
	m->ticklist=NULL;
	m->fdlist=NULL;
	m->linklayergraph=(int*)calloc(m->numnodes*m->numnodes,sizeof(int));
	m->rfState=NULL;
	m->hierarchygraph=NULL;
	m->mobility=NULL;
	memset(m->callbackList,0,sizeof(m->callbackList));

	for(i=0;i<m->numnodes;i++)
	{
		m->nlist[i].aradius=configSearchInt(m->conf,"antennaradius");
		m->nlist[i].x=0.0;
		m->nlist[i].y=0.0;
		m->nlist[i].z=0.0;
		m->nlist[i].index=i;
		m->nlist[i].manet=m;
		m->nlist[i].labelList=NULL;
		m->nlist[i].color=NULL;

		m->nlist[i].clusterhead=NULL;
		m->nlist[i].neighborlist=NULL;
		m->nlist[i].rootflag=0;
		m->nlist[i].rootgroupflag=0;
		m->nlist[i].level=0;

		m->nlist[i].addr=i;     /* may be replaced with an actual address later */
		m->nlist[i].netmask=0xFFFFFF00;
		m->nlist[i].bcastaddr=m->nlist[i].addr | (~m->nlist[i].netmask);

		m->nlist[i].status.packetList=(ApiPacketCount*)calloc(PACKET_MAX+1,sizeof(m->nlist[i].status.packetList[0]));
		m->nlist[i].status.numtypes=PACKET_MAX+1;
		for(j=0;j<m->nlist[i].status.numtypes;j++)
			m->nlist[i].status.packetList[j].type=j;
		m->nlist[i].status.period=5;
		m->nlist[i].status.level=0;
		m->nlist[i].status.rootflag=0;
	}

	return m;
}

void manetPacketHandlerSet(manetNode *us, int type, ManetPacketCallback *cb)
{
	if ((us->manet->callbackList[type]!=NULL) && (us->manet->callbackList[type]!=cb))
	{
		fprintf(stderr,"Redefined a packet type handler...  Very bad thing\n");
		abort();
	}
	us->manet->callbackList[type]=cb;
}

void firstStep(manet *m, int moduleinit)
{
	int i;

	manetUpdateLinklayer(m);

	for(i=0;i<m->numnodes;i++)
	{
		if (moduleinit)
		{
#ifdef MODULE_DATA
			dataInit(&m->nlist[i]);
#endif
#ifdef MODULE_ROUTING
			routeInit(&m->nlist[i]);
#endif
#ifdef MODULE_FLOOD
			floodInit(&m->nlist[i]);
#endif
#ifdef MODULE_TESTTRAFFIC
			testtrafficInit(&m->nlist[i]);
#endif
#ifdef MODULE_PACKETAPI
			packetApiInit(&m->nlist[i]);
#endif
		}
		nodeInit(&m->nlist[i]);
	}
}

void lastStep(manet *m)
{
	int i;

	for(i=0;i<m->numnodes;i++)
	{
		nodeFree(&m->nlist[i]);
	}
}

void desGotPacket(manetNode *us, packet *p)
{
	assert(p->type>=0);
	assert(p->type<=0xFF);
	ManetPacketCallback *cb=us->manet->callbackList[p->type];

	if (cb)
		(cb)(us,p);
	else
	{
		fprintf(stderr,"desGotPacket: unknown packet type 0x%x!\n",p->type);
//		abort();
	}
}


/* Check for any pending IO events.  
 * It will service all of them, and return when select says nothing is ready
 */
#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

void manetIOCheck(manet *m)
{
	fd_set readfds,writefds;
	int maxfd;
	int rc;
	eventnode *en;
	struct timeval timeout;

	do
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;
		timeout.tv_sec=0;
		timeout.tv_usec=0;

		for(en=m->fdlist;en;en=en->next)
		{
			switch(en->type)
			{
				case EVENT_FDREAD:
					FD_SET(en->fd,&readfds);
				break;
				case EVENT_FDWRITE:
					FD_SET(en->fd,&writefds);
				break;
			}
			GETMAXFD(maxfd,en->fd);
		}

		if (maxfd>-1)
		{
#ifdef DEBUG_SELECT
			fprintf(stderr,"Entering select\n");
#endif
			while (((rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout))<0)&& (errno==EINTR))
				;
		}
		else
			rc=-1;


		if (rc>0)
			/* This is hairy because a callback may schedule new events, and we don't want to look at the new ones, walking the current ones */
			{
				eventnode *curen = m->fdlist;
				eventnode *preven=NULL;
				eventnode *nexten=NULL;
				eventnode *calllist=NULL;
				int flag;

				while(curen)
				{
					nexten=curen->next;
					flag=0;
					switch(curen->type)
					{
						case EVENT_FDREAD:
							if (FD_ISSET(curen->fd,&readfds))
							{
#ifdef DEBUG_SELECT
								fprintf(stderr,"select: read on api %d\n",curen->fd);
#endif
								flag=1;
							}
						break;
						case EVENT_FDWRITE:
							if (FD_ISSET(curen->fd,&writefds))
							{
#ifdef DEBUG_SELECT
								fprintf(stderr,"select: write on api %d\n",curen->fd);
#endif
								flag=1;
							}
						break;
					}
					if (flag)
					{
						if (preven)                          /* remove this en from the event list */
							preven->next=curen->next;
						else
							m->fdlist=curen->next;

						curen->next=calllist;                  /* and add it to a calllist we're accumulating */
						calllist=curen;

					}
					else
						preven=curen;

					curen=nexten;
				}
				while(calllist)                    /* if we have any events in the calllist, call them  */
				{
					curen=calllist;
					calllist=calllist->next;
					curen->callback(curen->nodeaddr, curen->data.undef);
					eventnodeFree(curen);
				}
			}
	} while (rc>0);
}

/* This executes one timestamp of the DES, then returns.
*/
void takeStep(manet *m)
{
	eventnode *en,*enlist;

	manetUpdateLinklayer(m);
	do
	{
		/* Check FD IO here.
		 *
		 * Do we want to run in simulator mode in real time, or as fast as possible?
		 * (need both...)
		*/

		en=eventnodeHead(m);
		if ((en==NULL) || (en->rectime!=m->curtime))
			break;
		en=eventnodeDequeue(m);

#ifdef DEBUG_EVENTS
		fprintf(stderr,"next event, type= %d time= %d ",en->type,en->rectime);
#endif
		if (en)
		{
			switch(en->type)
			{
				case EVENT_PACKET:
				{
					packet *pkt=(packet*)en->data.pkt;
					statusCount(en->nodeaddr, PACKET_RECEIVE, pkt);
				}
				case EVENT_REPACKET:
				{
					packet *pkt=(packet*)en->data.pkt;
#ifdef DEBUG_EVENTS
					fprintf(stderr,"  packet type= %x src= %d dst= %d node= %d\n",pkt->type,pkt->src, pkt->dst, en->nodeaddr->addr);
#endif
					assert((en->xmittime - en->rectime) <= 1);
					assert(en->rectime==m->curtime);
					desGotPacket(en->nodeaddr,pkt);
					packetFree(pkt);
				}
				break;
				case EVENT_TIMER:
				{
#ifdef DEBUG_EVENTS
					fprintf(stderr,"  timer node= %d\n",en->nodeaddr->addr);
#endif
					(en->callback)(en->nodeaddr, en->data.undef);
				}
				break;
				default:
					fprintf(stderr,"invalid event type %d\n",en->type);
					exit(1);
				break;
			}
			eventnodeFree(en);
		}
	} while (1);

	if (en)
	{
		assert(en->rectime > m->curtime);
		m->curtime=en->rectime;
	}

//	eventnodeWalkReReceive(m);

	enlist=m->ticklist;           /* execute all the pending tick events */
	m->ticklist=NULL;

	while(enlist)
	{
		en=enlist;
		enlist=enlist->next;

		(en->callback)(en->nodeaddr, en->data.undef);
		eventnodeFree(en);
	}

	m->step++;
}


/* called to update packet counters.
 * if the receive arg is true, then we are receiving a packet.  
 * otherwise, we are transmitting
 */
void statusCount(manetNode *us, int origflag, packet *p)
{
	switch(origflag)
	{
		case PACKET_RECEIVE:
			if (p->dst==us->addr)
			{
				us->status.packetList[p->type].unicastRecCount++;
				us->status.packetList[p->type].unicastRecByte+=p->len;
			}
			else
			{
				us->status.packetList[p->type].bcastRecCount++;
				us->status.packetList[p->type].bcastRecByte+=p->len;
			}
		break;
		case PACKET_ORIGIN:
			if ((p->dst==NODE_BROADCAST) || (p->dst==us->bcastaddr))
			{
				us->status.packetList[p->type].origBcastXmitCount++;
				us->status.packetList[p->type].origBcastXmitByte+=p->len;
			}
			else
			{
				us->status.packetList[p->type].origUnicastXmitCount++;
				us->status.packetList[p->type].origUnicastXmitByte+=p->len;
			}
		break;
		case PACKET_REPEAT:
			if ((p->dst==NODE_BROADCAST) || (p->dst==us->bcastaddr))
			{
				us->status.packetList[p->type].repBcastXmitCount++;
				us->status.packetList[p->type].repBcastXmitByte+=p->len;
			}
			else
			{
				us->status.packetList[p->type].repUnicastXmitCount++;
				us->status.packetList[p->type].repUnicastXmitByte+=p->len;
			}
	}
}
