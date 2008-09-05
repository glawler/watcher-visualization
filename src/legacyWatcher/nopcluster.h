#ifndef NOPCLUSTER_H
#define NOPCLUSTER_H

#include "des.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: nopcluster.h,v 1.5 2007/04/25 14:20:05 dkindred Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

void nodeGotPacket(node *us, packet *p);

#ifdef __cplusplus
}
#endif

#endif
