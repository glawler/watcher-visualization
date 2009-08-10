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

#include "marshal.h"
#include "flood.h"
#include "node.h"

/*
 * This implements flooding of packets.
 */


#define FLOOD_HEADERSIZE (4*4)

/* NOTE: bounds checking against packet len done */
packetFlood *packetFloodUnmarshal(const packet *p)
{
	packetFlood *pf;
	unsigned char *hp=((unsigned char*)p->data)+(p->len - FLOOD_HEADERSIZE);
        if (p->len < FLOOD_HEADERSIZE)
        {
		return NULL;  /* packet too short */
        }

	pf=(packetFlood*)malloc(sizeof(*pf));

	UNMARSHALLONG(hp,pf->id);
	UNMARSHALLONG(hp,pf->origtype);
	UNMARSHALLONG(hp,pf->origsrc);
	UNMARSHALLONG(hp,pf->origdst);
        assert(hp == ((unsigned char *)p->data) + p->len);
	
	return pf;
}

/* return 0 on success */
/* NOTE: bounds checking against packet len done */
static int packetFloodMarshal(packet *p, packetFlood *pf)
{
	unsigned char *hp=((unsigned char*)p->data)+(p->len - FLOOD_HEADERSIZE);
        if (p->len < FLOOD_HEADERSIZE)
        {
		return -1;  /* packet too short */
        }

	MARSHALLONG(hp,pf->id);
	MARSHALLONG(hp,pf->origtype);
	MARSHALLONG(hp,pf->origsrc);
	MARSHALLONG(hp,pf->origdst);
        assert(hp == ((unsigned char *)p->data) + p->len);
	return 0;
}

void floodPacket(manetNode *us, packet *p);

void floodInit(manetNode *us)
{
	us->flood=(floodState*)malloc(sizeof(*us->flood));

	us->flood->numheard=0;
	us->flood->nextpos=0;
	us->flood->nextid=us->addr ^ getpid() ^ (getppid() << 16) ^ us->manet->curtime ^ 309680213;

	manetPacketHandlerSet(us, PACKET_FLOOD, floodPacket);
}

/* Search for a record in the last packets heard list
 *
 *  This function cost 31% of the runtime
 */
static floodEntry *floodPacketSearch(manetNode *us, ManetAddr src, int id)
{
	int i;

        /* XXX replace this linear search with a hashtable. */
	for(i=0;i<us->flood->numheard;i++)
		if ((us->flood->lastheard[i].id==id) && (us->flood->lastheard[i].src==src))
			return &(us->flood->lastheard[i]);
	return NULL;
}

static floodEntry *floodPacketInsert(manetNode *us, ManetAddr src, int id)
{
	int op;
	us->flood->lastheard[us->flood->nextpos].src=src;
	us->flood->lastheard[us->flood->nextpos].id=id;

	if (us->flood->numheard<FLOODMAXLASTHEARD)
		us->flood->numheard++;
	op=us->flood->nextpos;
	us->flood->nextpos=(us->flood->nextpos+1) % FLOODMAXLASTHEARD;

	return &(us->flood->lastheard[op]);
}


/* Called when we get a packet...
*/
void floodPacket(manetNode *us, packet *p)
{
	packetFlood *pf;
	packet *cpy;

	pf=packetFloodUnmarshal(p);

	if (pf==NULL)
	{
		fprintf(stderr,"node %d: %s: ERROR: failed unmarshaling (len= %d)\n",
			us->addr & 0xFF,__func__,p->len);
		return;
	}


#ifdef DEBUG_FLOOD
	fprintf(stderr,"node %d: flood, got a packet src= %d origsrc= %d dst= %d origdst= %d origtype= 0x%x id= %u ttl= %d\n",us->addr & 0xFF,p->src & 0xFF,pf->origsrc & 0xFF,p->dst & 0xFF,pf->origdst & 0xFF,pf->origtype,pf->id,p->ttl);
#endif

	if (floodPacketSearch(us,pf->origsrc,pf->id)==NULL)
	{
#ifdef DEBUG_FLOOD
		fprintf(stderr,"node %d: new packet\n",us->addr & 0xFF);
#endif
		floodPacketInsert(us,pf->origsrc,pf->id);    /* add to list of seen packets */

		if ((p->ttl>0) && (pf->origsrc!=us->addr))             /* TTL for one more hop?   flood it.  */
		{
#ifdef DEBUG_FLOOD
			fprintf(stderr,"node %d: flooding packet\n",us->addr & 0xFF);
#endif
			cpy=packetCopy(p,0);
			cpy->hopcount++;
			cpy->ttl--;
			packetSend(us,cpy, PACKET_REPEAT);
//					packetRepeatType(us,pf->origtype);
			packetFree(cpy);
		}

		if ((pf->origdst==NODE_BROADCAST)
		    || (pf->origdst==us->addr)
		    || (pf->origdst==NODE_ROOTGROUP
			&& us->rootgroupflag))
		{
#ifdef DEBUG_FLOOD
			fprintf(stderr,"node %d: flood decapsulating.  src= %d dst= %s id= %u type= 0x%x len= %d\n",us->addr & 0xFF,pf->origsrc & 0xFF,manetAddr2Str(pf->origdst),pf->id,pf->origtype,p->len-sizeof(*pf));
#endif
			cpy=packetCopy(p,-FLOOD_HEADERSIZE);          /* decapsulate payload packet, and deliver to local node  */
			cpy->type=pf->origtype;
			cpy->src=pf->origsrc;
			cpy->dst=pf->origdst;
			packetReReceive(us,cpy);
			packetFree(cpy);
		}
	}
#ifdef DEBUG_FLOOD
	else
		fprintf(stderr,"node %d: discarding dup packet\n",us->addr & 0xFF);
#endif
	free(pf);
}


/* Send packet p, encapsulated in a FLOOD packet.  
** On delivery, the encapsulated packet will be packetEnqueued, which will make it look like it wasn't encapsulated.
*/
void floodSend(manetNode *us, packet *p)
{
	packet *np;
	packetFlood pf;
	int rc;

	np=packetCopy(p,FLOOD_HEADERSIZE);

	assert((p->type <=0xFF) && (p->type >=0));

        /* XXX hmm, should we use secure hash sequence instead of just
         * incrementing?  a predictable sequence means a malicious node
         * could suppress our flood by preemptively sending one with the
         * same origsrc and id.  alternative would be to require the packet
         * to be signed by origsrc to avoid spoofing */
	pf.id=us->flood->nextid++;
	pf.origtype=p->type;
	pf.origsrc=p->src;
	pf.origdst=p->dst;
	rc=packetFloodMarshal(np,&pf);     /* marshal pf into the space in p reserved by the packetCopy above  */
	assert(rc==0);          /* we reserved the space -- can't fail */

	np->type=PACKET_FLOOD;
	np->dst=NODE_BROADCAST;

	np->ttl--;

#ifdef DEBUG_FLOOD	
	fprintf(stderr, "node %d: FLOOD: sending data type= 0x%x  id= %u from %d to %d\n",us->addr & 0xFF, p->type,pf.id, p->src & 0xFF, p->dst & 0xFF);
#endif

//	floodPacketInsert(us,p->src,pf->id);

	packetSend(us,np, PACKET_ORIGIN);
	packetFree(np);
}


/*
 * Why is the world in love again?
 * Why are we marching hand in hand?
 * Why are the ocean levels rising up?
 * It's a brand new record for 1990.
 * They Might be Giants' brand new album:
 * Flood
 *
 *   (Lyrics copyright 1990 They Might Be Giants)
 */
