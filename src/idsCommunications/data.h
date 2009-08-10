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

#ifndef DATA_H
#define DATA_H

#ifdef MODULE_DATA

#include "des.h"

#ifdef __cplusplus
extern "C" {
#endif

/* low level packet types:
*/
#define PACKET_DATA_DATA	 (PACKET_DATA|1)
#define PACKET_DATA_ACK		(PACKET_DATA|2)

/* Maximum number of destinations in a broadcast packet.
 * (after that number, we will send more than one)
 * Can't be too big because the header comes out of the MTU.
 */
#define DATA_MAX_DEST 10

/* Values of packetAck.destinationAck
*/
typedef enum DataAck
{
	DATA_ACK=1,
	DATA_NAK
} DataAck;

/* How to route a DATA packet
 */
typedef enum DataRouteEnum
{
	DATA_ROUTE_AMBIENT=0,		/* route using the ambient routing protocol  */
	DATA_ROUTE_HIERARCHY,		/* not implemented   route along the hierarchy */
	DATA_ROUTE_FLOOD,		/* route by flooding (at application layer)  */
	DATA_ROUTE_ROUTING,		/* route using the routing module (at application layer)  */
} DataRouteEnum;

#define DATA_ROUTE_NOFAILOVER 0x40000000

#define DATA_ROUTE_MASK	0x000000FF

/* A DataRoute is one of DataRouteEnum or'ed with one or more DATA_ROUTE macros
 */
typedef unsigned int DataRoute;

/* How to ack a DATA packet
 */
typedef enum DataAckType
{
	DATA_ACK_NONE,			/* no ACK packets will be sent (and no retransmissions)  */
	DATA_ACK_ENDTOEND,		/* final destinations will send ACKs  */
} DataAckType;

/* This is the header which the data module appends to a data packet for transmission
 * it will be type PACKET_DATA_DATA
 *
 * Note that unlike most modules, we are doing a marshal/unmarshal function, so
 * destinationList is carried correctly.
 */
typedef struct PacketData
{
	ManetAddr *destinationList;
	DataAck *destinationAck;
	int destinationCount;

	int len;                /* length of the marshaled packetData.  filled in by packetDataUnmarshal.  */
	unsigned int id;        /* assigned by data module.   */
	int origtype;           /* type of encapsulated packet */
	int xmitnum;		/* the number of this transmission attempt.  first packet is 0  */
	DataRoute routetype;
	DataAckType acktype;
} PacketData;

/* returns NULL on failure */
PacketData *packetDataUnmarshal(const packet *p);

/* This is the packet sent back when the data module ACKs a PACKET_DATA_DATA.
 * It will be type PACKET_DATA_ACK
 */
typedef struct PacketDataAck
{
	unsigned int id;
} PacketDataAck;

PacketDataAck *packetDataAckUnmarshal(const packet *p);


/* This is the packet sent to the calling module by the data module when its
 * ACKing the whole thing.  It will be type p->type + 1.
 *
 * There is a marshal/unmarshal function to convert the packet to this data
 * type, so the pointers to arrays of ManetAddrs are properly handled.
 */
typedef struct DataPacketAck
{
	unsigned int id;
	ManetAddr *destinationList;
	DataAck *destinationAck;
	int destinationCount;
} DataPacketAck;

/* unmarshal function for packetAck structure.
 * caller gets ownership of memory pointed to.  Caller must free  (with free())
 */
DataPacketAck *dataPacketAckUnmarshal(const packet *p);

/* Init any data structures...   */

void dataInit(manetNode *us);

/*  call to send packet p to node dest
 *
 *    routing method is determined by routing    only ambient works.
 *    ack type is determined by ack              only endtoend works.
 *    a packet ID will be returned in id.
 *
 *  The packet /WILL/ be acked, by a packet with a type of p->type + 1, and a payload with format
 *    of packetData (above).  The id from the transmission will be in the payload.  The ackflag
 *    will be either DATA_ACK (if it arrived at the remote node) or DATA_NAK (if it didn't arrive,
 *    or the ACK didn't get back here).
 *
 *    note that if the returning ACK is dropped, you will receive a DATA_NAK.
 */
void dataSend(manetNode *us, packet *p,DataRoute routing, DataAckType ack, unsigned int *id);


/* Call to multicast packet p to nodes listed in destinationList
 * Caller retains ownership of *destionationList.
 *
 * This works very similar to dataSend, only the packet will be delivered to all the destinations
 * listed in destinationList (and p->dest will be ignored!).  The ACK packet will be just like
 * that sent by dataSend, IE: a packetAck structure send to p->type + 1.
 *
 */

void dataSendMulti(manetNode *us, packet *p, ManetAddr *destinationList, int destinationCount, DataRoute routing, DataAckType ack, unsigned int *id);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_DATA */
#endif /* !DATA_H */
