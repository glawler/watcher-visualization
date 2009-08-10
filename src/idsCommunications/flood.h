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

#ifndef FLOOD_H
#define FLOOD_H

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
