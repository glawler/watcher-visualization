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

#include "node.h"
#include "data.h"
#include "routing.h"
#include "flood.h"
#include "hashtable.h"
#include "marshal.h"

#ifdef MODULE_DATA
                                
/*
 * This implements DATA packets, optionally calling the routing module as needed.
 *
 * This module will take a packet from another module, wrap it, and then send it using
 *  either the flood module, the routing module, or bare, to another node.  On that other node
 *  the packet will be decapsulated, and passed to the local instance of whatever module sent it,
 *  and an ack packet will then be sent back to the original node.
 *
 * ACK packets will be sent to the calling module by taking the packet type of the packet passed
 * to dataSend, and adding 1.  The payload of that ACK packet will be a packetData struct.
 * a packetData struct contains an id field.  When a packet is transmitted, its ID is returned 
 * by an argument in dataSend().  So, the caller can figure out /which/ packet is acked by looking
 * at the id field.
 * 
 * This functionality is bordering on the simulated and live cases
 * BUT we want the ACK packets to be present in the simulation, thus is must be in simulation space
 *
 * How to add multicast reliability:
 *  a DATA packet will need a list of destination addresses
 *  if a destination address is multi-hop away (looking in the neighbor list) then it must be unicast.
 *    That unicast packet's destination field contains that destination.
 *  if a destination is local, then a single broadcast packet to all the local destiations is sent.
 *    That broadcast packet's destination field contains the broadcast addr, and TTL=1
 *    IE: the routing arguments are ignored for these broadcasts.
 *    on retry, send individual unicasts?
 *  each destination sends its own ack packet back.
 *  when all ack packets arrive, then a single ack packet is sent back to the original caller.
 *    The ack packet needs a flag array for each destination.
 */

/* How long to keep track of a payload
 */
#define DATA_TIMEOUTENTRY (60*1000)

/* How long to attempt to retransmit a packet 
 */
#define DATA_TIMEOUT (10*1000)

/* How long to wait between retransmits
 */
#define DATA_REXMIT (1000)


/* data module's list of packets which are in flight.  
 *  When a packet is transmitted, it is put on the inflight list.  That list is then checked
 *  on the retransmit period, to see if any packets need to be retransmitted.  When a packet is
 *  acked, its entry on the list is marked acked, but not deleted until the entire timeout period.
 *  thus, duplicate ACKs can be taken care of (for a while...).
 * 
 * pointed to in the data module's private data...
 */

/* The status of an inflight packet.
 */
typedef enum DataStatus
{
	DATA_STATUS_UNACKED=0,
	DATA_STATUS_ACKED
} DataStatus;

typedef struct InFlight
{ 
//	struct InFlight *next;

	packet *p;			/* pointer to dup of original packet which was sent.  */
	PacketData *pd;

	int xmitnum;
	DataStatus finalStatus;
	destime rexmittimeout;
	destime acktimeout;
	destime entrytimeout;
	manetNode *us;
} InFlight;

typedef struct InFlightKey
{
	ManetAddr src;
	int id;
};

/* data module's private data in the node structure
 */
typedef struct dataState
{
	hashtable *inflight;		/* list of data packets which have been sent, but not ACKed   */
	unsigned int nextid;		/* next data packet ID  */

	int dataTimeout;	/* How long we will try to retransmit a packet */
	int dataTimeoutEntry;	/* How long we will remember a specific payload */
	int dataRexmit;		/* How long we will wait beteen rexmit attempts */
	int dataFailover;	/* If true, then when a msg is not acked, try again using flood routing  */
} dataState;


static void dataTimeout(manetNode *us, void *data);
static packet *packetDataMarshal(const packet *p, PacketData *pd);
static packet *dataPacketAckMarshal(manetNode *us, InFlight *inf);
static void dataData(manetNode *us, packet *p);
static void dataAck(manetNode *us, packet *p);

static packet *packetDataAckMarshal(manetNode *us, PacketDataAck *pda);

/* called at module startup, by the manet init function
 * mallocs module-private data, reads config vars, schedules timer callback for doing rexmits
 */
