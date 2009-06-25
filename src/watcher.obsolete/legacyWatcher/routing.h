#ifndef ROUTING_H
#define ROUTING_H

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: routing.h,v 1.16 2007/04/25 14:20:05 dkindred Exp $
 */

#ifdef MODULE_ROUTING

#include"des.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_ROUTING_ROUTE (PACKET_ROUTING | 0)
#define PACKET_ROUTING_DATA  (PACKET_ROUTING | 1)
#define PACKET_ROUTING_TEST  (PACKET_ROUTING | 2)

typedef struct
{
	int type;
	int numhops;
	ManetAddr src,dst;     /* destination on RREQUEST...  */
	unsigned int seqnum;
	ManetAddr hops[40];
} packetRouting;

#define ROUTE_RREQUEST 1
#define ROUTE_RREPLY   2

typedef struct
{
	int origtype;
	ManetAddr origsrc;
	ManetAddr origdst;
} packetRoutingData;

/* dest and nexthop can be equal.  for 1 hop neighbors.  (since the routing algorithm can't see the
 * link level stuff
 */

typedef struct routingNode
{
	struct routingNode *next;
	ManetAddr dst;        /* to get to dest...   */
	ManetAddr nexthop;     /* you send the packet to nexthop...  (and assume its paying attention) */
	int length;      /* length of route, for recognizing the shortest one...  */
	destime expiretime;
} routingNode;

typedef struct routingRequest
{
	struct routingRequest *next;
	ManetAddr src,dst;                  /* src, dst and seqnum are the key  */
	unsigned int seqnum;
	int hopcount;
} routingRequest;

typedef struct routingState
{
	routingNode *routelist;
	packet *unsentlist;
	routingRequest *requestlist;
	unsigned int reqseqnum;
} routingState;


/* Init any data structures...   */

/* XXX these functions should probably be named routingInit(), routingSend(), 
 * routingTest(), routingDumpBuffered() for consistency.  -dkindred */

void routeInit(manetNode *us);

/* call to send packet p to node dest */
void routeSend(manetNode *us, packet *p);

void routeTest(manetNode *n);

void routeDumpBuffered(manetNode *us,FILE *fd);

#ifdef __cplusplus
}
#endif

#endif
#endif
