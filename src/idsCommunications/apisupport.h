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

#ifndef APISUPPORT
#define APISUPPORT

#include "idsCommunications.h"
#include "des.h"

/* Private functions for the API
 *
 * These are to be used by the demon, and by the API implementation, and are not to be
 * exported to clients.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Conveneince function to get the current unix time in
 * milliseconds
 */
destime getMilliTime(void);

CommunicationsMessageType *communicationsMessageTypeSearch(MessageType type);


#define API_DEFAULTPORT 4837      /* Tommy's phone number from Herndon (no longer valid) (703 886 4837) */
#define API_VERSION 0x11	  /* protocol version, change when code is modified.  (checked on the
					wire between demons, and between demons and clients.)  */

/* MessageIDs are tags placed on messages to map their ACK messages back to their callbacks
 */
typedef unsigned int MessageID;

typedef void (*CommunicationsErrorFunction)(char const *fmt, ...) __attribute__ ((format(printf, 1, 2)));

/* The API communicates with the infrastructure demon over a TCP socket.  The structures
 * which go over that TCP socket are ApiCommand structs.  an ApiCommand has a type, this
 * enum describes the defined type.
 *
 * There are a number of marshal/unmarshal functions for creating the various types of 
 * ApiCommands.
 */
typedef enum
{
	APICOMMAND_TIME_GET,			/* request the demon to send the current time */
	APICOMMAND_MESSAGE_SEND,		/* request the demon to send the following message  */
	APICOMMAND_POSITION,			/* c->d means tell the demon this client is or is not eligible for a postion */
						/* d->c means demon is sending a Position update  */
	APICOMMAND_MESSAGETYPE_REQUEST,		/* tell the demon to send this client a message type */
	APICOMMAND_MESSAGETYPE_REMOVE,		/* tell the demon to not send this client a message type */
	APICOMMAND_STATUS,			/* request the demon to send a node state report to the client (debugging command) */
						/* OR, demon is sending a node state */
	APICOMMAND_POSITIONWEIGHT,		/* client is sending a node weight update.  */
						/* OR, demon is sending a node weight update back.  */

	APICOMMAND_NEIGHBOR,			/* the demon is sending a neighbor update */
	APICOMMAND_MESSAGE_ACK,			/* demon is sending a message ACK   */
	APICOMMAND_MESSAGE_NAK,			/* demon is sending a message NAK   */
	APICOMMAND_MESSAGE_REC,			/* demon is sending an incoming message */
	APICOMMAND_MESSAGE_NEXT,		/* client is returning a message it was inspecting to the demon (possibly rewritten)  */
						/* this is identical to an APICOMMAND_MESSAGE_SEND, except for the type */
	APICOMMAND_INIT,			/* demon is sending a horde of init data */
	APICOMMAND_LINKDOWN,			/* logging event, for when the API loses connection to demon */
	APICOMMAND_LINKUP,			/* logging event, for when the API restores connection to demon */
	APICOMMAND_RAWSEND,			/* client wishes to send a raw message to the local simulatorland */
	APICOMMAND_NAMESET,			/* client wishes to set its name and key  */
	APICOMMAND_NAMEGET			/* client wishes to get list of current clients.  or demon is sending list of current clients */
} ApiCommandType;

typedef struct ApiCommand
{
	struct ApiCommand *next;

	size_t len;
	ApiCommandType type;
	MessageID tag;
	unsigned char *payload;
	unsigned char *payloadptr;

} ApiCommand;

/* 
 * Functions to manage ApiCommands.  
 * 
 * ApiCommands are typically found in linked lists.  The apicommandFree 
 * function will free an entire list of ApiCommands
 */

/* Create an apicommand:  type is one of the enum above, len is the size of the payload.
 * This is intended to be called by marshal/unmarshal functions.
 */
ApiCommand *apiCommandMalloc(ApiCommandType type,int len);

/* Read an apiCommand from a FD.  Returns NULL if the read fails.
 * returns a list of ApiCommands, which contains only one member.
 */
ApiCommand *apiCommandRead(
        int fd,
        CommunicationsErrorFunction elog);

/* Write a vector of apiCommands to an FD.  
 * If 'useSendMsg' is non-zero, the data will be sent via sendmsg() with MSG_NOSIGNAL
 * set, otherwise it will be written via writev(). 
 */
