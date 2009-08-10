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
#include <assert.h>
#include <string.h>

#ifdef GRAPHICS
#include "graphics.h"
#endif

#include "rng.h"
#include "hello.h"
#include "node.h"
#include "marshal.h"

/* Time we wait before makeing a selection after a new neighbor arrives
*/
#define HELLO_SELECTTIME 3000
/* Time we must not be desired by any node before we cease to be a coordinator
*/
#define HELLO_UNDESIREDTIME 6000

typedef struct helloState
{
	helloNeighbor *neighborCallback;
	helloCH *chCallback;
	helloHello *helloCallback;
	int initflag;

	int floatingroot;
	int helloseqnum;
	destime selecttime;
	destime lastdesiredtime;
	neighbor *coordinator,*desiredCoordinator;
	int coordinatorFlag;

	unsigned char *payload;
	int payloadLen;
} helloState;

typedef struct
{
        int numNeighbors;                /* number of neighbors */
        int numSymNeighbors;             /* number of symmetric neighbors */
        int sequencenum;
        ManetAddr coordinator,desiredCoordinator;
	int coordinatorFlag;
        ManetAddr *addresslist;

	unsigned char *payload;
	int payloadLen;
} packetHello;

#ifdef USE_RNG
static RNG *rnd=NULL;
#define RAND_U01() (rnd->rand_u01())
#else
#define RAND_U01() (drand48())
#endif


/* choose a coordinator and desired coordinator.  
**  called by helloSend if selecttime is in the past.
*/
static void selectCoordinator(manetNode *us)
{
	int numSymNeighbors=0;
	neighbor *n;
	neighbor *max=NULL;
	neighbor *maxCoord=NULL;
	int neighborDesiresUs=0;

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if ((n->level==0) && (n->flags & NEIGHBOR_HEARS))
		{
#ifdef DEBUG_HELLO
			fprintf(stderr,"node %d: neighbor %d degree %d coordinatorflag %d  desires %d\n",us->addr & 0xFF, n->addr & 0xFF, n->helloDegree,n->helloCoordinatorFlag,n->helloDesiredCoordinator);
#endif
			numSymNeighbors++;

			if (n->helloDesiredCoordinator==us->addr)      /* someone wants us...  */
				neighborDesiresUs=1;

			if ((max==NULL) ||
				((max!=NULL) && (n->helloDegree > max->helloDegree)) ||
				((max!=NULL) && (n->helloDegree == max->helloDegree) && (n->addr < max->addr)) 
				)
					max=n;

			if (n->helloCoordinatorFlag)
				if ((maxCoord==NULL) ||
					((maxCoord!=NULL) && (n->helloDegree > maxCoord->helloDegree)) ||
					((maxCoord!=NULL) && (n->helloDegree == maxCoord->helloDegree) && (n->addr < maxCoord->addr))
					)
						maxCoord=n;
		}
	}
#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d: us degree %d coordinatorflag %d\n",us->addr & 0xFF, numSymNeighbors,us->hello->coordinatorFlag);
#endif


	/* Set and clear the coordinator flag:  
	*/
	if (neighborDesiresUs)
	{
		us->hello->lastdesiredtime=us->manet->curtime;
		if (maxCoord==NULL)      /* if someone wants us, and there are no other coordinators, then we are willing to become one  */
		{
#ifdef DEBUG_HELLO
			fprintf(stderr,"node %d: I'm a coordinator!\n",us->addr & 0xFF);
//			us->color=5;
#endif
			if ((us->hello->coordinatorFlag==0) && (us->hello->chCallback))
				(us->hello->chCallback)(us,1);
			us->hello->initflag=0;
			us->hello->coordinatorFlag=1;
			us->hello->coordinator=NULL;
		}
	}

	if ((!neighborDesiresUs) && (us->hello->lastdesiredtime < us->manet->curtime - HELLO_UNDESIREDTIME))
	{
#ifdef DEBUG_HELLO
		fprintf(stderr,"node %d: I'm NOT coordinator! (sigh)\n",us->addr & 0xFF);
		us->color=0;
#endif
		if ((us->hello->coordinatorFlag==1) && (us->hello->chCallback))
			(us->hello->chCallback)(us,0);
		us->hello->initflag=0;
		us->hello->coordinatorFlag=0;
	}

	if (us->hello->initflag)
	{
		if (us->hello->chCallback)
			(us->hello->chCallback)(us,us->hello->coordinatorFlag);
		us->hello->initflag=0;
	}

	us->hello->initflag=0;

	/* Determine the desired coordinator  */
	/* max now points to the neighbor with the greatest degree.   Make sure we don't beat it, and thats our desired coordinator */

	if (((max) && (max->helloDegree > numSymNeighbors)) ||
		((max) && (max->helloDegree == numSymNeighbors) && (max->addr < us->addr)))
		us->hello->desiredCoordinator=max;
	else
		us->hello->desiredCoordinator=NULL;