void dataInit(manetNode *us)
{
	int hashtablesize;

	us->data=(dataState*)malloc(sizeof(*us->data));

	us->data->nextid=us->index ^ getpid() ^ (getppid() << 16) ^ us->manet->curtime;
	hashtablesize=configSetInt(us->manet->conf,"data_inflighthashsize",16381);
	us->data->inflight=hashtableinit(hashtablesize,hashtablehash);
	us->data->dataTimeoutEntry=configSetInt(us->manet->conf,"data_timeoutentry",DATA_TIMEOUTENTRY);
	us->data->dataTimeout=configSetInt(us->manet->conf,"data_timeout",DATA_TIMEOUT);
	us->data->dataRexmit=configSetInt(us->manet->conf,"data_timerexmit",DATA_REXMIT);
	us->data->dataFailover=configSetInt(us->manet->conf,"data_routing_failover",1);

	dataTimeout(us,NULL);

	manetPacketHandlerSet(us, PACKET_DATA_DATA, dataData);
	manetPacketHandlerSet(us, PACKET_DATA_ACK, dataAck);

	/* we need a data structure for keeping track of packets in flight.  And how to ACK them  */

	/* schedule timer to check for timeouts */
}

/* Inserts a record for packet p into the pending ACK list
*/
static InFlight *dataPacketInsert(manetNode *us, packet *p, PacketData *pd)
{
	InFlight *inf;
	InFlightKey infkey;

	inf=(InFlight*)malloc(sizeof(*inf));

	inf->finalStatus=DATA_STATUS_UNACKED;
	inf->p=packetDup(p);
	inf->pd=pd;

	inf->entrytimeout= us->manet->curtime+us->data->dataTimeoutEntry;
	inf->rexmittimeout= us->manet->curtime+us->data->dataRexmit;
	inf->acktimeout= us->manet->curtime+us->data->dataTimeout;
	inf->xmitnum=0;
	inf->us=us;

	infkey.src=inf->p->src;
	infkey.id=inf->pd->id;
	hashtableinsert(us->data->inflight,(char*)&infkey,sizeof(infkey),inf);

#ifdef DEBUG_DATA
	{
		int count=0;
#if 0
		InFlight *p;
		for(p=us->data->inflight;p!=NULL;p=p->next)
			count++;
#endif
		fprintf(stderr,"node %d: dataPacketInsert: inserting record for packet with src %d id %u list length= %d\n",us->addr & 0xFF,inf->p->src & 0xFF,inf->pd->id,count);
	}
#endif
	return inf;
}

/* Search for a record in the pending ACK list
 * The key is the ManetAddr of the node which originated the data packet in question,
 * and the id field.  IE when we receive an ACK, we are looking for our own address.
 */
static InFlight *dataPacketSearch(manetNode *us, ManetAddr src, unsigned int id)
{
	InFlightKey infkey;

	infkey.src=src;
	infkey.id=id;

	return (InFlight*)hashtablesearch(us->data->inflight,(char*)&infkey,sizeof(infkey));
}

static void dataPacketRemove(manetNode *us, InFlight *d)
{
	InFlightKey infkey;

	infkey.src=d->p->src;
	infkey.id=d->pd->id;

#ifdef DEBUG_DATA
	InFlight *inf;
	inf= (InFlight*)hashtablesearch(us->data->inflight,(char*)&infkey,sizeof(infkey));

	if (inf!=d)
	{
		fprintf(stderr,"dataPacketRemove: inf= %p d= %p\n",inf,d);
	}
	assert(inf==d);
#endif

	hashtabledelete(us->data->inflight,(char*)&infkey,sizeof(infkey));

	packetFree(d->p);     /* matches packetDup() in dataPacketInsert()   */
	free(d->pd);
	free(d);
}

/* This will create and send a ACK message for the caller, reporting on which
 * destinations we successfully delivered to.
 * 
 */
static void sendPacketAck(manetNode *us, InFlight *inf)
{
	packet *ack;

	ack=dataPacketAckMarshal(us,inf);
	packetReReceive(us,ack);
	packetFree(ack);
}

/* This exists only to avoid duplicating the switch statement for the routing type everywhere
*/
static void intDataSend(manetNode *us, packet *p,DataRoute routetype, int origflag)
{

	/* walk list of destination addresses.  
		for addresses which have not acked yet:
			if local address, add destination to list for a link-local broadcast
			if remote address, send a unicast packet
	*/
#ifdef DEBUG_DATA
	fprintf(stderr,"node %d: intDataSend: sending packet to %d  route= 0x%x len= %d\n",us->addr & 0xFF,p->dst & 0xFF,routetype,p->len);
#endif
	switch(routetype & DATA_ROUTE_MASK)
	{
		case DATA_ROUTE_AMBIENT:
			packetSend(us,p,origflag);
		break;
		case DATA_ROUTE_ROUTING:
#ifdef MODULE_ROUTING
			routeSend(us,p); 
#else
			fprintf(stderr,"node %d: routing module not compiled in\n",us->addr & 0xFF);
			abort();
#endif
		break;
		case DATA_ROUTE_FLOOD:
#ifdef MODULE_FLOOD
			floodSend(us,p);
#else
			fprintf(stderr,"node %d: flood module not compiled in\n",us->addr & 0xFF);
			abort();
#endif
		break;
		case DATA_ROUTE_HIERARCHY:
		default:
			fprintf(stderr,"dataPacket: unknown routing type!\n");
	}
}