int apiCommandWriteOrSend(int fd, const ApiCommand *ac, int useSendMsg);

/* Copy a list of ApiCommands.
 */
ApiCommand *apiCommandCopy(ApiCommand *ac);

/* Free a list of Apicommands.
 */
void apiCommandFree(ApiCommand *);

/* Concatenate two lists of apiCommands.  
 * returns "new" value of first.  (IE, if you pass in first==NULL,
 * it'll return next.)
 */
ApiCommand *apiCommandConcatenate(ApiCommand *first, ApiCommand *next);


/* A MessageInfo represents a message in flight
 * They are used in both the client side of the API, and in the demon.  So there are a couple of "extra" fields
 */
typedef struct MessageInfo
{
	struct MessageInfo *next;

	struct CommunicationsState *cs;   /* used by the API, to point to its state info  */
	struct ApiSession *originApi;		/* set by demon, client which originally sent this MI  */
	struct ApiSession *chainApi;		/* set by demon, client whose pending list this MI is in   */

	MessageID tag;		/* set by API, when messageSend is called */
	MessageID demonId;	/* set by the demon */
	MessageID dataId;	/* set by data module, used by the demon.  (recycling data structure)  */

	MessageStatusHandler statusCallback;   /* used by the API, to hold the callback to call upon ACK  */
	void *statusData;

	ManetAddr origin;
	CommunicationsDestination dest;
	MessageType type;

	int apiOriginated;	/* if true, then this MI was created in the API, to go out, as opposed to */
				/* having been received by the API, and a client sending it means its a */
				/* APICOMMAND_MESSAGE_NEXT */
	int routeFlags;		/* This is used to indicate if DATA_ROUTE_NOFAILOVER should be set on xmit  */

	/*
	 * The payload is either in the same malloc block as the MessageInfo struct,
	 * so it is pointed to by payload, and payloadPtr is NULL, or the payload is in
	 * a separate malloc block, pointed to by payload, AND payloadPtr.
	 * if it is not NULL, payloadPtr will be passed to free when the MessageInfo is freed
	 * (using messageInfoDestroy)
	 */

	unsigned char *payload;
	void *payloadPtr;    
	int payloadLen;
} MessageInfo;

/* An ApiInit struct is used by the demon to send initialization information to
 * the API when it connects to the demon.   It is marshaled through an apicommand...
 *
 * The entire initialization sequence sent by the demon consists of the neighbor list,
 * in the form of a series of APICOMMAND_NEIGHBOR ApiCommands, followed by an
 * APICOMMAND_INIT.  The APICOMMAND_INIT indicates that the initialization is
 * complete, and the API may begin normal operation.
 */

typedef struct ApiInit
{
	ManetAddr localid;	/* this node's manet addr  */
	ManetAddr netmask;	/* netmask of the manet network */
	int apiVersion;
} ApiInit;

/* These are sent by a client to request to be sent messages
 */
typedef struct MessageHandlerRequest
{
	ApiCommandType request;            /* Is this request a subscribe, or a delete?   */
	CommunicationsMessageDirection direction;
	CommunicationsMessageAccess access;
	unsigned int priority;
	MessageType type;
} MessageHandlerRequest;


/************************************************************************************************
 *
 * calls to mantain a list of message type handler functions
 *
 *  These are used in the API to keep track of callback addresses, and in the demon to keep track
 *  of which clients have requested which message types.
 *
 * In the API, its a simple linked list, using the nextType pointer.  (and the functions below)
 * In the demon, the message chains are stored in them, and things are a little more complicated.
 * (see diagram in chains.fig)
 * These nodes are in three separate linked lists  
 */

typedef struct MessageTypeNode
{
        struct MessageTypeNode *nextType;	/* linked list from state->incoming or state->outgoing, pointing to lists of types */
        struct MessageTypeNode *nextClient;	/* when going down a type, this is the next client in the chain  */
	struct MessageTypeNode *nextClientType;	/* linked list from ApiSession->typeList for a client's registered msg types  */
	struct ApiSession *api;			/* ApiSession which this MessageTypeNode belongs to */
	MessageInfo *pending;			/* list of MIs which have been sent to this blocking client, and for which we are expecting */
						/* a CONSUME (really length==0) or APICOMMAND_MESSAGE_NEXT  */

	CommunicationsMessageAccess access;	/* is this a blocking or nonblocking client?   */
        MessageType type;
	unsigned int priority;
	CommunicationsMessageDirection direction;

        MessageHandler handler;		/* used in the API to hold which callback to call when this message type is received  */
        void *handlerData;
} MessageTypeNode;