#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d:  desired %d  %s\n",us->addr & 0xFF,(us->hello->desiredCoordinator?(us->hello->desiredCoordinator->addr):NODE_BROADCAST) & 0xFF,((us->hello->selecttime==0) || (us->hello->selecttime>us->manet->curtime))?"continuing":"not yet");
#endif

	/* We only proceed to make a real selection if the select time as been set, and has subsequently elapsed */

	if ((us->hello->selecttime==0) || (us->hello->selecttime>us->manet->curtime))
		return;

	if (us->hello->coordinatorFlag)    /* no selection if we are a coordinator.  Leave that to whatever is calling us */
		return;


	/* maxCoord now points to the neighbor greatest degree, of the neighbors with their Coord flags set */

	us->hello->coordinator=maxCoord;

#if 0
	if ((us->hello->selecttime>0) && (us->hello->selecttime<us->manet->curtime) && (us->hello->coordinator==NULL))
	{
			/* If we are orphaned...   IE: none of our neighbors have their Coordinator flag set  */
		us->color=6;
	}
#endif

	if (us->hello->coordinator!=NULL)
	{
		us->hello->selecttime=0;
		us->color=0;
	}

#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d:  Selected %d desired %d\n",us->addr & 0xFF,(us->hello->coordinator?(us->hello->coordinator->addr):NODE_BROADCAST) & 0xFF,(us->hello->desiredCoordinator?(us->hello->desiredCoordinator->addr):NODE_BROADCAST) & 0xFF);
#endif
}

static packet *helloMarshal(manetNode *us, packetHello *hp)
{
	packet *p;
	unsigned char *payload;
	int i;

	p=packetMalloc(us,2+2+2+4+4+4+1+(hp->numNeighbors*4)+hp->payloadLen);
        p->type=PACKET_HELLO_HELLO;
        p->dst=NODE_BROADCAST;
        p->ttl=0;

	payload=(unsigned char *)p->data;

	MARSHALSHORT(payload,hp->payloadLen);
	MARSHALSHORT(payload,hp->numNeighbors);

	MARSHALSHORT(payload,hp->numSymNeighbors);
	MARSHALLONG(payload,hp->sequencenum);
	MARSHALLONG(payload,hp->coordinator);
	MARSHALLONG(payload,hp->desiredCoordinator);
	MARSHALBYTE(payload,hp->coordinatorFlag);
	for(i=0;i<hp->numNeighbors;i++)
		MARSHALLONG(payload,hp->addresslist[i]);
	memcpy(payload,hp->payload,hp->payloadLen);
	payload+=hp->payloadLen;

	return p;
}

static packetHello *helloUnmarshal(packet *p)
{
	unsigned char *payload;
	packetHello *hp;
	int i;
	int payloadLen,numNeighbors;

	hp=(packetHello*)malloc(sizeof(*hp));
	payload=(unsigned char *)p->data;

	UNMARSHALSHORT(payload,payloadLen);
	UNMARSHALSHORT(payload,numNeighbors);

	hp=(packetHello*)malloc(sizeof(*hp) + (sizeof(hp->addresslist[0])*numNeighbors) + payloadLen);
	hp->addresslist=(ManetAddr*)(hp+1);
	hp->payload=(payloadLen==0)?NULL:(unsigned char*)(hp->addresslist+numNeighbors);

	hp->payloadLen=payloadLen;
	hp->numNeighbors=numNeighbors;

	UNMARSHALSHORT(payload,hp->numSymNeighbors);
	UNMARSHALLONG(payload,hp->sequencenum);
	UNMARSHALLONG(payload,hp->coordinator);
	UNMARSHALLONG(payload,hp->desiredCoordinator);
	UNMARSHALBYTE(payload,hp->coordinatorFlag);
	for(i=0;i<hp->numNeighbors;i++)
		UNMARSHALLONG(payload,hp->addresslist[i]);
	memcpy(hp->payload,payload,hp->payloadLen);
	payload+=hp->payloadLen;

	return hp;

}

/* Send a really boring hello packet
*/

