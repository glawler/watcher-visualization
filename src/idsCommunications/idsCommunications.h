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

/*
 * idsCommunications.h - defines methods for communicating with other
 * ids nodes.
 */

#ifndef IDSCOMMUNICATIONS_H_FILE
#define IDSCOMMUNICATIONS_H_FILE

#include <stdint.h>
#include <time.h>   /* for time_t */
#include <unistd.h> /* for size_t */
#include <limits.h> /* for INT_MIN / INT_MAX */

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifndef COMMUNICATIONS_CONFIG_DIR
#define COMMUNICATIONS_CONFIG_DIR "./"
#endif

/* This API has support for xml message payloads, generated and parsed
 * using libxml2.
 */

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

/* A message type is currently unsigned int.  Some message type values
 * are defined in idsCommunicationsMessages.h, but they are opaque
 * to this API.
 */

typedef unsigned int MessageType;

#include "idsCommunicationsMessages.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 *
 * To interface with client:
 *
 *  The API assumes that the client is running a select loop.  At the
 *  start of the select loop, the client can call
 *  communicationsReturnFD() to get a FD to give to its select loop
 *  (for read).  When it is readable, the client calls
 *  CommunicationsReadReady(), which will then call callbacks.  The
 *  file descriptor may change (since the API may attempt to reconnect
 *  to the daemon).  Thus communicationsReturnFD() must be called
 *  before each select.  If communicationsReturnFD() returns -1, it
 *  indicates that the API failed to get to the daemon.  In that case,
 *  the client should call communicationsReadReady() when it wants to
 *  retry the connection.  (5 second delay?)
 *
 * Callbacks are only called in communicationsReadReady()...  if
 * messageInfoSend() detects the daemon has been lost, it acts as if
 * the write succeeded.  When communicationsReadReady() is called, the
 * status callback for the message will then be called, and indicate
 * the error.
 *
 *  If the client is not running a select loop, it may call 
 *  CommunicationsReadReady, which will block until something happens,
 *  call one or more callbacks, and then return.  It may also return if the
 *  connection to the daemon is lost.  It may be re-called immediately, but that
 *  may cause a buzz loop.  
 */


/* A ManetAddr is actually an IPv4 address right now.
 * They are in host byte order, NOT network byte order.
 */
#ifndef MANETADDR_DEFINED
#define MANETADDR_DEFINED
typedef unsigned int ManetAddr;
#endif /* MANETADDR_DEFINED */

#ifndef MANETTIME_DEFINED
#define MANETTIME_DEFINED
typedef long long int destime;
#endif

/* Predefined node addresses:
 */
#define NODE_LOCAL	(0x7f000001)
#define NODE_ALL	(0xFFFFFFFF)
#define NODE_IS_MULTICAST(m) (((m) & 0xf0000000) == 0xe0000000)


#define MESSAGE_MAXTTL (255)


/* The Hierarchy can application-route messages.  The struct
 * HierarchyDestination is used to specify how to route.
 *
 * When specifying a destination, one can use the NODE_ macros above.
 *  For example, a node's parent node would be addr=NODE_LOCAL type=parentsof
 *
 * The ttl field is the maximum number of hops the message may traverse before
 * getting dropped.  For most purposes, one wants to use MESSAGE_MAXTTL.  For
 * "remote" modes, where the packet is unicast to some other node, and then
 * forwarded, the TTL is used when the message is being forwarded at the remote
 * node, not for the initial unicast step.
 */
typedef enum
{
    /* send the message directly to the node specified by addr  */
    COMMUNICATIONSDESTINATION_DIRECT=0,

    /* send the message directly to the node specified by addr, 
     * using the backup routing algorithm */
    COMMUNICATIONSDESTINATION_DIRECTBACKUP,

    /* flood the message to every node (EXPENSIVE) */
    COMMUNICATIONSDESTINATION_BROADCAST,

    /* unicast message to the node specified by addr, 
     * which then forwards the message to its neighbors 
     * UNIMPLEMENTED. */
    COMMUNICATIONSDESTINATION_NEIGHBORSOF,

    /* unicast message to the node specified by addr, 
     * which then forwards the message to its children.
     * (only addr=NODE_LOCAL is implemented) */
    COMMUNICATIONSDESTINATION_CHILDRENOF,

    /* unicast message to the node specified by addr,
     * which then forwards it to its children, 
     * which rewrite addr to be their addr, and 
     * forward to their own children. 
     * UNIMPLEMENTED. */
    COMMUNICATIONSDESTINATION_RECURSIVECHILDRENOF,

    /* unicast message to the node specified by addr,
     * which then forwards the message to its parents
     * (only addr=NODE_LOCAL is implemented) */
    COMMUNICATIONSDESTINATION_PARENTSOF,

    /* unicast message to the node specified by addr,
     * which then forwards the message to its parents,
     * which rewrite addr to be their addr, and forward
     * to their own parents.
     * UNIMPLEMENTED. */
    COMMUNICATIONSDESTINATION_RECURSIVEPARENTSOF,

    /* Send to the given node if it's a coordinator, 
     * else send to its parent.
     * (only addr=NODE_LOCAL is implemented) */
    COMMUNICATIONSDESTINATION_NEARESTCOORD,

    /* multicast the message to the given group */
    COMMUNICATIONSDESTINATION_MULTICAST,

} CommunicationsDestinationType;