/* This will send all the pending data packets for all the destinations listed
 * in the PacketData pointed to by inf
 */
static void intDataSendData(manetNode *us, InFlight *inf, int origflag)
{
	int i;
	PacketData multicast;
	PacketData unicast;
	ManetAddr multicastDestList[DATA_MAX_DEST];
	neighbor *n;
	packet *p;

	multicast.destinationCount=0;
	multicast.id=inf->pd->id;
	multicast.origtype=inf->p->type;
	multicast.destinationCount=0;
	multicast.routetype=inf->pd->routetype;
	multicast.acktype=inf->pd->acktype;
	multicast.xmitnum=inf->xmitnum;
	multicast.destinationList=multicastDestList;
	multicast.destinationAck=NULL;

	unicast.destinationCount=0;
	unicast.id=inf->pd->id;
	unicast.origtype=inf->p->type;
	unicast.destinationCount=0;
	unicast.routetype=inf->pd->routetype;
	unicast.acktype=inf->pd->acktype;
	unicast.xmitnum=inf->xmitnum;
	unicast.destinationList=NULL;
	unicast.destinationAck=NULL;


	for(i=0;i<inf->pd->destinationCount;i++)
	{
		if (inf->pd->destinationAck[i]!=DATA_NAK)   /* do not rexmit to destinations who have already ACKed  */
			continue;
		
#ifdef DEBUG_DATA
		fprintf(stderr, "node %d: data: checking %d\n",us->addr & 0xfF, inf->pd->destinationList[i] & 0xFF);
#endif

		n=neighborSearch(us,inf->pd->destinationList[i],0);   /* lookup destination in neighbor list.  Is it 1 hop away?  */
		if ((n) && (n->hopcount==1))
		{
			/* This destination is local, add to multicast packetdata  */
			multicast.destinationList[multicast.destinationCount]=inf->pd->destinationList[i];
			multicast.destinationCount++;

			if (inf->pd->acktype==DATA_ACK_NONE)     /* if we are not looking for ACKs, act as if the ACK has already been received.  */
				inf->pd->destinationAck[i]=DATA_ACK;

			if (multicast.destinationCount>=DATA_MAX_DEST)
			{
#ifdef DEBUG_DATA
				int j;
				fprintf(stderr,"node %d: data: sending broadcast data to ",us->addr & 0xFF);
				for(j=0;j<multicast.destinationCount;j++)
					fprintf(stderr,"%d ",multicast.destinationList[j] & 0xFF);
				fprintf(stderr," partial\n");
#endif

				p=packetDataMarshal(inf->p,&multicast);
				p->dst=NODE_BROADCAST;
				intDataSend(us,p,inf->pd->routetype, origflag);
				packetFree(p);
				multicast.destinationCount=0;
			}
		}
		else
		{
			/* make unicast PacketData */

#ifdef DEBUG_DATA
			fprintf(stderr,"node %d: data: sending unicast data to node %d\n",us->addr & 0xFF, inf->pd->destinationList[i] & 0xFF);
#endif
			p=packetDataMarshal(inf->p,&unicast);
			p->dst=inf->pd->destinationList[i];
			intDataSend(us,p,inf->pd->routetype, origflag);
			packetFree(p);
		}
	}

#ifdef DEBUG_DATA
	fprintf(stderr,"node %d: data: check partial bcast...  destinationCount= %d  total= %d\n", us->addr & 0xFF,multicast.destinationCount, inf->pd->destinationCount );
#endif

	if (multicast.destinationCount>0)    /* Do we have any multicast packets to send?  */
	{
		if (multicast.destinationCount==1)    /* if only one addr, just send unicast.  */
		{
#ifdef DEBUG_DATA
			fprintf(stderr,"node %d: data: sending unicast data to %d flush\n",us->addr & 0xFF, inf->pd->destinationList[i] & 0xFF);
#endif
			p=packetDataMarshal(inf->p,&unicast);
			p->dst=multicast.destinationList[0];
			intDataSend(us,p,inf->pd->routetype, origflag);
			packetFree(p);
		}
		else
		{
#ifdef DEBUG_DATA
			fprintf(stderr,"node %d: data: sending broadcast data to ",us->addr & 0xFF);
			for(i=0;i<multicast.destinationCount;i++)
				fprintf(stderr,"%d ",multicast.destinationList[i] & 0xFF);
			fprintf(stderr," flush\n");
#endif

			p=packetDataMarshal(inf->p,&multicast);
			p->dst=NODE_BROADCAST;
			intDataSend(us,p,inf->pd->routetype, origflag);
			packetFree(p);
		}
	}
	inf->xmitnum++;
}

