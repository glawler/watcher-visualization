#ifndef SIMULATION_H
#define SIMULATION_H

#include "des.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: simulation.h,v 1.22 2007/04/25 14:20:05 dkindred Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

/* packet types */
#define PACKET_INTERIM_HELLO		(PACKET_INTERIM|1)

/* packet counts
 * a node must send out this many HELLOs before it can be clusterhead or root
 */
#define HELLO_CLUSTERHEAD	2

typedef struct
{
	int onehopcount;          /* number of neighbors */
	int symcount;             /* number of symmetric neighbors */
	int clusterhead;
	int level;                     /* hierarchy level  */
	int sequencenum;
	ManetAddr addresslist[40];
} packet_hello;

#define TIME_HELLO 2000
#define TIME_HELLO_TIMEOUT 10000

void nodeInit(manetNode *n);
void nodeFree(manetNode *n);

void nodeGotPacket(manetNode *us, packet *p);

#ifdef __cplusplus
}
#endif

#endif
