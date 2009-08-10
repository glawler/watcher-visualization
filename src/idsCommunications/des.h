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

#ifndef DES_H
#define DES_H

#include "idsCommunications.h"
#include "config.h"

#ifndef MANETADDR_DEFINED
#define MANETADDR_DEFINED
typedef unsigned int ManetAddr;
#endif /* MANETADDR_DEFINED */

#ifndef MANETTIME_DEFINED
#define MANETTIME_DEFINED
typedef long long int destime;  // epoch milliseconds
#endif

/* This is actually infinity...  
 * I would also like to get rid of it.  (so don't use it...)
 */
#define MAXHOP 1024

#ifdef __cplusplus
extern "C" {
#endif

/* If this is defined, that packet malloc/copy/free code will implement a memory
 * allocation debugger, for finding where packets are not correctly freed.
 */
// #define DEBUG_PACKET

/* These should be called ManetNode, and ManetPacket  */
struct manetNode;
struct packet;

/* Per-node anonymous pointers, for various module's private data
 *
 * should be named   moduleNodeState
 *
 * The clustering one is odd, in that all the different clustering algorithms use it.
 * So, call it cluster, instead of the module...  (since we have different modules for
 * the differnet algorithms, so as to make sure that packet types are unique.)
 */
struct mobilityState;
struct dataState;
struct routingState;
struct helloState;
struct clusteringState;
struct floodState;
struct testtrafficState;
struct PacketApiNodeState;
struct ManetRFModelState;
struct groupClusterState; 

/* Per-neighbor anonymous pointers, for module state
 *
 * should be named   moduleNeighborState
 */

struct clusteringNeighbor;
struct groupClusteringNeighbor;  /* Should not really be here. */
struct helloNeighborState;

/* Per-manet anonymous pointers, for module state
 * 
 * should be named   moduleManetState
 */

struct mobilityManetState;

/*  These macros are module types.  
 *  They are then used to define packet types.  Thus a module's packets can be
 *  recognized using the most significant bits, and then the module can recognize
 *  which packets are which using the least significant bits.  See the PACKET_MODULE_MASK
 *  macro.
 *
 *  Look at hello.h for an example of how a module can define its packet types.
 *
 */

#define PACKET_NOPCLUSTER	0x00    /* This one is really an example, and fake  */
#define PACKET_DATA		0x10
#define PACKET_API		0x20
#define PACKET_ROUTING		0x30
#define PACKET_HELLO		0x40
#define PACKET_INTERIM		0x50
#define PACKET_GRAPHCLUSTER	0x60
#define PACKET_FLOOD		0x70
#define PACKET_AMROUTE		0x80
#define PACKET_TESTTRAFFIC	0x90
#define PACKET_BFT		0xA0
#define PACKET_INTERIM2		0xB0
#define PACKET_GROUP_MOBILITY	0xC0
#define PACKET_BARNACLE		0xD0
/* uh oh, only room for two more modules.  borrow a bit from the low byte? */

#define PACKET_MODULE_MASK(a)  ((a) & 0xF0)

/* These packet types are used to give a clustering algorithm a "state vector" of parent->child 
 * relationships.  They are in the PACKET_INTERIM2 space for no good reason...
 * To set a state vector, use the first two.  The third is used internally.
 * see statetest.c
 */
#define PACKET_STATEVECT		(PACKET_INTERIM2|0)
#define PACKET_STATEVECT_NOFLOOD	(PACKET_INTERIM2|1)

#define PACKET_STATEVECT_FLOOD		(PACKET_INTERIM2|2)

#define PACKET_MAX		0xFF

/* There is still some algorithm-specific fields in this struct...  need to move to anonymous ptrs (defined above)
 * Should also be called manetNeighbor
 */
typedef struct neighbor
{
        ManetAddr addr;          /* these two fields are the key  */
	int level;

        struct neighbor *next;

				/* generic fields */
        ManetAddr clusterhead;         /* the CH this neighbor is using. ==NODE_BROADCAST means none  */
	int clusterheadflag;           /* set if this neighbor is a clusterhead  (may not actually be known)  */

        destime firstheard;
        destime lastheard;

        int onehopdegree;        /* these are interim algorithm fields */
	int hopcount;
        unsigned int flags;

	ManetAddr helloCoordinator;         /* hello module fields  */
	int helloDegree;
	ManetAddr helloDesiredCoordinator;
	int helloCoordinatorFlag;

	struct clusteringNeighbor *cluster;
	struct groupClusteringNeighbor *groupCluster;

} neighbor;

/* values for neighbor.flags
*/
#define NEIGHBOR_1HOP			0x01
#define NEIGHBOR_CHILD			0x02
#define NEIGHBOR_PARENT			0x04
#define NEIGHBOR_ROOT			0x08
#define NEIGHBOR_HEARD			0x10
#define NEIGHBOR_HEARS			0x20


/* Should be called manetPacket
 */
typedef struct packet
{
	struct packet *next;	/* Caller: don't modify these */
	int refcount;  

	ManetAddr src;
	ManetAddr dst;
	int type;      /* This is in reality an 8 byte value */
	int len;       /* size of buffer pointed to by data */
	int ttl;       /* set by caller, and decremented by forwarding code.  dropped on 0 */
	int hopcount;  /* inited to 0, incremented by forwarding code */

	void *data;

#ifdef DEBUG_PACKET
	struct manetNode *originnode;
	struct packet *debugprev,*debugnext;
	char *mallocfile;
	int mallocline;
	destime malloctime;

	char *dupfile;
	int dupline;
	destime duptime;
#endif
} packet;

#define NODE_ADDRESSSIZE 4
#define NODE_BROADCAST (0xFFFFFFFF)

#define MAXLEVEL 50

/* Size of the recent packets list, for not repeating already-repeated packets
*/
#define NODE_PACKETLIST 100

/* The majority of the algorithm-specific fields per node are now in proper anonymous pointers
 * The physical model stuff needs some thought.
 */
typedef struct manetNode
{                    /* physical model stuff */
	double x,y,z;   /* location in 3 space.  not all functions are actually fully 3D though  */
	int aradius;    /* antenna radius */
	ManetAddr addr, bcastaddr, netmask;   /* our address */
	int index;        /* our index in the manet array.  */

	 unsigned char *color;   /* array of 4 chars...  or NULL to use default coloring */
	NodeLabel *labelList;    /* labels which have been applied to this node */
	
	struct manet *manet;           /* manet to which this node belongs.  used for IO, and breaking abstraction (for metrics) */

	ApiStatus status;		/* packet counters, and debugging stuff */

		/* stuff for doing list of last n packets.  (not all clustering algs use it)  */
	struct packet *packetlist[NODE_PACKETLIST];   /* list of last n heard packets, to avoid repeating a packet more than once */
	int packetlistlen,packetlistpos;

		/* visible clustering stuff:  */
	neighbor *clusterhead;		/* our clusterhead.  Its a pointer into the neighbor list.  Thus CH must be a neighbor right now */
	neighbor *neighborlist;
	int level;			/* which level we're at in the hierarchy  */
	int rootflag;           /* node is a hierarchy root */
	int rootgroupflag;      /* node is an active member of "root group" */

	struct routingState *routing;    /* routing stuff. */
	struct clusteringState *cluster;    /* clustering stuff */
	struct dataState *data;               /* reliable data packet stuff */
	struct floodState *flood;             /* flood routing algorithm stuff */
	struct testtrafficState *testtraffic;             /* test traffic stuff */
	struct helloState *hello;
	struct mobilityState *mobility;
	struct PacketApiNodeState *packetApi;

	struct groupClusterState *groupCluster; /* Does not belong here really. */
} manetNode;

/* packet type counters.  
 * Needs some additional thought, and code cleanup
 *
 * should be called manetPacketCount
 */
typedef struct
{
	long long int packetorigin;
	long long int packetrepeat;
} packetcount;


/* functions for sending/receiving packets
*/
#ifdef DEBUG_PACKET
void packetDumpDebug(void);
#else
#define packetDumpDebug(a)
#endif
#define packetMalloc(a,b) packetMallocInt(a,b,__FILE__,__LINE__)
packet *packetMallocInt(manetNode *us, int len,char *by,int line);
packet *packetRemalloc(manetNode *us, int len,packet *oldp);

#define packetDup(a) packetDupInt(a,__FILE__,__LINE__)
packet *packetDupInt(packet *p, char *fil, int lin);     /* does a refcount thing, intended only for broadcast simulation   */

#define packetCopy(a,b) packetCopyInt(a,b,__FILE__,__LINE__)
packet *packetCopyInt(packet const *o,int payloaddelta,char *by, int line);    /* does a real copy, must be done if caller wants to modify the packet   (payloaddelta is added to the payload length, if the caller wishes to change its size (all the better to encapsulate!))  */

void packetFree(packet *p);

#define packetOriginate(us,p) do{us->manet->packetorigin++;us->manet->packethistogram[p->type].packetorigin++;}while(0)
#define packetRepeat(us,p) do{us->manet->packetrepeat++;us->manet->packethistogram[p->type].packetrepeat++;}while(0)
#define packetRepeatType(us,t) do{us->manet->packetrepeat++;us->manet->packethistogram[t].packetrepeat++;}while(0)

/* This function needs to be separately defined for livenetwork vs simulator mode
 * So, it is documented/prototyped here (since it goes with all the other packet functions),
 * but declared elsewhere (in main.cpp for simulator, and livenetwork.cpp for livenetwork)
 * 
 * if origflag is is one of the following macros, and indicates how to count this packet
 * (a repeat of an existing packet, an origination of a packet, or nothing)
 * (used for statistics)
 */
#define PACKET_REPEAT	1
#define PACKET_ORIGIN	2
#define PACKET_RECEIVE	3
#define PACKET_NOP	4

void packetSend(manetNode *us, packet *p, int origflag);   /* node us will transmit packet p.  (us is the origin, not the destination */


void packetReReceive(manetNode *us, packet *p);

void statusCount(manetNode *us, int origflag, packet *p);



/* ***************************** Event list stuff *******************
 *
 * This code started life as a discrete event simulator.  It then
 * mutated into a live network imlementation, which uses the event list
 * as a sort of combined transmit and receive queue.
 *
 * Being able to run either as a simulator or a live implementation is
 * incredibly handly for debugging...
 */

typedef void eventCallback(struct manetNode *, void *data);

/* This is used for a linked list of events for the manet
*/
typedef struct eventnode
{
	int type;

	eventCallback *callback;	/* valid only if type == EVENT_TIMER or type == EVENT_TICK */
	int fd;				/* valid only if type == EVENT_READFD or EVENT_WRITEFD */

	union
	{
		packet *pkt;
		void *undef;
	} data;

	destime xmittime;		/* timestamp of event initiation  */
	destime rectime;		/* timestamp (in future...) of event  */
        struct manetNode *nodeaddr;	/* node which gets this event */

	struct eventbucket *bucket;	/* time bucket which this event belongs to */
	struct eventnode *prev;	/* previous and next events (unordered) in the bucket (this list does not cross buckets */
	struct eventnode *next;
} eventnode;

/* The eventnodes are kept in buckets by their rectime.  
 *  The assumption is that there will not be too many unique times,
 *  and the insert into a bucket is a cheap unordered type thing
 */
typedef struct eventbucket
{
	destime rectime;		/* timestamp of this bucket */
	eventnode *bucket;		/* first eventnode in linked list of eventnodes for this bucket */
	struct eventbucket *prev;	/*previous and next buckets */
	struct eventbucket *next;
} eventbucket;

/* event types:  (eventnode->type)
 * note that bit 0x10 idicates that the event type is a packet.
 */
#define EVENT_PACKET	0x10	/* event is reception of a packet  */
#define EVENT_REPACKET	0x11	/* this is just like PACKET, only its used for rereceived packets so they can be recognied as such */
#define EVENT_TIMER 	0x02	/* event is timer event  */
#define EVENT_TICK	0x03	/* event is called when the current bucket is empty and the simulator is going onto the next  */
#define EVENT_FDREAD	0x04	/* event is called when the fd is readable */
#define EVENT_FDWRITE	0x05	/* event is called when the fd is writeable */

/* functions for scheduling callbacks
 */
eventnode *timerSet(manetNode *us, eventCallback *cb, int delay, void *rawptr);

/* A "tick" will be called when the DES clock ticks.
 */
eventnode *tickSet(manetNode *us, eventCallback *cb,void *rawptr);


/* Create an event node for a packet, and put it in the event list
 * Do not call...  use packetSend instead
 */
eventnode *packetEnqueue(manetNode *us,packet *p,int delay);

/* File descriptor events:
 * When the FD is readable or writeable, its callback is called, and must be rescheduled
 * If its scheduled for both readable and writeable, and one happens, the other remains scheduled
 *
 * If the same fd is scheduled to be read (or written) twice one of them will be called (and
 * removed from the list) but the other will remain scheduled.  (I recomend NOT multiple-scheduling
 * things...)
 */
eventnode *eventFileRead(manetNode *us, int fd, eventCallback *cd, void *rawptr);
eventnode *eventFileWrite(manetNode *us, int fd, eventCallback *cd, void *rawptr);
void eventFileClose(manetNode *us, int fd);    /* cancel all events involving fd */

typedef void ManetPacketCallback(manetNode *, packet *);

typedef struct manet
{
	Config *conf;                /* config file structure */
	struct PacketProtection *packetProtection;

	int numnodes;
	manetNode *nlist;            /* the nodes themselves   */

	/* event list structures */
	eventbucket *eventlist;     /* list of events to be executed */
	eventnode *ticklist;      /* list of tick events, to be called when the current instant is over */
	eventnode *fdlist;        /* list of file descriptor events */

	destime starttime,curtime;
	int step;

	int *linklayergraph;
	struct ManetRFModelState *rfState;
	int *hierarchygraph;    /* if not NULL, then this is valid, otherwise compute from linklayer graph.  */

	ManetPacketCallback *callbackList[256];

	struct mobilityManetState *mobility;
} manet;

int eventnodeNumPackets(manet *m);

void manetPacketHandlerSet(manetNode *us, int type, ManetPacketCallback *cb);

/* called when a packet is received, to determine which module to send the packet to
*/
void desGotPacket(manetNode *us, packet *p);

void manetDraw(manet *m);

manet *manetInit(Config *conf,destime starttime);
void firstStep(manet *m, int moduleinit);     /* Call once, to complete init  set moduleinit to 1 to init all modules too   */
void takeStep(manet *m);      /* Call a great many times.      */
void manetIOCheck(manet *m);
void lastStep(manet *m);      /* Call once, to do cleanup      */

/* functions for playing with the event list.  This is intended for the livenetwork stuff, NOT simulations
*/

eventnode *eventnodeNextTimer(manet *m);
eventnode *eventnodeNextPacket(manet *m);
void eventnodeDelete(manet *m, eventnode *en);
void eventnodeFree(eventnode *en);
void eventnodeWalkReReceive(manet *m);

/* functions that must be implemented by each clustering algorithm */
void nodeInit(manetNode *n);
void nodeFree(manetNode *n);

#ifdef __cplusplus
}
#endif

#endif
