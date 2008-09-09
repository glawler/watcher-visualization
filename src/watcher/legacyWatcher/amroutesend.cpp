/*  TELCORDIA TECHNOLOGIES PROPRIETARY - INTERNAL USE ONLY
 *  This file contains proprietary information that shall be distributed,
 *  routed or made available only within Telcordia, except with written
 *  permission of Telcordia.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "des.h"
#include "amroute.h"
#include "routing.h"
#include "flood.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: amroutesend.cpp,v 1.9 2007/04/02 13:58:34 dkindred Exp $";

extern int notifypacket;
static int seqNo = 1;		/* start with 1 */


void sendAMRoutePacket(manetNode *us, ManetAddr dest, int ttl,
        amroutePacketType *type, packet *data)
    {
    packet *p;
    packet_amroute *pamr;
    extern void notifyPacket(manetNode *, packet_amroute *, const char *, int);

    p = packetMalloc(us, sizeof (packet_amroute));
    p->dst = dest;
    p->type = PACKET_AMROUTE;
    p->ttl = ttl;
    pamr = (packet_amroute *) p->data;
    pamr->origination = us->addr;
    pamr->origTime = us->manet->curtime;
    pamr->lastsender = us->addr;
    pamr->lastSendTime = us->manet->curtime;
    pamr->finaldest = dest;
    pamr->hopsSinceOrig = 0;
    pamr->hopsSinceLastSent = 0;
    pamr->sequenceNumber = seqNo++;
    pamr->type = type;
    pamr->data = data;
    type->originated++;
    type->copiesSent++;
    if (notifypacket != 0)
        notifyPacket(us, pamr, "sending", p->ttl);
//    packetOriginate(us, p);
    if (dest != NODE_BROADCAST)
	{
#ifdef MODULE_ROUTE
		routeSend(us, p);
#else
		fprintf(stderr,"routing module not compiled in\n");
		abort();
#endif
	}
    else
        floodSend(us, p);
    packetFree(p);
    }
void sendToNeighbors(int ntype, manetNode *us, amroutePacketType *type,
        packet *data, int maxCopies, int addTreeNeighbor)
    {
    int myIndex, n, i, k;
    packet *p;
    packet_amroute *pamr;
    extern void notifyPacket(manetNode *, packet_amroute *, const char *, int);

    if (us->cluster->nNeighbors[ntype] <= 0)
        return;		/* no one to send to */
    myIndex = addrToIndex(us->manet, us->addr);
    n = seqNo++;
    type->originated++;
    for (i = (myIndex + 1) % us->manet->numnodes, k = 0;
            i != myIndex && (maxCopies <= 0 || k < maxCopies);
            i = (i + 1) % us->manet->numnodes)
        if (includesNeighbor(ntype, us, i))
            {
            k++;
            p = packetMalloc(us, sizeof(packet_amroute));
            p->dst = us->manet->nlist[i].addr;
            p->type = PACKET_AMROUTE;
            p->ttl = INFINITE_TTL;
            pamr = (packet_amroute *) p->data;
            pamr->origination = us->addr;
            pamr->origTime = us->manet->curtime;
            pamr->lastsender = us->addr;
            pamr->lastSendTime = us->manet->curtime;
            pamr->finaldest = us->manet->nlist[i].addr;
            pamr->hopsSinceOrig = 0;
            pamr->hopsSinceLastSent = 0;
            pamr->sequenceNumber = n;
            pamr->type = type;
            pamr->data = data;
            type->copiesSent++;
            if (notifypacket != 0)
                notifyPacket(us, pamr, "sending multicast", p->ttl);
//            packetOriginate(us, p);
#ifdef MODULE_ROUTE
		routeSend(us, p);
#else
		fprintf(stderr,"routing module not compiled in\n");
		abort();
#endif
            packetFree(p);
            if (addTreeNeighbor != 0)
                addNeighbor(TREENEIGHBOR, us, i);
            }
    }
void forwardToNeighbors(int ntype, manetNode *us, packet_amroute *pamr,
        int maxCopies, int addTreeNeighbor)
    {
    int origIndex, senderIndex, i, k;
    packet *p;
    packet_amroute *pamr2;
    extern void notifyPacket(manetNode *, packet_amroute *, const char *, int);

    origIndex = addrToIndex(us->manet, pamr->origination);
    senderIndex = addrToIndex(us->manet, pamr->lastsender);
    for (i = (senderIndex + 1) % us->manet->numnodes, k = 0;
            i != senderIndex && (maxCopies <= 0 || k < maxCopies);
            i = (i + 1) % us->manet->numnodes)
        if (i != origIndex && i != senderIndex && includesNeighbor(ntype, us, i))
            {
            k++;
            p = packetMalloc(us, sizeof(packet_amroute));
            p->dst = us->manet->nlist[i].addr;
            p->type = PACKET_AMROUTE;
            p->ttl = INFINITE_TTL;
            (void) memcpy(p->data, (void *) pamr, sizeof (packet_amroute));
            pamr2 = (packet_amroute *) p->data;
            pamr2->lastsender = us->addr;
            pamr2->lastSendTime = us->manet->curtime;
            pamr2->finaldest = us->manet->nlist[i].addr;
            pamr2->hopsSinceOrig = pamr->hopsSinceOrig;
            pamr2->hopsSinceLastSent = 0;
            pamr2->type->copiesForwarded++;
            if (notifypacket != 0)
                notifyPacket(us, pamr2, "forwarding", p->ttl);
//            packetOriginate(us, p);
#ifdef MODULE_ROUTE
		routeSend(us, p);
#else
		fprintf(stderr,"routing module not compiled in\n");
		abort();
#endif
            packetFree(p);
            if (addTreeNeighbor != 0)
                addNeighbor(TREENEIGHBOR, us, i);
            }
    }
