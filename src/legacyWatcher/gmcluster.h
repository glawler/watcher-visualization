#ifndef GROUP_MOBILITY_DER_BORKBORKBORK_CLUSTER_H
#define GROUP_MOBILITY_DER_BORKBORKBORK_CLUSTER_H

#include "des.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: gmcluster.h,v 1.9 2007/04/25 14:20:05 dkindred Exp $
*/

#define PACKET_GROUP_MOBILTY_GPS 	(PACKET_GROUP_MOBILITY|1)
#define PACKET_GROUP_ID_ASSERTION 	(PACKET_GROUP_MOBILITY|2)

/* define various defaults
 * these can all be overridden with config file settings
 * (These might belong in the .c file, but, meh, whatever).
 */
#define GMCLUSTER_ANTENNARADIUS_DEFAULT			190
#define GMCLUSTER_POSITIONHISTORY_DEFAULT		30
#define GMCLUSTER_ACCEPTABLE_LINK_EXPIRATION_TIME	60
#define DEFAULT_GID_HEARTBEAT_TIMEOUT_FACTOR		3
#define DEFAULT_GID_HEARTBEAT_INTERVAL			2

#ifdef __cplusplus
extern "C" {
#endif

/* aliases for nodeInit(), nodeFree(). Can be called by liveinterim2
 * (or whoever) in place of nodeInit() */
void gmclusterNodeInit(manetNode *); 
void gmclusterNodeFree(manetNode *n);

void nodeGotPacket(manetNode *us, packet *p);

/*
 * Returns non-zero if n is in "us"'s group
 */
int gmclusterInMyGroup(const manetNode *us, const neighbor *n);

#ifdef __cplusplus
}
#endif

#endif /* GROUP_MOBILITY_DER_BORKBORKBORK_CLUSTER_H */