#define COMMUNICATIONSDESTINATION_IS_VALID(d) \
   (((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_DIRECT || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_DIRECTBACKUP || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_BROADCAST|| \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_NEIGHBORSOF || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_CHILDRENOF || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_RECURSIVECHILDRENOF || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_PARENTSOF || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_RECURSIVEPARENTSOF || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_NEARESTCOORD || \
    ((CommunicationsDestinationType)(d)) == COMMUNICATIONSDESTINATION_MULTICAST)

#if 1
/* temporary backwards compatibility definitions.  -dkindred 2007-07-15 */
#define DIRECT       COMMUNICATIONSDESTINATION_DIRECT
#define DIRECTBACKUP COMMUNICATIONSDESTINATION_DIRECTBACKUP
#define BROADCAST    COMMUNICATIONSDESTINATION_BROADCAST
#define CHILDRENOF   COMMUNICATIONSDESTINATION_CHILDRENOF
#define PARENTSOF    COMMUNICATIONSDESTINATION_PARENTSOF
#define NEARESTCOORD COMMUNICATIONSDESTINATION_NEARESTCOORD
#endif

typedef struct CommunicationsDestination
{
    ManetAddr addr;
    CommunicationsDestinationType type;
    int ttl;
} CommunicationsDestination;


typedef struct MessageInfo *MessageInfoPtr;
typedef struct CommunicationsState *CommunicationsStatePtr;


/*
 * Initialize the communication system.
 *
 * logProc      - called to log a message somewhere
 *
 * host         - IP addr of machine where the API to talk to is running.  
 *                Set to 0 for local host.  (The API can connect to a
 *                remote daemon, BUT only for debugging, and that should be
 *                disabled in deployment.)
 *
 * Returns non-NULL on success.
 */
CommunicationsStatePtr communicationsInit(ManetAddr host);


/* Client Name stuff
 *  Allows a client to set its name, and retrieve the names of the other 
 *  connected clients.
 *
 * This struct is used both for the client setting its name, and the daemon 
 * reporting the current set of clients (thus the message count fields).
 */
typedef struct ApiName
{
    char *name;             /* This client's name  */
    char *key;              /* This client's key  */
    /* client traffic counters */
    int messagesSent, messagesAcked, messagesRec, messagesUnacked;
    struct ApiName *next;
} ApiName;

/* Set the name of the client, so the daemon can identify which
 * connection is which.  Intended primarily to simplify debugging
 * but the key may serve an access control function in the future.
 *
 * Makes a copy of name and key.
 */

void communicationsNameSet(CommunicationsStatePtr cs, const char *name, 
                           const char *key);

/* Send a request to receive the current list of clients.
 */
void communicationsNameGet(CommunicationsStatePtr cs);

typedef void (*NameHandler)(void *nameHandlerData, const struct ApiName *list);

/* Call to set a handler for incoming client lists.  (Call this before calling 
 * communicationsNameGet().
 * (Might be nice to also support registering to get updates to this list as
 * clients arrive or depart.)
 */
void communicationsNameHandlerSet(CommunicationsStatePtr cs, 
                                  void *nameHandlerData, NameHandler cb);



/*
 * Set up logging functions. The defaults for all communications can be
 * set. Once you have a CommunicationsStatePtr, you can modify the
 * logging for that particular communications state.
 */
void communicationsLogDebugDefaultSet(void (*dlog)(char const *fmt, ...));
void communicationsLogWarnDefaultSet(void (*wlog)(char const *fmt, ...));
void communicationsLogErrorDefaultSet(void (*elog)(char const *fmt, ...));
void communicationsLogDebugSet(
        CommunicationsStatePtr cs, void (*dlog)(char const *fmt, ...));
void communicationsLogWarnSet(
        CommunicationsStatePtr cs, void (*wlog)(char const *fmt, ...));
void communicationsLogErrorSet(
        CommunicationsStatePtr cs, void (*elog)(char const *fmt, ...));

/* CommunicationsReturnFD() returns a file descriptor for a select loop.
 *  The API will need attention when that file descriptor indicates
 *  it is ready to be read.
 *
 * If the API loses its connection to the daemon, this function will
 * return -1.  In that event, call communicationsReadReady() regularly,
 * and the API will attempt to reconnect.  communicationsReturnFD will
 * return a valid FD when the retry succeeds.  
 *
 * Returns -1 if cs is reading events from a log rather than communicating
 * with a daemon.
 *
 * returns non-negative on success
 */
int communicationsReturnFD(CommunicationsStatePtr cs);

/* Returns true if this API instance is alive
 */
int communicationsLinkup(CommunicationsStatePtr cs);

/*  When the FD returned by CommunicatsionsReturnFD() is readable, the
 *  client calls this function.  It will then read whatever happened,
 *  call one or more callbacks, and return.
 *  
 */
int communicationsReadReady(CommunicationsStatePtr cs);

/* Abandon all in-flight messages, and close connection to the API
 */
void communicationsClose(CommunicationsStatePtr cs);

/* Return the "current time".  This uses the system clock if we are
 * connected to a real daemon, or the "virtual" time value (same as
 * communicationsLogTimeGet()) if we are reading from a log.
 */
/* OBSOLETE: time_t communicationsTimeGet(CommunicationsStatePtr cs); */
destime communicationsDestimeGet(CommunicationsStatePtr cs);
void communicationsTimevalGet(CommunicationsStatePtr cs, struct timeval *tv);

/* Return the time of the most recent API event (received message or ack,
 * position update, etc.).
 *
 * This is useful in callback functions for getting "current time" without
 * making another system call.  Another advantage is that when reading from 
 * a log (via communicationsLogLoad()) rather than talking to a daemon,
 * this function will return the recorded time from the log file, so you
 * can play the log back at full speed without losing the timing data.
 */
destime communicationsLastEventTimeGet(CommunicationsStatePtr cs);


/******************************************************************************
 *
 * Calls to receive messages:
 *
 * To receive messages, a client will initialize the API, and then
 * register one or more MessageHandler callback functions, for specific
 * message types.  When messages of that type arrive, the API will call
 * the client's callbacks, and pass it MessageInfo structs for the messages.
 * The client will then do whatever it wishes with those messages, and
 * free the MessageInfo structs.
 *
 */

typedef enum
{
    COMMUNICATIONS_MESSAGE_INBOUND=0,
    COMMUNICATIONS_MESSAGE_OUTBOUND,
} CommunicationsMessageDirection;

typedef enum
{
    COMMUNICATIONS_MESSAGE_READONLY=0,
    COMMUNICATIONS_MESSAGE_READWRITE,
} CommunicationsMessageAccess;

/* Predefined values for priority 
 */
#define COMMUNICATIONS_MESSAGE_BEFOREALL 0
#define COMMUNICATIONS_MESSAGE_AFTERALL 0xFFFFFFFF

/*
 *  When a message is received by the API, it is delivered to the 
 *  client using a MessageHandler Callback.
 *
 */

typedef void (*MessageHandler)(void *messageHandlerData, 
                               const struct MessageInfo * messageInfo);
typedef void (*MessageHandlerReadWrite)(void *messageHandlerData, 
                                        struct MessageInfo * messageInfo);

/*
 * Register a message handler for a given type of IDS message.
 *
 * priority - Where in the chain of detectors (for this specific message type)
 * does this detector need to be?  The lower the number the closer to reception
 * the detector is.
 *
 * access - Is this detector going to be rewriting messages?
 *
 * messageType - type of message the handler will be called for.
 *
 * messageHandlerFunc - procedure called when a message of the given type
 *                  arrives.  If access is READWRITE, it it safe to
 *                  pass a MessageHandlerReadWrite (cast to MessageHandler).
 *
 * messageHandlerData - passed into the "messageHandler()" procedure
 *                      whenever it is called.
 *
 * Returns non-zero on success.
 *
 */
int messageHandlerSet(
        CommunicationsStatePtr cs,
        CommunicationsMessageDirection direction,
        unsigned int priority,
        CommunicationsMessageAccess messageAccess,
        MessageType messageType,
        MessageHandler messageHandlerFunc,
        void *messageHandlerData);



/******************************************************************************
 *
 * Calls to send messages:
 *
 *  To send a message a client does the following:
 *    creates a MessageInfo struct using messageInfoCreate
 *       (This specifies a message type, destination, Status callback, and 
 *       Status data.)
 *       The most common message destination is a node's parent, which is a
 *       CommunicationsDestination struct with the value addr=NODE_LOCAL, 
 *       type=PARENTSOF.
 *    assigns a payload to the message using messageInfoPayloadSet() or
 *       messageInfoRawPayloadSet()
 *    transmits the message using messageInfoSend()
 *
 *    The API then takes ownership of the MessageInfo, and attempts to 
 *    transmit the message.
 *    When it succeeds or gives up, it will call the Status callback given 
 *    in the messageInfoCreate call.
 *    In the status callback, the API returns ownership of the MessageInfo 
 *    struct, and it may be freed.
 *
 *    [XXX What about messages which have been sent to multiple nodes, e.g.,
 *    using CHILDRENOF?  IIRC they generate a single ack callback on successful
 *    delivery to all destinations, and a single nack otherwise. -dkindred]
 *
 *    Reusing MessageInfo structs is not recomended.
 *
 * XML messages can be processed using libxml with messageInfoPayloadSet() and
 * messageInfoPayloadGet().  Other messages can be processed with 
 * messageInfoRawPayloadSet() and messageInfoRawPayloadGet(), which handle
 * arbitrary byte sequences.  All messages are compressed during transport
 * using an IDMEF-derived zlib dictionary -- see messageTypeList in 
 * apisupport.c).
 *
 * If you wish to send hand-generated XML, pass it as a string to
 * messageInfoRawPayloadSet, with the length not including the NUL
 * terminator.  If it is syntatically correct, it will be readable
 * using messageInfoGetPayload on the far side.  If you wish to parse
 * the XML yourself, read it using messageInfoRawPayloadSet, and keep
 * in mind that the string is NOT NUL terminated.
 *  
 */

/*
 *  When a messageInfo is created, a status callback is specified.
 *  that callback will then be called after the message is transmitted,
 *  to indicate if the transmission is successful.
 */

/* Enum to indicate success of message transmission:
 */
typedef enum 
{
    MESSAGE_SUCCESSFUL=0,       /* The message was delivered and ACKed  */
    MESSAGE_THE_CATS_EATEN_IT,  /* not used */
    MESSAGE_FAILED,
    /* other status codes may be added later -- 
     * be sure to update MESSAGE_STATUS_TEXT() below */
} MessageStatus;

#define MESSAGE_STATUS_TEXT(status) \
    ((status) == MESSAGE_SUCCESSFUL ? "Success" : \
     (status) == MESSAGE_THE_CATS_EATEN_IT ? "Cat Ate" : \
     (status) == MESSAGE_FAILED ? "Failed" : "Unknown")

/* Status callback function typedef
 */
typedef void (*MessageStatusHandler)(
    const struct MessageInfo* messageInfo,  /* the messageInfo struct that
                                             * was sent  */
    void *messageStatusHandlerData,         /* the void pointer passed into
                                             * messageInfoCreate(), when the
                                             * messageInfo was created */
    MessageStatus statusNum);               /* whether the message was
                                             * delivered successfully */

/*
 * Create a MessageInfo structure.
 *
 * messageType - the type of the message to be sent
 *
 * destination - the node, or group of nodes, to send the message to
 *
 * messageStatusHandler - called when a packet is either ACKed or given up on.
 *
 * messageStatusHandlerData - passed into "messageStatusHandler()" when it 
 *                            is called
 */
MessageInfoPtr messageInfoCreate(
        CommunicationsStatePtr cs,
        MessageType messageType,
        CommunicationsDestination destination,
        MessageStatusHandler messageStatusHandler,
        void *messageStatusHandlerData);

/*  Assigns an XML document to be the payload of an outgoing message.
 *  The XML document will be converted into a byte representation, compressed, 
 *  and copied into the MessageInfo struct.
 *
 *  The xmlDoc pointed to by payload will be marshaled into a 
 *  API-malloced buffer.  The API does not claim ownership of it.
 */
void messageInfoPayloadSet(MessageInfoPtr messageInfo, const xmlDoc *payload);

/* If one does not wish to use XML, this call will take a void pointer and
 * length, and specify it as payload.
 *
 * The API takes ownership of block of memory pointed to by payload.  
 * The pointer will be passed to free() when the messageInfo structure 
 * is destroyed.
 */
void messageInfoRawPayloadSet(MessageInfoPtr messageInfo, 
                              void *payload, int len);


/*  Send a given messageInfo struct
 *
 *    The API then takes ownership of the MessageInfo, and attempts to 
 *    transmit the message.
 *    When it succeeds or gives up, it will call the Status callback 
 *    given in the messageInfoCreate call.
 *
 */
void messageInfoSend(MessageInfoPtr messageInfo);




/******************************************************************************
 *
 * Calls to inspect and free MessageInfo structures.
 * These may be used on either transmitted or received MessageInfo structs
 */

/*
 * Destroy a MessageInfo structure.
 */
void messageInfoDestroy(MessageInfoPtr messageInfo);

/*
 * Returns the originator of a message
 */
ManetAddr messageInfoOriginatorGet(const struct MessageInfo *messageInfo);

/*
 * Returns the type of a message
 */
MessageType messageInfoTypeGet(const struct MessageInfo *messageInfo);

/* Returns the CommunicationsState of a message
 * (handy for sending new messages in a message receive callback)
 */

CommunicationsStatePtr messageInfoCommunicationsStateGet(
    const struct MessageInfo *messageInfo);

/*
 * Returns the destination of a message
 */
CommunicationsDestination messageInfoDestinationGet(
    const struct MessageInfo *messageInfo);

/* Returns the XML document payload of a message.
 *
 * The xmlDocPtr returned by this points to a copy, which the caller
 * must free, using xmlFreeDoc().
 */
xmlDocPtr messageInfoPayloadGet(const struct MessageInfo *messageInfo);

/* If one is not using XML, this will return a pointer to an API owned
 * block of memory containing the payload, which is valid until the 
 * MessageInfo struct is destroyed.  (The length of the payload
 * is returned by messageInfoRawPayloadLenGet().)
 */
void *messageInfoRawPayloadGet(const struct MessageInfo *messageInfo);

/* Returns the length of the payload representation.
 */
size_t messageInfoRawPayloadLenGet(const struct MessageInfo *messageInfo);



/******************************************************************************
 *
 *  Hierarchy Geometry calls
 *   Used to examine the geometry of the hierarchy.
 *
 */

/*
 * Enumerates possible IDS hierarchy positions.
 *
 *  COORDINATOR_ROOT active indicates that the node is a root node.
 *  COORDINATOR_NEIGHBORHOOD active indicates that the node is one level 
 *      above a leaf node (has no "grandchildren")
 *  COORDINATOR_REGIONAL active indicates that the node is "somewhere in
 *      the middle" (has grandchildren but is not root)
 *
 *  A node that is not an active ROOT/NEIGHBORHOOD/REGIONAL coordinator
 *  is a leaf node.
 *
 *  There may be several layers of Regional Coordinators.  They can have 
 *  children that are Regional Coordinators, as well as parents that are 
 *  Regional Coordinators.
 *
 * COORDINATOR_MAXVAL is used to determinate the size of an array. 
 * Make sure its value is greater than any of the other COORDINATOR_* values.
 */
typedef enum IDSPositionType
{
    COORDINATOR_NEIGHBORHOOD=0,
    COORDINATOR_REGIONAL,
    COORDINATOR_ROOT,
#define COORDINATOR_MAXVAL 4
} IDSPositionType;

/*
 * Indicates whether a node is assigned to a particular hierarchy position 
 * or not.  Note that the API supports a node having more than one position
 * at once.
 */
typedef enum IDSPositionStatus
{
    IDSPOSITION_INACTIVE=0,
    IDSPOSITION_UNDEFINED,     /* internal value, to make sure that an update 
                                * is sent immediately.  If seen by a caller, 
                                * there is a bug. */
    IDSPOSITION_INFORM,
    IDSPOSITION_ACTIVE,
#define IDSPOSITION_MAXVAL (IDSPOSITION_ACTIVE+1)
} IDSPositionStatus;

/*
 * Callback used by the communications module to notify a client of a position
 * change.
 */
typedef void (*IDSPositionUpdateProc)(
        void *idsPositionUpdateProcData,
        IDSPositionType position,
        IDSPositionStatus positionStatus);

/* Struct returned to describe the list of current IDSPositions
 */
typedef struct IDSPosition
{
    struct IDSPosition *next;

    IDSPositionType position;
    IDSPositionStatus eligibility;
#define eligability eligibility /* backwards compatibility */
    IDSPositionStatus status;
    IDSPositionUpdateProc update;
    void *updateData;
} IDSPosition;

/*
 * If the Hierarchy assigns or removes this node from a position, the 
 * PositionUpdateProc callback will be called, with the void pointer, and
 * an IDSPositionStatus indicating if the node is assigned to the position
 * or not.
 *
 * position               - position to register for
 *
 * eligibility	          - Indicates if this client just wishes to be 
 *                          informed of position changes (IDSPOSITION_INFORM),
 *                          or is offering to actually serve in a position
 *                          (IDSPOSITION_ACTIVE).
 *
 * positionUpdateProc     - called when position status changes
 *
 * positionUpdateProcData - passed into "positionUpdateProc()" whenever it is
 *                          called.
 *
 * To indicate inelegibility, call idsPositionRegister with 
 * eligibility=IDSPOSITION_INACTIVE
 *
 * Returns non-zero on success.
 */
int idsPositionRegister(
        CommunicationsStatePtr cs,
        IDSPositionType position,
        IDSPositionStatus eligibility,
        IDSPositionUpdateProc idsPositionUpdateProc,
        void *idsPositionUpdateProcData);

/* This function will return a linked list of the
 * IDSPositions which this node has claimed eligibility for, and
 * if they are active or not.  
 *
 * The memory returned is owned by the API.  Caller may not modify or free it.
 */
IDSPosition *idsPositionCurrent( CommunicationsStatePtr cs);

/* To set the arrangement of the nodes to a specific configuration, create a
 * "state vector", consisting of a list of nodes with the parent node which
 * each that node should use. Then, pass that vector to IDSStateSet, and it will
 * be flood-routed to all the nodes, and immediately adopted.
 * The clustering algorithm will then run from there.  (It may immediately 
 * change things too, so try to make sure the set configuration is a valid
 * one for the clustering algorithm, or use the "justneighbors" clustering
 * algorithm.)
 */

typedef struct
{
    ManetAddr node;
    ManetAddr parent;
    /* Clustering-algorithm-specific data.
     * Don't go overboard; the entire IDSState vector must fit in a packet.
     * This will be freed by IDSStateFree() using free().
     */
    void *clusteringData;
    unsigned short clusteringDataLen;
} IDSStateElement;

typedef struct 
{
    unsigned int numNodes;
    CommunicationsStatePtr cs;
    IDSStateElement *state;
    unsigned int noflood;
    unsigned int lock;		/* after applying new state, lock in position */
} IDSState;

IDSState *IDSStateMalloc(CommunicationsStatePtr cs, int num);
void IDSStateFree(IDSState *vect);
void IDSStateSet(IDSState *vect);

/******************************************************************************
 *
 * Neighbor List calls:
 * the neighbor list includes one hop neighbors, and multi-hop neighbors with
 * which the node has links (IE: parents, children.).  Look at the distance 
 * field to tell them apart.  One hop neighbors are distance==1.  The neighbor
 * list may be stale, but will be updated as best as the clustering algorithm
 * can do so.
 *
 * To get only one hop neighbor arrivals and departures, look at the
 * distance and WASONEHOP bit.
 *
 * arrival with distance==1 is a one hop arrival
 * update with distance>1 WASONEHOP set is a one hop departure
 * update with distance==1 WASONEHOP clear is a one hop arrival
 * departure with WASONEHOP set is a one hop departure
 *
 * Leaf nodes will be 1 hop from their Coordinators.  Coordinators
 * will probably be more than one hop from their Coordinators.
 *
 * XXX: How do we want to do history?  IE: a node's previous
 * positions, and a node's previous parents?
 *
 * XXX: Do we need to tell child leaf nodes from child coordinator nodes?
 */

/* A neighboring node is one of this list of types
 */
typedef unsigned int CommunicationsNeighborType;

#define COMMUNICATIONSNEIGHBOR_UNKNOWN      0
#define COMMUNICATIONSNEIGHBOR_PARENT       0x001
#define COMMUNICATIONSNEIGHBOR_CHILD        0x002
#define COMMUNICATIONSNEIGHBOR_ROOT         0x004
#define COMMUNICATIONSNEIGHBOR_WASONEHOP    0x008



/* When the Neighbor Callback is called, it indicates that the
 * neighbor is one of this list of states.
 */
typedef enum
{
    COMMUNICATIONSNEIGHBOR_ARRIVING,
    COMMUNICATIONSNEIGHBOR_UPDATING,
    COMMUNICATIONSNEIGHBOR_DEPARTING,
} CommunicationsNeighborState;

typedef struct CommunicationsNeighbor
{
    ManetAddr addr;				/* the neighbor's address */
    CommunicationsNeighborType type;	/* neighbor's type, child or parent */
    CommunicationsNeighborState state;	/* If the neighbor is present
                                         * (ARRIVING) or absent (DEPARTING) */
    int distance;              /* the neighbor's distance in hops.  
                                * 1 hop neighbors will have a distance of 1 */
    struct CommunicationsNeighbor *next; /* next pointer for the linked list */
} CommunicationsNeighbor;

/* Callback type for the callback specified by communicationsNeighborRegister
 *
 *  The *neighbor argument is read-only.
 */

typedef void (*CommunicationsNeighborUpdateProc)
    (void *communicationsNeighborUpdateProcData,	/* void pointer specified with communicationsNeighborRegister */
     CommunicationsNeighbor *neighbor);  /* RO arg describing what changed. */

/* Register a callback to be called when neighbors arrive or depart
 * from the neighbor list.
 *
 * This will include 1 hop neighbors, as well as multi-hop neighbors
 * which the node has a link to.
 *
 * IE: If the node gets a new parent, this will be called twice, once
 * with the old parent departing, and once with the new parent
 * arriving.
 *
 * Register a NULL pointer to disable the callback.
 */
void communicationsNeighborRegister(CommunicationsStatePtr cs, 
         CommunicationsNeighborUpdateProc communicationsNeighborUpdateProc, 
         void *communicationsNeighborUpdateProcData);

/* Return the address of this node.  
 */
ManetAddr communicationsNodeAddress(CommunicationsStatePtr cs);

/* Return the netmask of the manet interface of this node
 */
ManetAddr communicationsNodeMask(CommunicationsStatePtr cs);

/* Returns a linked list of the current set of neighbors which this node has.
 * 
 *  neighbors include all the 1 hop neighbors, and multi-hop neighbors which
 *  the node has a link to, IE: parents and children.  
 *
 * The memory pointed to is owned by the API.  The client may not modify it.
 */
CommunicationsNeighbor *communicationsNeighborList(CommunicationsStatePtr cs);

/* Search the neighbor list for a given address, and return the
 * CommunicationsNeighbor struct for that neighbor.  This is handy for
 * determining if an incoming message is from a child node.
 *
 * The memory pointed to is owned by the API.  The client may not modify it.
 */
CommunicationsNeighbor *communicationsNeighborSearch(CommunicationsStatePtr cs,ManetAddr neighbor);


/******************************************************************************
 *
 * Calls to get and set node position weights for the Hierarchy building algorithm.
 *
 * The intention is that an administrator may want to assign nodes to
 * hierarchy positions, or bias the hierarchy algorithm's choices.
 * This is done using a list of (node, position, weight) tuples.  A
 * node with a position weight of 0xFFFFFFFF is assigned to that
 * position, while a node with a position weight of 0 is banned from
 * that position.
 *
 * The list of weights is input into the hierarchy algorithm, but may
 * be ignored for now.
 *
 * It is assumed that the list will be distributed to the nodes by
 * some policy messages and that there will exist a client which will
 * register for those policy messages, and then call these API entry
 * points as the policy messages are distributed.  All the nodes will
 * need to have the list individually specified, the API does not
 * distribute it.
 *
 * This section of the API is still open to debate.
 */

typedef struct CommunicationsPositionWeight
{
    ManetAddr addr;
    IDSPositionType position;
    /* weight: higher value is more favored.  
     * see COMMUNICATIONSPOSITIONWEIGHT_{BANNED,DEFAULT,ASSIGNED} macros below.
     */
    int weight;

    struct CommunicationsPositionWeight *next;
} CommunicationsPositionWeight;

#define COMMUNICATIONSPOSITIONWEIGHT_BANNED   INT_MIN
#define COMMUNICATIONSPOSITIONWEIGHT_DEFAULT  0
#define COMMUNICATIONSPOSITIONWEIGHT_ASSIGNED INT_MAX

/* Return the current list of position weights.
 *
 * The memory pointed to is owned by the API.  The client may not modify it.
 */
const CommunicationsPositionWeight *communicationsPositionWeightGet(
    CommunicationsStatePtr cs);

/* Search the current list of position weights for a specific postion,
 * and return its entry.
 *
 * The memory pointed to is owned by the API.  The client may not modify it.
 */
const CommunicationsPositionWeight *communicationsPositionWeightSearch(
    CommunicationsStatePtr cs, 
    CommunicationsPositionWeight *position);

/* This is exactly like communicationsPositionWeightSearch, only
 * different signature.  It is here to allow the clustering algorithm
 * to use some of this code.  However, its a kludge, until the
 * clustering algorithm API gets cleaned up.
 *
 * Use communicationsPositionWeightSearch!
 */
CommunicationsPositionWeight *communicationsPositionWeightSearchList(
    CommunicationsPositionWeight *list, 
    const CommunicationsPositionWeight *position);

/* Add a list of weights to the current set.
 *
 * If a node already has an existing weight for a Position, it is
 * replaced with the new value.
 *
 * This merely sends a request to the daemon.  The change won't be 
 * reflected by, e.g., communicationsPositionWeightSearch() until/unless
 * the daemon echoes the change back.
 *
 * XXX: This is a privileged function, and the daemon should apply 
 * some access controls, but currently does not.
 *
 * XXX: This might be better named communicationsPositionWeightSet()
 *
 * The API will copy the argument list: the client retains ownership
 * (and thus frees it).
 * 
 */
void communicationsPositionWeightAdd(CommunicationsStatePtr cs,
                                     const CommunicationsPositionWeight *list);

/* Remove a list of weights from the current set.
 *
 * The weight field will be ignored.  If there is an entry in the
 * current list for a node and position tuple on the argument list,
 * the matching entry on the current list will be deleted.
 *
 * This merely sends a request to the daemon.  The change won't be 
 * reflected by, e.g., communicationsPositionWeightSearch() until/unless
 * the daemon echoes the change back.
 *
 * The API will copy the argument list: the client retains ownership
 * (and thus frees it).
 *
 * XXX: This is a privileged function, and should have some access controls.
 *
 * XXX: This might be better named communicationsPositionWeightDel() or
 * or ...Remove() or ...Unset()
 *
 */
void communicationsPositionWeightSub(CommunicationsStatePtr cs,
                                     CommunicationsPositionWeight *list);


/******************************************************************************
 *
 * Debugging entry points
 */

/* The ApiStatus struct is used to carry some status information which
 * is used only for debugging.  The Watcher program uses it to display
 * state information from the clustering algorithm.
 *
 * This is expected to be used by exactly one client at a time.  It
 * will work when used by more than one, but there is only a single
 * time, so the messages will be sent at the smallest period of all
 * the clients requesting.  (As well as being sent when the fields in
 * ApiStatus change.)
 */

typedef struct ApiPacketCount
{
    /* Type values are defined in des.h, as PACKET_FOO.  
     * Each module can then define PACKET_FOO_BAR packet types in
     * their own headers  */
    int type;

    long long int unicastRecCount,origUnicastXmitCount,repUnicastXmitCount;
    long long int unicastRecByte,origUnicastXmitByte, repUnicastXmitByte;

    long long int bcastRecCount,origBcastXmitCount, repBcastXmitCount;
    long long int bcastRecByte,origBcastXmitByte, repBcastXmitByte;

} ApiPacketCount;

typedef struct ApiStatus
{
    int period;			/* -1 is disabled.  0 is only when level changes, positive is a period in milliseconds.  */
    int level;
    int rootflag;
    destime timestamp;   /* timestamp, in unix time, in milliseconds */

    int numtypes;   		/* Indicates length of the array pointed to by packetList */
    ApiPacketCount *packetList;	/* array of packet types and their counts  */
} ApiStatus;

/* Callback type, for receiving ApiStatus messages.
 * 
 * API owns the memory pointed to by as.  The user may not modify, or
 * preserve pointers to between callbacks.
 */
typedef void (*CommunicationsStatusUpdateProc)(
    void *communicationsStatusUpdateProcData, 
    ApiStatus *as);

/* Register a callback to be called when the node status changes, and
 * every period milliseconds To disable, call with the callback
 * function set to NULL
 *
 */
void communicationsStatusRegister(CommunicationsStatePtr cs, 
                                  int period, 
                                  CommunicationsStatusUpdateProc communicationsStatusUpdateProc, 
                                  void *communicationsStatusUpdateProcData);

/* put a label containing the string pointed to by string on the watcher
 * display for node node.
 *
 * To remove the label, call again and pass a NULL pointer in for string.
 *
 * color is either NULL if you don't care, or a pointer to 4 chars, containing
 * an RGBA color for the label.
 */

/* Values for priority
 * (not an enum, so as to avoid fighting with types as it is
 * marshaled/unmarshaled, and making des.h dependant on this .h)
 */
#define	COMMUNICATIONS_LABEL_PRIORITY_CRITICAL	 0
#define	COMMUNICATIONS_LABEL_PRIORITY_WARN		128
#define	COMMUNICATIONS_LABEL_PRIORITY_INFO		255

#define COMMUNICATIONS_LABEL_FAMILY_ALL			0xFFFFFFFF

#define COMMUNICATIONS_LABEL_FAMILY_UNDEFINED		0
#define COMMUNICATIONS_LABEL_FAMILY_PHYSICAL		1
#define COMMUNICATIONS_LABEL_FAMILY_HIERARCHY		2
#define COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH		3
#define COMMUNICATIONS_LABEL_FAMILY_ROUTING		4
#define COMMUNICATIONS_LABEL_FAMILY_ROUTING_ONEHOP	5
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_6		6
#define COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS	6
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_7		7
#define COMMUNICATIONS_LABEL_FAMILY_SANITYCHECK		7
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_8		8
#define COMMUNICATIONS_LABEL_FAMILY_ANOMPATHS		8
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_9		9
#define COMMUNICATIONS_LABEL_FAMILY_CORRELATION		9
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_10		10
#define COMMUNICATIONS_LABEL_FAMILY_ALERT		10
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_11		11
#define COMMUNICATIONS_LABEL_FAMILY_CORRELATION_3HOP	11
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_12		12
#define COMMUNICATIONS_LABEL_FAMILY_ROUTING2		12
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_13		13
#define COMMUNICATIONS_LABEL_FAMILY_ROUTING2_ONEHOP	13
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_14		14
#define COMMUNICATIONS_LABEL_FAMILY_FLOATINGGRAPH	14
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_15		15
#define COMMUNICATIONS_LABEL_FAMILY_NORMPATHS		15

#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_16		16
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_17		17
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_18		18
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_19		19
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_20		20
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_21		21
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_22		22
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_23		23
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_24		24
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_25		25
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_26		26
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_27		27
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_28		28
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_29		29
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_30		30
#define COMMUNICATIONS_LABEL_FAMILY_RESERVED_31		31

typedef enum { WATCHER_PROPERTY_SHAPE=0, WATCHER_PROPERTY_COLOR, WATCHER_PROPERTY_EFFECT, WATCHER_PROPERTY_SIZE } WatcherProperty; 
typedef enum { WATCHER_SHAPE_CIRCLE=0, WATCHER_SHAPE_SQUARE, WATCHER_SHAPE_TRIANGLE,
               WATCHER_SHAPE_TORUS, WATCHER_SHAPE_TEAPOT } WatcherShape;
typedef enum { WATCHER_EFFECT_SPIN=0, WATCHER_EFFECT_SPARKLE, WATCHER_EFFECT_FLASH } WatcherEffect; 
typedef struct 
{
    // enum item { node, label, edge };         // only node for now

    unsigned int identifier;    // For nodes, this is the node's ManetAddr.

    WatcherProperty property;  

    union {
        unsigned char color[4];     // If property is color, fill this out.
        WatcherShape shape;         // If propery is shape, fil this out.
        WatcherEffect effect;       // etc. 
        float size;
    } data; 
    
} WatcherPropertyInfo;

typedef struct NodeLabel
{
    unsigned char bgcolor[4],fgcolor[4]; /* background & foreground colors */
    char *text;
    ManetAddr node;

    int family;
    int priority;
    int tag;			/* client assigned grouping value.  */
    destime expiration;    /* set to 0 to never expire   (Milliseconds)  */

    struct NodeLabel *next;
} NodeLabel;

typedef struct NodeEdge
{
    ManetAddr head, tail;
    struct manetNode *nodeHead, *nodeTail;
    NodeLabel labelHead, labelMiddle, labelTail;
    unsigned char color[4];
    int width;

    int family;
    int priority;
    int tag;
    destime expiration;    /* set to 0 to never expire  */

    struct NodeEdge *next;
} NodeEdge;

typedef struct FloatingLabel
{
    int x,y,z;
    unsigned char bgcolor[4],fgcolor[4]; /* background & foreground colors */
    char *text;

    int family;
    int priority;
    int tag;                        /* client assigned grouping value.  */
    destime expiration;    /* set to 0 to never expire   (Milliseconds)  */

    struct FloatingLabel *next;
} FloatingLabel;

void communicationsWatcherFloatingLabel(CommunicationsStatePtr cs, 
                                        FloatingLabel *lab);
void communicationsWatcherFloatingLabelRemove(CommunicationsStatePtr cs, 
                                              int bitmap, 
                                              FloatingLabel *lab);

typedef enum
{
    NODE_DISPLAY_MANET = 0,
    NODE_DISPLAY_HIERARCHY = 1
} NodeDisplayType;

/* struct to describe which labels to display
 */
typedef struct
{
    int minPriority;
    int familyBitmap;
    float scaleText[2];
    float scaleLine[2];
    int monochromeMode;
    int threeDView;
    int backgroundImage;
} NodeDisplayStatus;

/* change the label of a node displayed by the watcher.
 */
void communicationsWatcherLabel(CommunicationsStatePtr cs, NodeLabel *label);
void communicationsWatcherLabelRemove(CommunicationsStatePtr cs, 
                                      int bitmap, 
                                      NodeLabel *lab);

/* mask values for the bitmap in communicationsWatcherLabelRemove() */
#define COMMUNICATIONS_LABEL_REMOVE_FAMILY          0x01
#define COMMUNICATIONS_LABEL_REMOVE_PRIORITY        0x02
#define COMMUNICATIONS_LABEL_REMOVE_TAG             0x04
#define COMMUNICATIONS_LABEL_REMOVE_NODE            0x08

/* Change the color of a node displayed by the watcher.
 * The color arg is a pointer to an array of 4 bytes (r/g/b/alpha).
 *
 * To remove the color, call with color == NULL
 */
void communicationsWatcherColor(CommunicationsStatePtr cs, 
                                ManetAddr node, 
                                unsigned char *color);
/* To modify a watcher property.
 */
void communicationsWatcherProperty(CommunicationsStatePtr cs, ManetAddr node, WatcherPropertyInfo *prop);

/* To add an edge to a graph displayed by the watcher
 */
void communicationsWatcherEdge(CommunicationsStatePtr cs, NodeEdge *edge);

/* To remove an edge added previously.  (NOP if dosn't exist.)
 */
void communicationsWatcherEdgeRemove(CommunicationsStatePtr cs, 
                                     int bitmap, 
                                     ManetAddr head, 
                                     ManetAddr tail, 
                                     int family, 
                                     int priority,
                                     int tag);

/* mask values for the bitmap in communicationsWatcherEdgeRemove() */
#define COMMUNICATIONS_EDGE_REMOVE_HEAD        0x01
#define COMMUNICATIONS_EDGE_REMOVE_TAIL        0x02
#define COMMUNICATIONS_EDGE_REMOVE_FAMILY      0x04
#define COMMUNICATIONS_EDGE_REMOVE_PRIORITY    0x08
#define COMMUNICATIONS_EDGE_REMOVE_TAG         0x10
#define COMMUNICATIONS_EDGE_REMOVE_ALL         0xFFFFFFFF

/* If you wish to use a string as a tag, use this hash function
 * (see labeltest.c)
 */
int communicationsTagHash(char *str);

/******************************************************************************
 *
 * Convenient calls to prettyprint a couple of the data types.
 */


char const *communicationsNeighborState2Str(CommunicationsNeighborState st);
char const *manetAddr2Str(ManetAddr x);
char const *messageType2Str(MessageType messageType);
char const *idsPosition2Str(IDSPositionType position);
char const *idsPositionStatus2Str(IDSPositionStatus idsPositionStatus);
char const *communicationsDestinationType2Str(CommunicationsDestinationType t);

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF
#define PRINTADDRCPP(a) \
    (((a)>>24)&0xFF) << '.' << (((a)>>16)&0xFF) << '.' \
<< (((a)>>8)&0xFF) << '.' << ((a)&0xFF)


/******************************************************************************
 *
 * API event logging functions
 */

typedef struct CommunicationsLogState *CommunicationsLogStatePtr;

/* An API can be configured to log everything to a FD
 * That FD can then be replayed later for postmortem analysis.
 * 
 * The FD may be passed to multiple CommunicationsState instances.
 * BUT: they must all be done at the same time, at the time of init.
 * (IE: call communicationsInit many times, THEN call communicationsLogEnable
 * many times, and only then may you call communicationsReadReady.)
 */
void communicationsLogEnable(CommunicationsStatePtr cs, int fd);

/* If the log gathering program (like the watcher) needs to stuff
 * additional data into the log file, this may be called to insert
 * what appears to be a message.  That message will then be delivered
 * to the log inspecting program as if it was message from local to
 * local DIRECT of type 'type'.
 */
void communicationsLogMessage(CommunicationsStatePtr cs, 
                              int type, 
                              unsigned char *payload, 
                              int payloadlen);

/* To do logfile playback, call this function on a log file.
 * 
 * Use communicationsLogNodesGet() to retrieve the array of 
 * CommunicationsState pointers, with which you can register your
 * callbacks.
 */
CommunicationsLogStatePtr communicationsLogLoad(int fd);

/* Delete the given communicationsLogState and all the associated
 * communicationsStatePtrs.  This does *not* close the file descriptor
 * that was given to communicationsLogLoad().
 */
void communicationsLogClose(CommunicationsLogStatePtr);

/* To see outgoing messages in a log file, call this and pass in a
 * standard handler.  The handler will have to call messageInfoTypeGet
 * to tell the types apart.
 */
  
int communicationsLogOutgoingHandler(
        CommunicationsStatePtr cs,
        MessageHandler messageHandlerFunc,
        void *messageHandlerData);

/* And for logfile playback, this is not unlike communicationsReadReady,
 * and will call your callbacks.
 *
 * 'step' is a number of milliseconds; advance current time by that many 
 * milliseconds, calling callbacks along the way.  If step==0, process
 * only a single event.
 *
 * The destime (epoch milliseconds) pointed to by callercurtime will
 * be set to the current time as any callbacks are called.
 *
 * Return number of events processed on success, -1 on error (e.g., EOF).
 */
long communicationsLogStep(CommunicationsLogStatePtr cl, 
			   int step, 
			   destime *callercurtime);


/* Return the CommunicationsLogStatePtr associated with the given 
 * CommunicationsStatePtr, or NULL if cs is connected to a daemon
 * rather than reading from a log. */
CommunicationsLogStatePtr communicationsLogStateGet(CommunicationsStatePtr cs);

/* Return the "current time" value, in epoch milliseconds.  
 * This value is advanced by communicationsLogStep().
 *
 * It's usually easier to use communicationsTimevalGet()
 * or communicationsDestimeGet() instead.
 */
destime communicationsLogTimeGet(CommunicationsLogStatePtr cl);

/* Return the time of the next event in the log file, in epoch milliseconds.
 * If there are no more events in the file, return -1.
 */
destime communicationsLogNextEventTimeGet(CommunicationsLogStatePtr cl);

/* Return pointer to array of CommunicationsStatePtrs represented in a log
 * file.  The array is terminated with a NULL pointer.  Caller may not modify
 * the array, but may use the CommunicationsStatePtrs (to register callbacks).
 */
CommunicationsStatePtr const * communicationsLogNodesGet(CommunicationsLogStatePtr cl);


/* Icky non-thought-out stuff for feeding data to the 3D graph in the watcher
 * This API WILL change and/or go away.
 */

typedef struct CommunicationsGraphEdge
{
    ManetAddr a,b;
    float value;
} CommunicationsGraphEdge;

/*
 * Send 'numPoints' of datapoints for a graph named 'graphname' for this node.
 */
void graphMarshal(const char *graphName, const float *floatData, int numPoints, unsigned char **buffer, int *len);
// void graphUnmarshal(float *graph, int numnodes, const unsigned char *buffer);

void graphSend(CommunicationsStatePtr cs, const char *graphName, const float *dataPts, int numDataPts);
void graphSendEdge(CommunicationsStatePtr cs,
                   CommunicationsGraphEdge *graph, 
                   int numnodes);

/* Does a namelookup, and returns a ManetAddr for a host.
 */
ManetAddr communicationsHostnameLookup(const char *nam);

/* Convert a destime to struct timeval */
void timevalFromDestime(destime t, struct timeval *tv);

#ifdef __cplusplus
}
#endif
#endif /* IDSCOMMUNICATIONS_H_FILE */