static void helloSend(manetNode *us, void *data)
{
	packet *p;
        packetHello hello,*hp=&hello;
	ManetAddr addresslist[200];

	hp->addresslist=addresslist;
	hp->payload=us->hello->payload;
	hp->payloadLen=us->hello->payloadLen;

	neighbor *n;
        unsigned int numNeighbors=0,numSymNeighbors=0;
	int noise;

	        /* first, reschedule...  */
        noise=(int)(RAND_U01()*100.0)-50;
        timerSet(us,helloSend,TIME_HELLO+noise,NULL);

	selectCoordinator(us);

        hp->sequencenum=us->hello->helloseqnum++;
	hp->coordinator=us->hello->coordinator?(us->hello->coordinator->addr):NODE_BROADCAST;
	hp->desiredCoordinator=us->hello->desiredCoordinator?(us->hello->desiredCoordinator->addr):NODE_BROADCAST;
	hp->coordinatorFlag=us->hello->coordinatorFlag;

	n=us->neighborlist;
#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d:  Send Hello  ",us->addr & 0xFF);
#endif
	while(n)
	{
		if ((n->level==0) && (n->flags & NEIGHBOR_HEARD) && (numNeighbors<(sizeof(addresslist)/sizeof(hp->addresslist[0]))))
		{
			hp->addresslist[numNeighbors++]=n->addr;
#ifdef DEBUG_HELLO
			fprintf(stderr,"%d  ",n->addr & 0xFF);
#endif
			if (n->flags & NEIGHBOR_HEARS)
				numSymNeighbors++;
		}
		n=n->next;
	}
#ifdef DEBUG_HELLO
	fprintf(stderr,"\n");
#endif

	hp->numNeighbors=numNeighbors;
	hp->numSymNeighbors=numSymNeighbors;

	p=helloMarshal(us, hp);
        p->dst=NODE_BROADCAST;
        p->ttl=0;

	packetSend(us,p, PACKET_ORIGIN);
	packetFree(p);
}

/* Called when a really boring hello packet arrives
**
**  must be called by des.cpp.  (to make sure that our packet types are actually delivered to us)
*/
void helloPacket(manetNode *us, packet *p)
{
	neighbor *n;
	packetHello *hp=NULL;
	int i;

	if (p->src==us->addr)               /* don't listen to strange nodes...  */
		return;

	hp=helloUnmarshal(p);

	if (us->hello->floatingroot)
	{
		us->rootflag=0;
		us->hello->floatingroot=0;
	}

	n=neighborSearch(us,p->src,0);
	if (n==NULL)                   /* we have an entirely new neighbor!  */
		n=neighborInsert(us,p->src,0);

	n->lastheard=us->manet->curtime;
	n->hopcount=p->hopcount;

	n->flags|=NEIGHBOR_HEARD;    /* we heard them...  */

#if 0
		/* Do we want to make a new selection, if we have a new coordinator?  */
	if (n->helloCoordinatorFlag!=hp->coordinatorFlag)
		us->hello->selecttime=us->manet->curtime+HELLO_SELECTTIME;
#endif

		/* if our coordinator just dropped its coordinator flag, we need to reselect  */
	if ((us->hello->coordinator==n) && (hp->coordinatorFlag==0) && (n->helloCoordinatorFlag))
	{
		us->hello->selecttime=us->manet->curtime;
	}

	n->helloCoordinator=hp->coordinator;
#if 0
	if (n->helloCoordinator==us->addr)
		n->flags|=NEIGHBOR_CHILD;
	else
		n->flags&=~NEIGHBOR_CHILD;
#endif

	n->helloDesiredCoordinator=hp->desiredCoordinator;
	n->helloDegree=hp->numSymNeighbors;
	n->helloCoordinatorFlag=hp->coordinatorFlag;


#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d: Rec Hello from %d  hears ",us->addr & 0xFF,p->src & 0xFF);
#endif
	for(i=0;i<hp->numNeighbors;i++)
	{
#ifdef DEBUG_HELLO
		fprintf(stderr,"%d ",hp->addresslist[i] & 0xFF);
#endif
		if (hp->addresslist[i]==us->addr)
		{
#ifdef DEBUG_HELLO
			fprintf(stderr,"(us) ");
#endif
			if (!(n->flags & NEIGHBOR_HEARS))
			{
#ifdef DEBUG_HELLO
//				fprintf(stderr,"node %d: just got a new symmetric link.\n",us->addr & 0xFF);    /* got a new symmetric link... call the OLSR stuff */
#endif
				n->flags|=NEIGHBOR_HEARS;   /* they listed us, so they hear us  */
				n->firstheard=us->manet->curtime;

				/* Do we want to set selecttime here?  if we have no Coordinator, its already set.  If we have one already, we don't really need a new one  */

				if (us->hello->neighborCallback)
					(us->hello->neighborCallback)(us,n,1);
			}
		}
	}
#ifdef DEBUG_HELLO
		fprintf(stderr,"\n");
#endif

	selectCoordinator(us);

	if (us->hello->helloCallback)
		(us->hello->helloCallback)(us,us->hello->coordinator, n, hp->payload,hp->payloadLen);

	free(hp);
}

