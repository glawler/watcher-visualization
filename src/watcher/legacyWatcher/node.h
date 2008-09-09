#ifndef NODE_H
#define NODE_H

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: node.h,v 1.41 2007/07/11 03:50:45 dkindred Exp $
 */

#include "des.h"

#ifdef __cplusplus
extern "C" {
#endif

packet *nodePacketSearch(manetNode *us, packet *p);
void nodePacketInsert(manetNode *us, packet *p);

/* A clustering algorithm exports a neighbor list, using these functions
 *
 * Neighbors can have levels, and even multiple levels (with separate node structs).
 * BUT all the levels are considered equivilant when the packetAPI code is generating
 * its neighbor list.
 * 
 * Neighbors have flags (in neighbor->flags).  They are defined in des.h, the
 * NEIGHBOR_ macros.  The HEARD and HEARS macros indicate if a neighbor is asymmetrically
 * heard (HEARD is set) or symmetrically connected to (HEARS, think S for symmetric).
 * a neighbor must have its hopcount field set.  0 is infinity, 1 is one hop, etc.
 * If a neighbor is 1 hop away, set the 1HOP flag.  (redundant from hopcount... need code cleanup)
 * If a neighbor is a hierarchy child, set the CHILD flag  (note that children can be
 * multihop, these neighbors are NOT just onehop neighbors)
 * If a neighbor is a hierarchy parent, set the PARENT flag, and in the node struct,
 * set the clusterhead field to point to that neighbor struct.  More than one parent
 * is acceptable (intended for redundancy)
 */

neighbor *neighborSearch(manetNode *us, ManetAddr addr,int level);
/* search the neighbor list for the node addr at level level
*/

neighbor *neighborInsert(manetNode *us, int addr, int level);
/* insert a new neighbor, at addr, level level
*/


/* These two functions may delete neighbor structs.  But the clusterhead field may point
 * into the neighbor list...  So these compare the pointer values, and if clusterhead points
 * to the neighbor struct to be freed, it sets clusterhead to NULL.  keep that in mind... 
 */
void neighborNuke(manetNode *us);
/* Deallocate the neighbor list
*/

void neighborDelete(manetNode *us, neighbor *n);
/* delete neighbor n from the neighbors list
*/

int neighborCount(manetNode *us, int level);
/* returns the number of neighbors with level == to level arg
*/


/* If schedflag is true, then we're in the simulation, and schedule
 * a callback.  Otherwise, just make a report (to stdout) and return.
 */
void nodeMetrics(manetNode *us,int schedflag);

int *nodePathLength(manet *m);
int nodeAvailability(manetNode *us,int *flagarray);      /* pass a NULL for the second arg.  (used only for recursing)  */
double manetAvailability(manet *m);
int manetGetNodeNum(manet *m, ManetAddr addr);    /* given a ManetAddr, returns the index of that node.  (or -1 if not found)  */
manetNode *manetNodeSearchAddress(manet *m, ManetAddr addr);		/* given a ManetAddr, returns PTR to its node struct.  */


int *manetGetPhysicalGraph(manet *m);            /* returns the RF propigation model  */
int *manetGetHierarchyGraph(manet *m);           /* returns the clusterhead pointer model   */
int *manetGetHierarchyLevels(manet *m);		/* return the levels of the nodes, as computed by the hierarchy algorithm */
int *manetGetNeighborGraph(manet *m);            /* returns the linklayer neighbor list model  */

void graphMinPath(manet *m, int *distance);               /* ponder graphs here...  */
void graphTransitiveClosure(manet *m, int *distnace);
void graphIntersection(manet *m, int *a, int *b); /* not yet implemented */
void graphMakeBidirectional(manet *m, int *g);
void graphMultiply(manet *m, int *a, int *b);
void graphAnd(manet *m, int *a, int *b);
int *graphNodeDegree(int *a, int num);
int *graphComputeLevels(manet *m, int *hierarchygraph);
void graphDump(manet *m, char *label,int *graph, FILE *fd);
double graphAggregationCompute(manet *m, int *hierarchygraph, int *physicalgraphminpath,int numphysicalpartitions, double *aggregation);

int graphNumPartitions(manet *m,int *g);

void nodeHierarchyCountEdges(manetNode *us, int schedflag);

NodeLabel *nodeLabelApply(manetNode *us, NodeLabel *lab);
void nodeLabelRemovePtr(manetNode *us,NodeLabel *l);
void nodeLabelRemove(manetNode *us,int bitmap, NodeLabel *l);
void nodeLabelTimeout(manetNode *us);     /* walk list of labels, and remove the ones which have timed out  */
/* Values for bitmap arg are defined in idsCommunications.h:  */

void nodeLabelRemoveTag(manetNode *us,NodeLabel *l);
void nodeLabelRemoveAll(manetNode *us);
void nodeColor(manetNode *us, unsigned char *color);

#ifndef MIN
#define MIN(a,b)  ((a>b)?(b):(a))
#endif
#ifndef MAX
#define MAX(a,b)  ((a<b)?(b):(a))
#endif

#ifdef __cplusplus
}
#endif

#endif
