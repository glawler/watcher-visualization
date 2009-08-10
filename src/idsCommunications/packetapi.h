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

#ifndef PACKET_API_H
#define PACKET_API_H

#ifdef MODULE_PACKETAPI

#include"des.h"

/* low level packet types:
*/

#define PACKET_API_ROUTE        (PACKET_API|1)
#define PACKET_API_RECEIVE      (PACKET_API|2)
#define PACKET_API_RECEIVEACK   (PACKET_API_RECEIVE+1)

#define PACKETAPI_HEADERLEN (4+1+4+1+1)

#ifdef __cplusplus
extern "C" {
#endif

/* Module Interface calls:
 */
void packetApiInit(manetNode *us);

/* This is the packet format which an API transmitted packet is contained in when
 * its going through simulator land
 */
typedef struct
{
	MessageType type;               /* The client's packet type  */
	CommunicationsDestinationType desttype;     /* current API application routing type.  */
	CommunicationsDestination origdest;
	unsigned char *payload;
	int payloadLen;
} PacketApi;

/* The HeaderMarshal and HeaderUnmarshal calls do not do the payload,
 * they are intended to be called in the livenetwork code when it is doing
 * the application routing thing, and needs to rewrite the header.
 *
 * They return 0 on success, -1 on failure.
 */
int packetApiHeaderUnmarshal(const packet *p, PacketApi *pa);
int packetApiHeaderMarshal(packet *p, const PacketApi *pa);
packet *packetApiMarshal(manetNode *us, const PacketApi *pa,int doCompression);
PacketApi *packetApiUnmarshal(const packet *p, int doCompression);

void packetApiSend(manetNode *us, ManetAddr dest, int ttl,  PacketApi *pa);   /* this takes ownership of the payload, but not the pa struct.  */


void packetApiMessageHandlerSet(
	manetNode *us,
        CommunicationsMessageDirection direction,
        unsigned int priority,
        CommunicationsMessageAccess messageAccess,
        MessageType messageType,
        MessageHandler messageHandlerFunc,
        void *messageHandlerData);


void watcherNodeColor(manetNode *us, ManetAddr node, unsigned char *color);

MessageInfoPtr packetApiMessageInfoCreate(
    manetNode *us,
    MessageType messageType,
    CommunicationsDestination destination,
    MessageStatusHandler messageStatusHandler,
    void *messageStatusHandlerData);

void packetApiMessageInfoSend(MessageInfoPtr *mi);

CommunicationsPositionWeight *packetApiPositionWeightSearchList(manetNode *us, CommunicationsPositionWeight *key);
void packetApiPositionWeightLoad(manetNode *us, const char *filename);

#ifdef __cplusplus
}
#endif

#endif
#endif