/* Hello timeout timer
** called every HELLO interval, to remove HEARD and HEARS flags from entries from the neighbor list
** which we havn't heard a HELLO from recently enough
*/
static void helloTimeout(manetNode *us,void *data)
{
        neighbor *n;
	neighbor *d;
	int numneighbors=0;

        timerSet(us,helloTimeout,TIME_HELLO_TIMEOUT,NULL);

        n=us->neighborlist;
        while(n)
        {
                if (n->level==0)
		{
			numneighbors++;
			if ((us->manet->curtime - n->lastheard) > TIME_HELLO_TIMEOUT)
			{
				if (n->flags & NEIGHBOR_HEARS)
				{
#ifdef DEBUG_HELLO
					fprintf(stderr,"node %d: node %d timed out\n",us->addr & 0xFF,n->addr & 0xFF);   /* lost an edge, call the OLSR stuff */
#endif
					us->hello->selecttime=us->manet->curtime+HELLO_SELECTTIME;     
				}

				d=n;
				n=n->next;

				if (us->hello->neighborCallback)
					(us->hello->neighborCallback)(us,d,0);

				neighborDelete(us,d);

				if (us->hello->coordinator==d)          /* lost coordinator, reselect immediately.   */
				{
#ifdef DEBUG_HELLO
					fprintf(stderr,"node %d: lost coordinator\n",us->addr & 0xFF);
#endif
					us->hello->selecttime=us->manet->curtime;
					us->hello->coordinator=NULL;
					selectCoordinator(us);
				}
			}
			else
				n=n->next;
		}
		else
			n=n->next;
        }

		/* if we have no neighbors, transition into FLOATINGROOT, set root flag  */
	if (numneighbors==0)
	{
		us->hello->floatingroot=1;
#if 0
		us->rootflag=1;
#endif
	}
}

/* So the evil plot is:

This is a sub-clustering algorithm.
If a clustering algorithm wants to use it, they call helloInit(), with some callbacks.
The init call will then schedule its own callbacks to transmit hello packets.  The packet reception
is already there, since this looks more like a clustering algorithm to the des code.

This also does the greatest degree thing with the hello packets.  Thus the clusterheadState callback,
which will be called if this module decides this node should be a first level CH, or not be a first
level CH.

The callbacks:
neighborDepart
clusterheadState
*/
void helloInit(manetNode *us, helloNeighbor *neighborCallback, helloCH *chCallback, helloHello *helloCallback)
{
        us->neighborlist=NULL;

#ifdef USE_RNG
	if (rnd==NULL)
		rnd= new RNG(42);
#endif

	us->hello=(helloState*)malloc(sizeof(helloState));
	us->hello->helloseqnum=23;
	us->hello->neighborCallback=neighborCallback;
	us->hello->chCallback=chCallback;
	us->hello->helloCallback=helloCallback;
	us->hello->initflag=1;
	us->hello->selecttime=us->manet->curtime+HELLO_SELECTTIME;
	us->hello->lastdesiredtime=us->manet->curtime-HELLO_SELECTTIME;
	us->hello->coordinatorFlag=0;
	us->hello->coordinator=NULL;
	us->hello->desiredCoordinator=NULL;
	us->hello->floatingroot=1;
	us->hello->payload=NULL;
	us->hello->payloadLen=0;

	manetPacketHandlerSet(us, PACKET_HELLO_HELLO, helloPacket);

	helloSend(us,NULL);
	helloTimeout(us,NULL);
}

void helloFree(manetNode *us)
{
}

void helloPayloadSet(manetNode *us, unsigned char *payload, int payloadLen)
{
	us->hello->payload=payload;
	us->hello->payloadLen=payloadLen;
	if (us->hello->payloadLen==0)
		us->hello->payload=NULL;
}

