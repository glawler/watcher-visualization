#ifndef TESTTRAFFIC_H
#define TESTTRAFFIC_H

#ifdef MODULE_TESTTRAFFIC

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: testtraffic.h,v 1.8 2007/04/25 14:20:05 dkindred Exp $
*/

#include"des.h"

#ifdef __cplusplus
extern "C" {
#endif

/* low level packet types:
 */
#define PACKET_TESTTRAFFIC_DATA	 	(PACKET_TESTTRAFFIC|1)
#define PACKET_TESTTRAFFIC_DATAACK	(PACKET_TESTTRAFFIC|2)

typedef struct
{
        int id;
} packetTesttraffic;

typedef struct testtrafficState
{
	int nextid;
} testtrafficState;

/* Init any data structures...   */

void testtrafficInit(manetNode *us);

/*  call to send packet p to node dest, via flooding
 *
 */
void testtrafficSend(manetNode *us,packet *p);

void testtrafficTest(manetNode *us);

#ifdef __cplusplus
}
#endif

#endif
#endif
