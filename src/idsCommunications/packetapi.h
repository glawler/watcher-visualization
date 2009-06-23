#ifndef PACKET_API_H
#define PACKET_API_H

#ifdef MODULE_PACKETAPI

#include"des.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  All rights reserved.
 */

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