/* Called when we get an incoming data packet
 *
 * if p->dst == BROADCAST
 *	if destinationCount==0
 *		Its for us   (pure broadcast)
 *	if destinationCount!=0  and we are on the list
 *		Its for us
 * if p->dst == us
 *		Its for us
 *		assert(destinationCount==0)
 *
 * if its for us
 * 	if finalStatus==DATA_STATUS_UNACKED
 *		decapsulate and rereceive
 *	send an ACK
 *	mark it finalStatus=DATA_STATUS_ACKED
 *
 */
static void dataData(manetNode *us, packet *p)
{
	InFlight *inf;
	PacketData *pd;
	int forus=0;	/* flag, is this packet for us  */
	int i;
	packet *ack;
	packet *cpy;
	int dontfreepd=0;

	pd=packetDataUnmarshal(p);

        if (pd==NULL)
        {
		fprintf(stderr,"node %d: %s: unmarshal failed, len= %d\n",
			us->addr & 0xFF, __func__, p->len);
		return;
        }

	if (p->dst==NODE_BROADCAST)
	{
		if (pd->destinationCount==0)
			forus=1;
		else
		{
			for(i=0;i<pd->destinationCount;i++)
				if (pd->destinationList[i]==us->addr)
					forus=1;
		}
	}
	if (p->dst==us->addr)
	{
		forus=1;
                /* XXX can a malicious packet force us to abort here? */
                /*     should check for other such asserts */
		assert(pd->destinationCount==0);
	}

	if (!forus)
	{
		free(pd);
		return;
	}

	inf=dataPacketSearch(us,p->src,pd->id);

	if (inf==NULL)
	{
#ifdef DEBUG_DATA
		fprintf(stderr,"node %d: data: new inf structure...\n",us->addr & 0xFF);
#endif
		/* insert entry, so we can rexmit the ACK if the DATA packet gets repeated.    */
		inf=dataPacketInsert(us,p,pd);
		dontfreepd=1;
	}

#ifdef DEBUG_DATA
	fprintf(stderr,"node %d: data: id= %u inf->finalStatus= %d\n",us->addr & 0xFF,pd->id,inf->finalStatus);
#endif

	if (inf->finalStatus==DATA_STATUS_UNACKED)   /* If we have not seen this packet, (identified by the tuple srcaddr, id)  */
	{
#ifdef DEBUG_DATA
		fprintf(stderr,"node %d: data: decapsulating payload,  src= %d route= 0x%x ack= %d len= %d type= %x\n",us->addr & 0xFF,p->src & 0xFF,pd->routetype,pd->acktype,p->len - pd->len,pd->origtype);
#endif

		/* The first time we receive a packet, it will be unacked, and thats the only time we want to decapsulate and deliver it  */
		
		cpy=packetCopy(p,-pd->len);          /* decapsulate the enclosed packet */
		if (pd->destinationCount!=0)		/* there are 3 cases for this, pure broadcast/multicast (p->dst==NODE_BROADCAST or a multicast addr destinationCount==0), pure unicast (p->dst=us->addr, destinationcount==0), multiple-destinations broadcast (pd->dst==NODE_BROADCAST, destinationCount>0).  In the third case, we need to reset the packet's observed dst addr to our own (its /not/ a broadcast, though it was carried in one)  */
			cpy->dst=us->addr;
		cpy->type=pd->origtype;

#ifdef DEBUG_DATA
		fprintf(stderr,"node %d: data: src= %d dst= %d type= %x len= %d\n",us->addr & 0xFF,cpy->src & 0xFF, cpy->dst & 0xFF, cpy->type,cpy->len);
#endif

		packetReReceive(us,cpy);

		packetFree(cpy);
		if (p->src!=us->addr)
			inf->finalStatus=DATA_STATUS_ACKED;
	}

	if (inf->pd->acktype!=DATA_ACK_NONE)
	{
		if (p->src == NODE_BROADCAST || NODE_IS_MULTICAST(p->src))
		{
			fprintf(stderr,"node %d: data: WARNING: refusing to send ack to bogus src address %u.%u.%u.%u\n",
				us->addr & 0xFF, PRINTADDR(p->src));
		}
		else
		{
			PacketDataAck ackpd;
			/* But, every time we get a data packet ,we want to send an ack, in case the ack was lost...   */

			ackpd.id=pd->id;
			ack=packetDataAckMarshal(us,&ackpd);		 /* send ACK packet to originating node */
			/* XXX should we check for bogus (e.g., broadcast) src? */
			ack->dst=p->src;

#ifdef DEBUG_DATA
			fprintf(stderr,"node %d: data: sending ACK to %d   route= 0x%x ack= %d len= %d\n",us->addr & 0xFF,ack->dst & 0xFF,pd->routetype,pd->acktype,ack->len);
#endif

//			intDataSend(us,ack,pd->routetype);
// #warning how to send ACKs back with ambient, with fallback to flood or route?
			packetSend(us,ack, PACKET_ORIGIN);

			packetFree(ack);
		}
	}
	if (!dontfreepd)
		free(pd);
}