MessageTypeNode *messageTypeHandlerSearch(MessageTypeNode **list, MessageType typ);
MessageTypeNode *messageTypeHandlerInsert(MessageTypeNode **list, CommunicationsMessageDirection direction, unsigned int priority, CommunicationsMessageAccess messageAccess, MessageType messageType, MessageHandler messageHandlerFunc, void *messageHandlerData, int replace);
void messageTypeHandlerDelete(MessageTypeNode **list, MessageType typ);


/************************************************************************************************
 *
 * Calls to mantain a list of position functions for which a client is eligible
 *
 * Used in the API and the demon.
 */

IDSPosition *idsPositionInsert(IDSPosition **list, IDSPositionType position, IDSPositionStatus eligibility, IDSPositionUpdateProc update, void *data);
void idsPositionDelete(IDSPosition **list, IDSPositionType position);
IDSPosition *idsPositionSearch(IDSPosition **list,IDSPositionType position);


/************************************************************************************************
 *
 * Calls to mantain a list of nodes, positions, and weights for those positions.
 * (to assign roles to nodes.)
 *
 * Used in the API and the demon.
 */

CommunicationsPositionWeight *communicationsPositionWeightUnmarshal(const ApiCommand *ac);
ApiCommand *communicationsPositionWeightMarshal(const CommunicationsPositionWeight *cpw);

void communicationsPositionWeightRemove(CommunicationsPositionWeight **list,const CommunicationsPositionWeight *entry);
CommunicationsPositionWeight *communicationsPositionWeightInsert(CommunicationsPositionWeight **list,const CommunicationsPositionWeight *entry);

void communicationsPositionWeightLoad(char const *weightfile,CommunicationsPositionWeight **list);


/************************************************************************************************
 *
 * Calls to do neighbor update callbacks
 */

/* Given a CommunicationsNeighbor struct, updates a neighborlist with it
 */
void communicationsIntNeighborUpdate(CommunicationsNeighbor **neighborlist, CommunicationsNeighbor *n);

/* Given an address, searches a neighborlist for it
 */
CommunicationsNeighbor *communicationsIntNeighborSearch(CommunicationsNeighbor **neighborlist, ManetAddr addr);

/* Convenience function for mallocing CommunicationsNeighbor structs
 */
CommunicationsNeighbor *communicationsIntNeighborMalloc(ManetAddr addr,CommunicationsNeighborType type, int distance);

/* Function to free the entire list, for purposes of initialization
 */
void communicationsIntNeighborFlush(CommunicationsNeighbor **list);



/************************************************************************************************
 *
 * Functions to handle a list of node position and weights
 */



/************************************************************************************************
 *
 * Functions to marshal and unmarshal ApiCommands
 */

ApiCommand *messageInfoMarshal(const MessageInfo *mi);
/* The resulting MessageInfo will point into the ApiCommand
 * It will thus save a copy of the pointer to the ApiCommand, and will 
 * take care of freeing it.  THUS CALLER DOES NOT FREE the ac!
 */
MessageInfoPtr messageInfoUnmarshal(ApiCommand *ac);

ApiCommand *idsPositionMarshal(IDSPositionType pos,IDSPositionStatus positionStatus);
void idsPositionUnmarshal(const ApiCommand *ac, IDSPositionType *pos, IDSPositionStatus *roleStatus);

ApiCommand *IDSStateMarshal(const IDSState *vect);
IDSState *IDSStateUnmarshal(const packet *p);

ApiCommand *messageHandlerRequestMarshal(
	CommunicationsMessageDirection direction,
	unsigned int position,
	CommunicationsMessageAccess messageAccess,
	MessageType messageType);
MessageHandlerRequest *messageHandlerRequestUnmarshal(const ApiCommand *ac);

ApiCommand *communicationsNeighborMarshal(const CommunicationsNeighbor *n);
CommunicationsNeighbor *communicationsNeighborUnmarshal(const ApiCommand *ac);

ApiCommand *apiInitMarshal(const ApiInit *in);
ApiInit *apiInitUnmarshal(const ApiCommand *ac);

