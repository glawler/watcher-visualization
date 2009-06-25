#ifndef FLOOD_H
#define FLOOD_H

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: flood.h,v 1.11 2007/07/11 03:52:28 dkindred Exp $
 */

#ifdef MODULE_FLOOD

#include"des.h"

#define FLOODMAXLASTHEARD 5000

/* low level packet types:
 */
#define PACKET_FLOOD_DATA	 (PACKET_FLOOD|1)

typedef struct floodEntry
{
	ManetAddr src;
	int id;
} floodEntry;

typedef struct packetFlood
{
	int id;
	int origtype;            /* type of encapsulated packet */
	ManetAddr origsrc,origdst;
} packetFlood;


typedef struct floodState
{
	floodEntry lastheard[FLOODMAXLASTHEARD];
	int numheard,nextpos;
	unsigned int nextid;             /* next data packet ID  */
	unsigned int nexttestid;         /* next test packet ID, used only for floodTest  */
} floodState;

#ifdef __cplusplus
extern "C" {
#endif

/* Init any data structures...   */

void floodInit(manetNode *us);

/*  call to send packet p to node dest, via flooding
 *
 */
void floodSend(manetNode *us,packet *p);

void floodTest(manetNode *us);

packetFlood *packetFloodUnmarshal(const packet *p);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_FLOOD */
#endif /* !FLOOD_H */
