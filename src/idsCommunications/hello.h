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

#ifndef HELLO_H
#define HELLO_H

#include "des.h"

#ifdef MODULE_HELLO

#define PACKET_HELLO_HELLO	(PACKET_HELLO|1)

#define TIME_HELLO 2000
#define TIME_HELLO_TIMEOUT 10000

#ifdef __cplusplus
extern "C" {
#endif

/* Called when a neighbor arrives, or departs 
 */
typedef void helloNeighbor(manetNode *us, neighbor *n,int present);

/* Called when a node becomes, or ceases to be, a first level CH
 */
typedef void helloCH(manetNode *us, int present);

/* Called when a node gets a HELLO packet
 * if the packet had a piggypacked payload, it will be passed in the payload args
 * caller owns the payload, so callee must copy it if it wants to keep it.
 */
typedef void helloHello(manetNode *us,neighbor *ch, neighbor *src, const unsigned char *payload, int payloadLen);


/* So the evil plot is:

This is a sub-clustering algorithm.  It will send OLSR style HELLO packets, mantain a 
list of one hop neighbors, and do the greatest degree thing on them.

If a clustering algorithm wants to use it, they call helloInit(), with some callbacks.
The init call will then schedule its own callbacks to transmit the hello packets.  The packet reception
is already there, since this looks more like a clustering algorithm to the des code.
        
The callbacks:

neighborcallback: called when a neighbor arrives or departs (look at the present arg)
chcallback: called when a node becomes a clusterhead, or ceases to be a clusterhead (again, present arg)
helloCallback: called whenever a HELLO packet arrives.  Also indicates the current clusterhead

Look at testhello for an example...

Call helloFree from your nodeFree...
*/

void helloInit(manetNode *us, helloNeighbor *neighborcallback, helloCH *chcallback, helloHello *helloCallback);
void helloFree(manetNode *us);

/* Set a payload to piggypack on the hello packets.
 * The payload will not be copied, the hello module will refer to whatever the pointer points
 * to.  The payload will be sent in each hello packet until a new payload is set, or
 * removed (call with payloadLen==0)
 */
void helloPayloadSet(manetNode *us, unsigned char *payload, int payloadLen);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_HELLO */
#endif /* !HELLO_H */