/* For an incoming ACK
 * do a lookup for inflight   (key our-addr, id)
 * mark that destination ACKed
 * Are all the destinations ACKed?
 *	Yes, send ACK packet to calling module
 */
static void dataAck(manetNode *us, packet *p)
{
	PacketDataAck *ackpd;
	int i;
	int allacked=1;
	InFlight *inf;

#ifdef DEBUG_DATA
	fprintf(stderr,"node %d: data: got an ack src= %d dst= %d\n",us->addr & 0xFF,p->src & 0xFF, p->dst & 0xFF);
#endif

	if (p->dst!=us->addr)
		return;

	ackpd=packetDataAckUnmarshal(p);
	if (ackpd==NULL)
	{
		fprintf(stderr,"node %d: %s: unmarshal failed, len= %d\n",
			us->addr & 0xFF, __func__, p->len);
		return;
	}
	inf=dataPacketSearch(us,us->addr,ackpd->id);

	if (inf)
	{

#ifdef DEBUG_DATA
		fprintf(stderr,"data: found our copy of the inflight id= %u\n",ackpd->id);
#endif

		for(i=0;i<inf->pd->destinationCount;i++)
			if (inf->pd->destinationList[i]==p->src)
				break;

		if (inf->pd->destinationList[i]==p->src)    /* if the sender of the ack is on our list of receivers...  */
		{

			inf->pd->destinationAck[i]=DATA_ACK;    /* then its acked...   */

			for(i=0;i<inf->pd->destinationCount;i++)    /* are all receivers acked?   */
			{
	#ifdef DEBUG_DATA
				fprintf(stderr,"data:  %d: %d %s\n",i,inf->pd->destinationList[i] & 0xFF,inf->pd->destinationAck[i]!=DATA_ACK?"unacked":"acked");
	#endif
				if (inf->pd->destinationAck[i]!=DATA_ACK)
					allacked=0;
			}

			if (allacked)                        /* yep, all acked...  send ack to caller.  */
			{
				inf->finalStatus=DATA_STATUS_ACKED;
	#ifdef DEBUG_DATA
				fprintf(stderr,"node %d: data: got an ack for us from %d  id= %u.  sending ACK to caller\n",us->addr & 0xFF,p->src & 0xFF,inf->pd->id);
	#endif
				sendPacketAck(us, inf);
			}
		}
	}
	/* If we don't have an entry for this packet, then its already been NAKed in the timeout, and we don't do it again (so, we get a
	** false indication that it was not delivered.
	*/
	free(ackpd);
}

void dataSend(manetNode *us, packet *p,DataRoute route, DataAckType ack, unsigned int *id)
{
	ManetAddr dst=p->dst;

	dataSendMulti(us, p, &dst,1, route, ack, id);
}

/* Send packet p, encapsulated in a DATA packet.  
 * On delivery, the encapsulated packet will be packetEnqueued, which will make it look like it wasn't encapsulated.
 */
