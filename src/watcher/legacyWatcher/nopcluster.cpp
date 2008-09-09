#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef USE_RNG
#include "rng.h"
#endif
#include "graphics.h"
#include "node.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is the NOP clustering algorithm.  It does nothing, except assist with benchmarking
 * the simulator, and document the clustering algorithm interface.
 *
 * A clustering algorithm will be assigned a packet type, which sets the most significant bits
 * of the 8 bit packettype value, and allows the clustering algorithm to use the least significant
 * bits for its own [neferious] purposes.  See the PACKET_ macros in des.h, and PACKET_DATA macros
 * in data.h for an example.
 *
 * The clustering algorithm must export a neighbor list.  See node.h, for the neighbors functions.
 *
 * The clustering algorithm must export a clusterhead, which is doen with the neighbor list, see node.h
 *
 * The clustering algorithm must export hierarchy positions     (This needs to be cleaned up!)
 *   if node->rootflag is set, then the node is the root node.  
 *   if any of the neighbors have NEIGHBOR_CHILD set, then the node is a neighborhood coordinator
 *   if any of the neighbors have NEIGHBOR_CHILD set, and our level (node->level) is >0, then the
 *    node is a regional coordinator.
 *
 * A clustering algorithm is given position weights, to assign nodes specific positions.
 *  These are gotten by calling communicationsPositionWeightSearchList, defined in idsCommunications.
 *
 * Clustering routing algorithm?
*/

static const char *rcsid __attribute__ ((unused)) = "$Id: nopcluster.cpp,v 1.15 2007/08/17 19:42:45 dkindred Exp $";

/* A clustering algorithm gets the field us->cluster for its private per-node data
 * It will be inited to NULL, and can be malloced in nodeInit() (below).
 */
typedef struct clusteringState
{
} clusteringState;

/* A clustering algorithm gets the field neighbor->cluster for private per-neighbor data
 * When a neighbor is malloced, neighbor->cluster is set to NULL.  
 * when a neighbor is freed, if it is non-NULL, it will be freed as well.
 */
typedef struct clusteringNeighbor
{
} clusteringNeighbor;

/* If a clustering algorithm wants random numbers, it needs its own private random number generator
 */
#ifdef USE_RNG
static RNG *rnd=NULL;
#define RAND_U01() (rnd->rand_u01())
#else
#define RAND_U01() (drand48())
#endif

/* Called by simulation code when a packet arrives at a node
 */
static void nopPacket(manetNode *us, packet *p)
{
}

/* Called by simulation code on each node to setup 
 */
void nodeInit(manetNode *us)
{
	us->clusterhead=NULL;
        us->neighborlist=NULL;
        us->level=0;
        us->rootflag=1;

#ifdef USE_RNG
	if (rnd==NULL)
		rnd= new RNG(configSearchInt(us->manet->conf,"nop_randomseed"));
#endif

	manetPacketHandlerSet(us, PACKET_NOPCLUSTER, nopPacket);
}

void nodeFree(manetNode *n)
{
}
