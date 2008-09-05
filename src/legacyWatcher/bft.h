#ifndef BANNERJEE_H
#define BANNERJEE_H

#include "des.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: bft.h,v 1.7 2007/04/25 14:20:05 dkindred Exp $
 */

#define PACKET_BFT_HELLO      (PACKET_BFT|1)
#define PACKET_BFT_TREE       (PACKET_BFT|2)


#define TIME_TREE 2000
#define TIME_HELLO 2000
#define TIME_HELLO_TIMEOUT 10000

/* Additional state information on the node which BFT needs
*/
typedef struct clusteringState
{
        ManetAddr rootid;
        int root_seqnum;
//        int root_distance;
        int subtreesize;

        int mode;
	
	destime lasttreerec,lasttreesent;
	int treeseq;
	int hellosequencenum;
} clusteringState;

/* values for clusteringState.mode
*/
#define NOTREEYET 1
#define TREEINIT  2
#define FLOATINGROOT  3
#define TREEREADY 4

typedef struct
{
        int onehopcount;          /* number of neighbors */
        int symcount;             /* number of symmetric neighbors */
        int sequencenum;
	ManetAddr clusterhead;
        ManetAddr addresslist[40];
} packet_hello;

void nodeGotPacket(manetNode *us, packet *p);

#endif
