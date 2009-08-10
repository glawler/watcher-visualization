/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TESTTRAFFIC_H
#define TESTTRAFFIC_H

#ifdef MODULE_TESTTRAFFIC

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