ApiCommand *apiStatusMarshal(const ApiStatus *us);
ApiStatus *apiStatusUnmarshal(const ApiCommand *ac);

ApiCommand *apiNameMarshal(const ApiName *name);
ApiName *apiNameUnmarshal(const ApiCommand *ac);
/* Call this to free the results of apiNameUnmarshal
 */
void apiNameFree(ApiName *an);


typedef struct CommunicationsState
{
	int fd;				/* file descriptor to demon process */
	CommunicationsLogStatePtr readLog; /* non-NULL if reading from a log instead of talking to a daemon */
	int linkup;			/* true if we have current telemetry.  (separate from fd for playback support) */

	ManetAddr host;			/* Address that we connect to the demon with */
	ManetAddr localid;		/* our manet address (generally not the same as host) */
	ManetAddr localmask;		/* manet network netmask */

	CommunicationsErrorFunction elog; /* error log */
	CommunicationsErrorFunction wlog; /* warning log */
	CommunicationsErrorFunction dlog; /* debug log */

	struct IDSPosition *positionList;			/* list of positions we are eligible for, and callbacks */
	CommunicationsPositionWeight *positionWeightList;	/* list of positions, and weights */
	struct MessageTypeNode *typehandler;			/* list of message types we are subscribed to, and callbacks */
	CommunicationsNeighborUpdateProc neighborHandler;	/* callback for neighbor events */
	void *neighborHandlerData;
	int statusPeriod;					/* minimum period for status messages (milliseconds) */
	CommunicationsStatusUpdateProc statusHandler;		/* callback for status messages */
	void *statusHandlerData;

	ApiName *name;						/* if client has set its name, this is it.  otherwise NULL */
	NameHandler nameHandler;	
	void *nameHandlerData;

	CommunicationsNeighbor *neighborList;			/* list of neighbors.  updated using neighbor events.  */

	struct MessageInfo *inflight;				/* list of outgoing messages which we are awaiting ACKs for */
	MessageID nexttag;					/* outgoing messages are tagged, to be able to recognize them on inflight */

	int initflag;						/* set after the init ApiCommand has been received.  */
	int logFD;						/* File to log apiCommands to */

	destime lastEventTime;					/* time of last received ApiCommand */
} CommunicationsState;


/* Debugging stuff, to marshal and unmarshal commands going to the watcher to
 * label nodes.
 */

unsigned char *communicationsWatcherLabelMarshal(unsigned char *hp, const NodeLabel *lab);
unsigned char *communicationsWatcherLabelUnmarshal(unsigned char *hp, NodeLabel *lab);

unsigned char *communicationsWatcherPropertyMarshal(unsigned char *hp, WatcherPropertyInfo *props);
unsigned char *communicationsWatcherPropertyUnmarshal(unsigned char *hp, WatcherPropertyInfo *props);

unsigned char *communicationsWatcherFloatingLabelMarshal(unsigned char *hp, const FloatingLabel *lab);
unsigned char *communicationsWatcherFloatingLabelUnmarshal(unsigned char *hp, FloatingLabel *lab);

unsigned char *watcherColorMarshal(unsigned char *hp, ManetAddr node, const unsigned char *color);
unsigned char *watcherColorUnMarshal(unsigned char *hp, ManetAddr *node, unsigned char *color);

/* Code for logging API events
 */

typedef struct CommunicationsLogState
{
	CommunicationsStatePtr *nodeList;
	ApiCommand *nextCmd;   /* next command to process, or NULL at EOF */
	destime nextCmdTime;   /* time of next command */
	CommunicationsStatePtr nextCmdCs;  /* cs ptr for nextCmd */
	destime curtime;	/* time of the last command processed, or some time between then an nextCmdTime if we've done communicationsLogStep() with too short a 'step' time.  */
	int fd;
	int inflight;           /* some node(s) may have inflight messages that need to be failed */
} CommunicationsLogState;

ApiCommand *communicationsLogApiCommandRead(int fd, destime *tim, ManetAddr *localid);
int communicationsLogApiCommandWrite(int fd, ApiCommand *ac, destime tim, ManetAddr localid);

#ifndef ZLIB_FREE_FUNC
#define ZLIB_FREE_FUNC free_func
// #define ZLIB_FREE_FUNC zfree_func
#endif

#ifdef __cplusplus
};
#endif


#endif
