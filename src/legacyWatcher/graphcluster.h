#ifndef GRAPHCLUSTER_H
#define GRAPHCLUSTER_H

#include "des.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: graphcluster.h,v 1.23 2007/04/25 14:20:05 dkindred Exp $
 */

#define PACKET_GRAPHCLUSTER_HELLO	(PACKET_GRAPHCLUSTER|1)
#define PACKET_GRAPHCLUSTER_TREE	(PACKET_GRAPHCLUSTER|2)
#define PACKET_GRAPHCLUSTER_TERM	(PACKET_GRAPHCLUSTER|3)
#define PACKET_GRAPHCLUSTER_CLUSTER	(PACKET_GRAPHCLUSTER|4)
#define PACKET_GRAPHCLUSTER_DADDY	(PACKET_GRAPHCLUSTER|5)
#define PACKET_GRAPHCLUSTER_DADDYACK	(PACKET_GRAPHCLUSTER|6)

#define TIME_INITTREE   6000
#define TIME_TREEREXMIT 1000
#define TIME_TERM 200
#define TIME_HELLO 2000
#define TIME_HELLO_TIMEOUT 10000

#ifdef __cplusplus
extern "C" {
#endif

void nodeGotPacket(manetNode *us, packet *p);

#ifdef __cplusplus
}
#endif

#endif