void dataSendMulti(manetNode *us, packet *p, ManetAddr *destinationList, int destinationCount, DataRoute route, DataAckType ack, unsigned int *id)
{
	PacketData *pd;
	int i;
	InFlight *inf;

	pd=(PacketData*)malloc(sizeof(*pd)+(sizeof(pd->destinationList[0]) * destinationCount ) + ( sizeof(pd->destinationAck[0]) * destinationCount));

	pd->destinationList=(ManetAddr*)((char *)pd+sizeof(*pd));
	pd->destinationAck=(DataAck*)((char*)(pd->destinationList) + ( sizeof(pd->destinationList[0]) * destinationCount ) );
	for(i=0;i<destinationCount;i++)
	{
		pd->destinationList[i]=destinationList[i];
		pd->destinationAck[i]=DATA_NAK;
	}
	pd->destinationCount=destinationCount;

	pd->id=us->data->nextid++;
	*id=pd->id;
	pd->routetype=route;
	pd->acktype=ack;
	pd->origtype=p->type;
	pd->xmitnum=0;

	inf=dataPacketInsert(us,p,pd);          /* insert outgoing data packet into pending ACK list  */
		/* and do not free pd, because dataPacketInsert takes ownership of it */

	intDataSendData(us,inf, PACKET_ORIGIN);
}


static void dataTimeoutInFlight(char *key,
                                void *data)
{
	InFlight *p=(InFlight*)data;
	manetNode *us=p->us;

#ifdef DEBUG_DATA
	fprintf(stderr,"node %d: dataTimeout  src= %d dst= %d id= %u status= %d rexmit= %lld acktime= %lld entrytime= %lld routetype= 0x%x\n",us->addr & 0xFF,p->p->src & 0xFF, p->p->dst & 0xFF ,p->pd->id,p->finalStatus,p->rexmittimeout,p->acktimeout,p->entrytimeout,p->pd->routetype);
#endif

	if (p->entrytimeout < us->manet->curtime)    /* Is inflight entry all done?   */
	{
#ifdef DEBUG_DATA
		fprintf(stderr,"node %d: data: entry timeout, deleting status= %d id= %u\n",us->addr & 0xFF,p->finalStatus,p->pd->id);
#endif
		dataPacketRemove(us,p);    /* remove from pending ACK list */
	}
	else if (p->acktimeout < us->manet->curtime)    /* Is the entry too old to rexmit? */
	{
		/* rerecieve NAK  to caller */
		/* mark as ACKed */
		/* entry will be ignored until entrytimeout, when it gets deleted*/

		if (p->finalStatus==DATA_STATUS_ACKED)    /* this packet has already been acked, so duplicate caller ACK, ignore.  */
			return;
		if ((us->data->dataFailover) && (!(p->pd->routetype & DATA_ROUTE_NOFAILOVER)))
			switch(p->pd->routetype & DATA_ROUTE_MASK)
			{
				case DATA_ROUTE_HIERARCHY:
				case DATA_ROUTE_AMBIENT:
				case DATA_ROUTE_ROUTING:
					/* we were ambient...  switch to FLOOD, and try again */
					p->entrytimeout= us->manet->curtime+(us->data->dataTimeoutEntry);
					p->rexmittimeout= us->manet->curtime+us->data->dataRexmit;
					p->acktimeout= us->manet->curtime+us->data->dataTimeout;
					p->pd->routetype=DATA_ROUTE_FLOOD;

					p->rexmittimeout=us->manet->curtime+us->data->dataRexmit;
					p->xmitnum=0;

					intDataSendData(us,p, PACKET_REPEAT);
				break;
				case DATA_ROUTE_FLOOD:
					/* we have timed out doing FLOOD.  giveup, and send caller its NAK
					 */

					p->acktimeout=p->entrytimeout;    /* IE: do not call this condition again...   */
					p->rexmittimeout=p->entrytimeout;    /* IE: do not call this condition again...   */

#ifdef DEBUG_DATA
					fprintf(stderr,"node %d: data: timeout, sending NAK to API id= %u type= %x\n",us->addr & 0xFF,p->pd->id,p->pd->origtype+1);
#endif
					p->finalStatus=DATA_STATUS_ACKED;
					sendPacketAck(us,p);    /* This will be a negative ACK.  IE: we didn't get ACKs from all the destinations.  */
				break;
			}
		else
		{
			/* we have timed out doing FLOOD.  giveup, and send caller its NAK
			 */

			p->acktimeout=p->entrytimeout;    /* IE: do not call this condition again...   */
			p->rexmittimeout=p->entrytimeout;    /* IE: do not call this condition again...   */

#ifdef DEBUG_DATA
			fprintf(stderr,"node %d: data: timeout, sending NAK to API id= %u type= %x\n",us->addr & 0xFF,p->pd->id,p->pd->origtype+1);
#endif
			p->finalStatus=DATA_STATUS_ACKED;
			sendPacketAck(us,p);    /* This will be a negative ACK.  IE: we didn't get ACKs from all the destinations.  */
		}

	}
	else if (p->rexmittimeout < us->manet->curtime)    /* has the rexmit timer gone off?  */
	{
		if (p->finalStatus==DATA_STATUS_ACKED)
		{
			p->rexmittimeout=p->entrytimeout;      /*IE: do not call this condition again  */
			return;
		}

#ifdef DEBUG_DATA
		fprintf(stderr,"node %d: data: retransmitting dst= %d id= %u\n",us->addr & 0xFF,p->p->dst & 0xFF,p->pd->id);
#endif

		p->rexmittimeout=us->manet->curtime+us->data->dataRexmit;

		intDataSendData(us,p, PACKET_REPEAT);
	}
}

