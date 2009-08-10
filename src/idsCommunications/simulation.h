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

#ifndef SIMULATION_H
#define SIMULATION_H

#include "des.h"

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