/* This is called regularly to walk the list of inflight packets, and
** send NAKs for the ones which have not been ACKed.
**  Should it rexmit instead?
*/
static void dataTimeout(manetNode *us, void *data)
{
	timerSet(us,dataTimeout,(us->data->dataRexmit/2),NULL);
#ifdef DEBUG_DATA
//	fprintf(stderr,"node %d: dataTimeout called time= %lld\n",us->addr & 0xFF,us->manet->curtime);
#endif

	hashtabletraverse(us->data->inflight,dataTimeoutInFlight);
}

/* should the routing system be doing that encapsulation trick too?  
** IE: you call routeSend(), and your packet magically gets to the
##     destination...
** Problem with that here, is then we can't do hop by hop ACKs...  
*/

/* convenience functions for marshaling multi-byte vars into a header.
 * note that they are backwards!
 */
#define MARSHALLONGREV(a,b) \
	do\
	{\
		*--a=((unsigned char)((b) >> 24)); \
		*--a=((unsigned char)((b) >> 16)); \
		*--a=((unsigned char)((b) >> 8)); \
		*--a=((unsigned char)(b)); \
	} while(0)

#define UNMARSHALLONGREV(a,b)  {b=(*(a-1)<<24) | (*(a-2)<<16) | (*(a-3)<<8) | *(a-4); a-=4;}

#define MARSHALBYTEREV(a,b)\
	do\
	{\
		*--a=((unsigned char)(b)); \
	} while(0)

#define UNMARSHALBYTEREV(a,b)  {b=*(--a);}


#define PACKETDATA_HEADERLEN(dcount)                            \
        ((dcount)*4     /* list of addrs */                     \
         + 1            /* destinationCount */                  \
         + 4            /* pd->id  */                           \
         + 1            /* pd->origtype  */                     \
         + 1            /* pd->routetype, pd->acktype */        \
         + 1)           /* pd->xmitnum */

/* Marshal and unmarshal a PacketData header into or out of a data packet 
 * the header is actually appended, not prepended.  So, it is "backwards".
 */
PacketData *packetDataUnmarshal(const packet *p)
{
	PacketData *pd;
	unsigned char *hp;
	int i,destinationCount;

	if (p->type!=PACKET_DATA_DATA)
		return NULL;

        if (p->len < PACKETDATA_HEADERLEN(0))
        {
		return NULL;
        }            

	hp=((unsigned char*)p->data)+p->len;

	/* parse the header appended to the packet...  unfold it into *pd  
	*/
	UNMARSHALBYTEREV(hp,destinationCount);

        if (p->len < PACKETDATA_HEADERLEN(destinationCount))
        {
		return NULL;
        }            

	pd=(PacketData*)malloc(sizeof(*pd)+sizeof(pd->destinationList[0])*destinationCount);
	pd->destinationList=(ManetAddr*)((unsigned char *)pd + sizeof(*pd));
	pd->destinationAck=NULL;

	pd->destinationCount=destinationCount;
	UNMARSHALLONGREV(hp,pd->id);
	UNMARSHALBYTEREV(hp,pd->origtype);
	UNMARSHALBYTEREV(hp,i);
// #warning not enough bits for route type
	pd->routetype = (DataRoute)(i & 0x03);
	pd->acktype = (DataAckType)((i >> 2) & 0x03);
	UNMARSHALBYTEREV(hp,pd->xmitnum);

	for(i=0;i<pd->destinationCount;i++)
		UNMARSHALLONGREV(hp,pd->destinationList[i]);

	pd->len=PACKETDATA_HEADERLEN(pd->destinationCount);
	assert(hp == ((unsigned char *)p->data) + p->len - pd->len);

	return pd;
}

/* NOTE: modifies pd */
static packet *packetDataMarshal(const packet *p, PacketData *pd)
{
	unsigned char *hp;
	packet *n;
	int i;

	/* determine how big the data module header is.  allocate it and fill it out.  original type from *p and update it to data module type */

	pd->len=PACKETDATA_HEADERLEN(pd->destinationCount);

	n=packetCopy(p,pd->len);
	pd->origtype=p->type;
	n->type=PACKET_DATA_DATA;
	hp=((unsigned char *)n->data) + n->len;
	MARSHALBYTEREV(hp,pd->destinationCount);
	MARSHALLONGREV(hp,pd->id);
	MARSHALBYTEREV(hp,pd->origtype);
	i=(pd->routetype & 0x3 ) | ((pd->acktype & 0x03) << 2);
	MARSHALBYTEREV(hp,i);
	MARSHALBYTEREV(hp,pd->xmitnum);

	for(i=0;i<pd->destinationCount;i++)
		MARSHALLONGREV(hp,pd->destinationList[i]);

        assert(hp == ((unsigned char *)n->data) + p->len);
	return n;
}

/* Now we're doing the marshaling forwards, since the ACK packets are
 * not an appended header, so we use the standard MARSHALLONG and
 * friends from marshal.h.
 */

static packet *dataPacketAckMarshal(manetNode *us, InFlight *inf)
{
	int i,len;
	packet *p;
	unsigned char *hp;

	len= 1 +   /* pd->destinationCount */
		4 +  /* pd->id */
		5 * inf->pd->destinationCount;   /* pd->destinationList[i] + pd->destinationAck[i] */

	p=packetMalloc(us,len);
	p->src=inf->p->src;
	p->dst=us->addr;
	p->type=inf->p->type+1;
	hp=(unsigned char *)p->data;

	MARSHALBYTE(hp,inf->pd->destinationCount);
	MARSHALLONG(hp,inf->pd->id);
	for(i=0;i<inf->pd->destinationCount;i++)
	{
		MARSHALLONG(hp,inf->pd->destinationList[i]);
		MARSHALBYTE(hp,inf->pd->destinationAck[i]);
	}

	return p;
}

DataPacketAck *dataPacketAckUnmarshal(const packet *p)
{
	DataPacketAck *pd;
	int i,j,numdest;
	unsigned char *hp;

	hp=(unsigned char*)p->data;
	UNMARSHALBYTE(hp,numdest);

	pd=(DataPacketAck*)malloc(sizeof(*pd) + (sizeof(pd->destinationList[0]) * numdest) + (sizeof(pd->destinationAck[0]) * numdest));
	pd->destinationList=(ManetAddr*)((char *)pd + sizeof(*pd));
	pd->destinationAck=(DataAck*)((char *)pd->destinationList + (sizeof(pd->destinationList[0]) * numdest));

	pd->destinationCount=numdest;
	UNMARSHALLONG(hp,pd->id);
	for(i=0;i<numdest;i++)
	{
		UNMARSHALLONG(hp,pd->destinationList[i]);
		UNMARSHALBYTE(hp,j);
		pd->destinationAck[i]=(DataAck)j;
	}

	return pd;
}

static packet *packetDataAckMarshal(manetNode *us, PacketDataAck *pda)
{
	packet *ack;
	unsigned char *hp;

	ack=packetMalloc(us,4);              /* send ACK packet to originating node */
	ack->type=PACKET_DATA_ACK;
	ack->ttl=255;

	hp=(unsigned char*)ack->data;
	MARSHALLONG(hp,pda->id);
	assert(hp == ((unsigned char *)ack->data) + ack->len);

	return ack;
}

PacketDataAck *packetDataAckUnmarshal(const packet *p)
{
	if (p->len < 4)
	{
		return NULL;   /* too short */
	}
	PacketDataAck *pda=(PacketDataAck*)malloc(sizeof(*pda));
	unsigned char *hp=(unsigned char*)p->data;

	UNMARSHALLONG(hp,pda->id);

	return pda;
}
#endif
