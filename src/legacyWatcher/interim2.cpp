/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006,2007  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 *  This is the second version of the interim algorithm, after lots of lessons
 *  learned.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "interim2.h"
#include "packetapi.h"
#include "apisupport.h"		/* for IDSStateUnmarshal() */

#include "des.h"
#include "graphics.h"
#include "node.h"
#include "marshal.h"
#include "flood.h"
#include "gmcluster.h"

/* If a clustering algorithm wants random numbers, it needs its own 
 * private random number generator */
#ifdef USE_RNG
/* use the RNG class pseudorandom number generator */
#include "rng.h"
static RNG *rnd=NULL;
#define RAND_U01() (rnd->rand_u01())
#else
#define RAND_U01() (drand48())
#endif


static const char *rcsid  __attribute__ ((unused)) = "$Id: interim2.cpp,v 1.84 2007/08/14 17:38:18 dkindred Exp $";

/* A clustering algorithm gets the field us->cluster for its private per-node data
 */
typedef struct clusteringState
{
	destime selecttime;

	int floatingrootflag;

	neighbor *desiredCoordinator;
	destime lastDesiredTime;
	destime lastPromotionTime;
	int ttls[MAXLEVEL];
	int sequencenum;
	ManetAddr root;
	int rootDistance;	/* only set this using setRootDistance() */
	int locked;             /* parent, root, and rootgroup state locked */

	neighbor **desireList;
	neighbor **useList;
	neighbor **assignedList;

	int nailedroot;

	int maxTTL;          /* config parameter, read from conf file in nodeInit() */
	int timeSelect;
	int timeHello;
	int timeHelloTimeout;
	int timeUndesired;
	int timePromotion;
	int minSequentialCount;
	int hopCountWindowSize;
	int leafscale;
	int enrouteCHEnable;
	int gmClusterEnable;
	int gmClusterTTL;
	int rootGroupRadius; 		/* nodes whose rootDistance <= this value are in the RG */
	int rootGroupRadiusMax;
	int rootGroupMinSize;  		/* 0=don't do dynamic root group, else use this val and expand root group radius */
				     	/* until there are this many root group nodes */

	helloHello *helloCallback;
	unsigned char *helloPayload;
	int helloPayloadLen;
} clusteringState;

#define INTERIM2_TIME_SELECT 2000
#define INTERIM2_TIME_HELLO  2000
#define INTERIM2_TIME_HELLO_TIMEOUT  5000
#define INTERIM2_UNDESIREDTIME 10000
#define INTERIM2_PROMOTIONTIME 5000
#define INTERIM2_MINSEQUENTIALCOUNT 3
#define INTERIM2_LEAFSCALE 4
#define INTERIM2_ENROUTECHENABLE 1
#define INTERIM2_ROOTGROUPRADIUS 2
#define INTERIM2_ROOTGROUPMINSIZE 0

#define INTERIM2_ROOT 1
#define INTERIM2_ROOTNAILED 2

#define INTERIM2_MAXNEIGHBORS 100

#define INTERIM2_HOPCOUNTWINDOW 4

#define PRINTCOORD(a) (((a)?((a)->addr) : NODE_BROADCAST) & 0xFF)

#ifdef DEBUG_DYNAMICROOTGROUP
	#define DYNRG_RADIUS_DEBUG(message, ...) fprintf(stderr, message, ## __VA_ARGS__)
#else
	#define DYNRG_RADIUS_DEBUG(message, ...) 
#endif
					  
					  
/* A clustering algorithm gets the field neighbor->cluster for private per-neighbor data
*/
typedef struct clusteringNeighbor
{
	int lastsequencenum;
	int totSymmetricNeighbors;
	/* List of symmetric neighbors? */
	ManetAddr coordinator;
	ManetAddr desiredCoordinator;
	int level;
	ManetAddr root;
	int rootDistance;  /* the neighbor computes its distance from the root, and announces it in hellos */
	int ttl;           /* the neighbor's current TTL, read from hellos  */
	int rootflag;      /* announced by neighbor, indicates that it is root, is actually one of 0, INTERIM2_ROOT, or INTERIM2_ROOTNAILED */
	int rootGroupRadius;	/* -1 means "not set".  0..255 are valid */

	ManetAddr route[MAXHOP];
	int routelen;

	int *hopCountWindow;
	int hopCountPos;
	int hopCountNum;

	int sequentialCount;    /* how many hello packets have we heard with no gaps in the sequence numbers?  */

	int weights[COORDINATOR_MAXVAL];
} clusteringNeighbor;

/* clustering algorithm-private hello packet struct.
 * (marshaled and unmarshaled with functions below)
 */

typedef struct
{
        int numNeighbors;                /* number of neighbors (including asymmetric) */
        int sequencenum;
        ManetAddr coordinator,desiredCoordinator;
	int totSymmetricNeighbors;
        ManetAddr addressList[400];
	int level;
	ManetAddr root;
	int rootflag;
	int rootDistance;

	ManetAddr route[MAXHOP];
	int routelen;

	unsigned char *helloPayload; /* extra data to insert in HELLOs */
	int helloPayloadLen;
	int helloPayloadMalloced;    /* if nonzero, payload should be freed */

	/* roots normally include their preferred rootgroup radius */
	int rootGroupRadius;	/* -1 means "not set".  0..255 are valid */

} packetHello;

typedef struct
{
	ManetAddr dst;
	ManetAddr route[MAXHOP];
	int routelen;
} packetRoot;

static void stateRec(manetNode *us, packet *p);
static void helloRec(manetNode *us, packet *p);
static void rootRec(manetNode *us, packet *p);
static void selectCoordinator(manetNode *us);
static void setRoot(manetNode *us);
static void setRootTo(manetNode *us, ManetAddr rootAddr);
static void setRootDistance(manetNode *us, int newValue);
static void setClusterHead(manetNode *us, neighbor *ch, const char *reason);
static int neighborCompare(const void *ap,const void *bp);
static neighbor* newNeighbor(manetNode *us, ManetAddr addr);
static int checkRootGroupMembership(manetNode *us);
static void setRootGroupRadius(manetNode *us); 

#define INTERIM2_HELLO_OPTION_NOOP             0
#define INTERIM2_HELLO_OPTION_ROOTGROUP_RADIUS 1
#define INTERIM2_HELLO_OPTION_PAYLOAD          2

/* most options look like
 *    BYTE tag
 *    SHORT len
 *    <len bytes of data>
 * except for these special ones:
 *    INTERIM2_HELLO_OPTION_NOOP             (no len, no data)
 *    INTERIM2_HELLO_OPTION_ROOTGROUP_RADIUS (no len, one data byte)
 *
 * We can add more "non-special" options without bumping the protocol version
 * because old daemons will just ignore them.
 */

/* if options is NULL, returns size required */
static unsigned int helloOptionsMarshal(manetNode *us, const packetHello *p,
					unsigned char *options)
{
	unsigned char *o = options;
	unsigned int len = 0;

	/* rootgroup radius */
	if (p->rootGroupRadius >= 0)
	{
		len += 1 + 1;	/* tag, radius */
		if (o)
		{
			MARSHALBYTE(o, INTERIM2_HELLO_OPTION_ROOTGROUP_RADIUS);
			MARSHALBYTE(o, p->rootGroupRadius);
		}
	}

	/* user-defined payload */
	if (p->helloPayloadLen != 0)
	{
		len += 1 + 2 + p->helloPayloadLen; /* tag, length, payload */
		if (o) 
		{
			MARSHALBYTE(o, INTERIM2_HELLO_OPTION_PAYLOAD);
			MARSHALSHORT(o, p->helloPayloadLen);
                        MARSHALBUFFERLONG(o, p->helloPayload, 
                                          p->helloPayloadLen);
		}
	}

	return len;
}

/* return 0 on success */
static int helloOptionsUnmarshal(manetNode *us, packetHello *p,
				 unsigned char *options,
				 size_t optionsLen)
{
	unsigned char *o = options;
	while (o < options + optionsLen)
	{
		unsigned char tag;
		UNMARSHALBYTE(o, tag);
		switch (tag)
		{
		case INTERIM2_HELLO_OPTION_NOOP:
			break;
		case INTERIM2_HELLO_OPTION_ROOTGROUP_RADIUS:
		{
			UNMARSHALBYTE(o, p->rootGroupRadius);
		}				
			break;
		case INTERIM2_HELLO_OPTION_PAYLOAD:
		{
			unsigned short len;
			UNMARSHALSHORT(o, len);
                        p->helloPayload = (unsigned char *)malloc(len);
			p->helloPayloadLen = len;
                        UNMARSHALBUFFERLONG(o, p->helloPayload,
                                            p->helloPayloadLen);
                        p->helloPayloadMalloced = 1;
		}				
			break;
		default:
			break;
		}
	}
	return (o == options + optionsLen) ? 0 : -1;
}

static packet *helloMarshal(manetNode *us, packetHello *p)
{
	packet *np;
	int len;
	unsigned char *hp;
	int i;
	unsigned int optionsLen = helloOptionsMarshal(us,p,NULL);

	len=
		2 +			/* helloOptionsLen  */
		1 +			/* p->numNeighbors */
		1 +			/* p->level */
		1 +			/* p->totSymmetricNeighbors */
		4 +			/* p->sequencenum */
		4 +			/* p->coordinator */
		4 +			/* p->desiredCoordinator */
		(4 * p->numNeighbors) + /* hp->addressList */
		1 + (4 * p->routelen) + /* routelen, and route */
		1 +			/* root flag  */
		1 +			/* root distance  */
		4 +			/* root manetAddr */
		optionsLen
		;

	np=packetMalloc(us,len);
	hp=(unsigned char *)np->data;
	
	MARSHALSHORT(hp,optionsLen);
	MARSHALBYTE(hp,p->numNeighbors);
	MARSHALBYTE(hp,p->level);
	MARSHALBYTE(hp,p->totSymmetricNeighbors);
	MARSHALLONG(hp,p->sequencenum);
	MARSHALLONG(hp,p->coordinator);
	MARSHALLONG(hp,p->desiredCoordinator);
	for(i=0;i<p->numNeighbors;i++)
		MARSHALLONG(hp,p->addressList[i]);

	MARSHALBYTE(hp,p->routelen);
	for(i=0;i<p->routelen;i++)
		MARSHALLONG(hp,p->route[i]);
	MARSHALBYTE(hp,p->rootflag);
	MARSHALBYTE(hp,p->rootDistance);
	MARSHALLONG(hp,p->root);
	hp += helloOptionsMarshal(us,p,hp);
	hp+=p->helloPayloadLen;

	return np;
}

static packetHello *helloUnmarshal(manetNode *us, packet *np)
{
	packetHello *p = NULL;
	unsigned char *hp;
	unsigned char *endp = (unsigned char *)np->data + np->len;
	int i;
	int optionsLen;
	int numNeighbors;
	int maxNeighbors = sizeof(p->addressList)/sizeof(p->addressList[0]);

	hp=(unsigned char *)np->data;

	/* check that there's enough data to get up to start of neighborlist */
	if (endp - hp < (2 +			/* helloOptionsLen  */
		       1 +			/* p->numNeighbors */
		       1 +			/* p->level */
		       1 +			/* p->totSymmetricNeighbors */
		       4 +			/* p->sequencenum */
		       4 +			/* p->coordinator */
		       4))			/* p->desiredCoordinator */
	{
		goto trunc;
	}

	UNMARSHALSHORT(hp,optionsLen);
	UNMARSHALBYTE(hp,numNeighbors);

	if (numNeighbors > maxNeighbors)
	{
		fprintf(stderr, "node %u: WARNING: ignoring hello with too many neighbors listed (%d > %d)\n",
			us->addr & 0xFF, numNeighbors, maxNeighbors);
		return NULL;
	}

	p=(packetHello*)malloc(sizeof(*p));       /* stick the block of neighbor addresses in this malloc block?  */

	p->numNeighbors=numNeighbors;
	UNMARSHALBYTE(hp,p->level);
	UNMARSHALBYTE(hp,p->totSymmetricNeighbors);
	UNMARSHALLONG(hp,p->sequencenum);
	UNMARSHALLONG(hp,p->coordinator);
	UNMARSHALLONG(hp,p->desiredCoordinator);
	/* enough data left for the neighbor list and routelen? */
	if (endp - hp < p->numNeighbors * 4 + 1) goto trunc;
	for(i=0;i<p->numNeighbors;i++)
		UNMARSHALLONG(hp,p->addressList[i]);

	UNMARSHALBYTE(hp,p->routelen);
	/* enough data left for route, rootflag, rootDistance, root, options? 
	 */
	if (endp - hp < p->routelen * 4 + 1 + 1 + 4 + optionsLen)
	{
		goto trunc;
	}
	for(i=0;i<p->routelen;i++)
		UNMARSHALLONG(hp,p->route[i]);
	UNMARSHALBYTE(hp,p->rootflag);
	UNMARSHALBYTE(hp,p->rootDistance);
	UNMARSHALLONG(hp,p->root);
	p->helloPayload = NULL;
	p->helloPayloadLen = 0;
	p->helloPayloadMalloced = 0;
	p->rootGroupRadius = -1;
	if (0 == helloOptionsUnmarshal(us, p, hp, optionsLen))
	{
		hp += optionsLen;
	}
	else
	{
		fprintf(stderr, "node %u: WARNING: ignoring hello with malformed options\n",
			us->addr & 0xFF);
		goto fail;
	}
	if (hp != endp)
	{
		fprintf(stderr, "node %u: ignoring %d trailing extra bytes in hello\n",
			us->addr & 0xFF, endp-hp);
	}

	return p;
trunc:
	fprintf(stderr, "node %u: WARNING: ignoring truncated hello (%d bytes)\n",
		us->addr & 0xFF, np->len);
fail:
	if (p) free(p);
	return NULL;
}

static void helloFree(packetHello *p)
{
	if (p->helloPayloadMalloced) free(p->helloPayload);
	free(p);
}

static packet *rootMarshal(manetNode *us, packetRoot *p)
{
	packet *np;
	int len;
	unsigned char *hp;
	int i;

	len=	4 + 1 + (4 * p->routelen);  /* routelen, and route */
		;

	np=packetMalloc(us,len);
	hp=(unsigned char *)np->data;
	
	MARSHALLONG(hp,p->dst);
	MARSHALBYTE(hp,p->routelen);
	for(i=0;i<p->routelen;i++)
		MARSHALLONG(hp,p->route[i]);
	return np;
}

static packetRoot *rootUnmarshal(packet *np)
{
	packetRoot *p;
	unsigned char *hp;
	int i;

	p=(packetRoot*)malloc(sizeof(*p));
	hp=(unsigned char *)np->data;
	
	UNMARSHALLONG(hp,p->dst);
	UNMARSHALBYTE(hp,p->routelen);
	for(i=0;i<p->routelen;i++)
		UNMARSHALLONG(hp,p->route[i]);
	return p;
}

static void rootSend(manetNode *us, neighbor *dst)
{
	packetRoot rp;
	int i;
	packet *p;

	rp.dst=dst->addr;
	rp.routelen=dst->cluster->routelen;
	for(i=0;i<rp.routelen;i++)
		rp.route[rp.routelen-1-i]=dst->cluster->route[i];

	p=rootMarshal(us,&rp);
	p->type=PACKET_INTERIM2_ROOT;
	p->dst=NODE_BROADCAST;
	p->ttl=us->cluster->maxTTL;
	packetSend(us,p,PACKET_ORIGIN);
	packetFree(p);
}

static void setLevel(manetNode *us, int newLevel)
{
	if (us->level != newLevel)
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr, "node %d: changing level %d -> %d\n",
			us->addr & 0xFF, us->level, newLevel);
		
#endif
		if (us->level < newLevel)
		{
			/* Promotion:  set TTL to level-1 * 2  */
			/* XXX this isn't what's documented in interim2.txt.  also it should presumably be a function of newLevel, not oldlevel */
			if (newLevel>0) us->cluster->ttls[newLevel]=(us->level+1)*2;
			us->cluster->lastPromotionTime=us->manet->curtime;
		}
		else
		{
			/* Demotion: set TTL if it's not already set
			 * (we may have skipped this level on the way
			 * up).
			 */
			if (newLevel > 0
			    && us->cluster->ttls[newLevel] <= 0)
			{
				us->cluster->ttls[newLevel] = 
					newLevel*2;
			}
			/* XXX interim2.txt suggests that we forget all our
			 * stored ttls once we hit level 0.  could do that
			 * via
			 *   for (i = 1; i < MAXLEVEL; i++) us->cluster->ttls[i] = -1;
			 * (as in nodeInit()).  Should we?
			 */
		}
		us->level = newLevel;
	}
	assert(us->cluster->ttls[us->level] > 0);
}

/* When a root node surrenders its rootness, it sends a ROOT
 * packet to its new CH.  If that CH has a CH, it forwards the ROOT
 * packet to its CH.  If a node gets a ROOT packet, and dosn't have
 * a CH, then it is now root.
 */

static void rootRec(manetNode *us, packet *p)
{
	packetRoot *rp;

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: got a root packet from %d\n",us->addr & 0xFF, p->src & 0xFF);
#endif
	rp=rootUnmarshal(p);
	if (rp->dst!=us->addr)
	{
		/* do the source routing thing with the root packet (need a module for this?) */
		if ((p->ttl>0) 
		    && p->hopcount-1 < rp->routelen 
		    && (rp->route[p->hopcount-1]==us->addr))
		{
			packet *cpy;

#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: forwarding a root packet from %d to %d\n",us->addr & 0xFF, p->src & 0xFF, p->dst & 0xFF);
#endif
			cpy=packetCopy(p,0);
			cpy->hopcount++;
			cpy->ttl=p->ttl-1;
			packetSend(us,cpy,PACKET_REPEAT);
			packetFree(cpy);
		}
		free(rp);
		return;
	}
	free(rp);

	if ((us->clusterhead) && (us->clusterhead->addr!=p->src))
	{
/* This can cause root packets to go into loops...  */
#if 0
		rootSend(us,us->clusterhead);
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: %d says we're root, but we want %d to be root\n",us->addr & 0xFF, p->src & 0xFF,us->clusterhead->addr & 0xFF);
#endif
#endif
	}
	else
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: node %d says we're root!\n",us->addr & 0xFF, p->src & 0xFF);
#endif
		us->rootflag=INTERIM2_ROOT;
		us->cluster->ttls[us->level]=us->cluster->maxTTL;
	}
}

/* XXX there should be some way to configure which nodes (if any)
 * we'll accept a statevect from.  And they should probably be signed.
 */
static void stateRec(manetNode *us, packet *p)
{
        IDSState *stateVec;
	int newRootgroupflag = 0;
	ManetAddr newParent = us->addr;
	ManetAddr newRoot;
	unsigned int i,j,foundroot;

	fprintf(stderr,"node %d: got a state vector\n", us->addr & 0xFF);
	if (p->type==PACKET_STATEVECT)
	{
		packet *cpy;

		fprintf(stderr,"node %d: flooding state vector\n", us->addr & 0xFF);

		cpy=packetCopy(p,0);
		cpy->type=PACKET_STATEVECT_FLOOD;
		cpy->ttl=255;
		cpy->dst=NODE_BROADCAST;
		floodSend(us,cpy);
		packetFree(cpy);
	}

        if ((stateVec = IDSStateUnmarshal(p)) == NULL)
	{
		fprintf(stderr,"node %d: IDSStateUnmarshal() failed\n",us->addr & 0xFF);
		return;
	}

	us->cluster->locked = 0; /* (temporarily) unlock clustering state */

	setRootTo(us, NODE_BROADCAST);
	for (i = 0; i < stateVec->numNodes; i++)
	{
		ManetAddr node = stateVec->state[i].node;
		ManetAddr parent = stateVec->state[i].parent;
		int rootgroupflag = stateVec->state[i].rootgroupflag;
		fprintf(stderr,"%d.%d.%d.%d (rg:%s) to %d.%d.%d.%d\n",PRINTADDR(node),rootgroupflag ? "yes" : "no", PRINTADDR(parent));
		if (node==us->addr)
		{
			fprintf(stderr,"node %d: Got a state vector, CH= %d RG= %d\n",us->addr & 0xFF, parent & 0xFF, rootgroupflag);
			newParent = parent;
			if (parent==us->addr)
			{
				fprintf(stderr,"node %d: state vector says we're root\n",us->addr & 0xFF);
				setClusterHead(us, NULL, "state vector");
				us->rootflag=INTERIM2_ROOT;
			}
			else
			{
				neighbor *ch;
				if ((ch=neighborSearch(us, parent,0)) == NULL)
				{
					ch = newNeighbor(us, parent);
				}
				setClusterHead(us, ch, "state vector");
				fprintf(stderr,"node %d: state vector says not root\n",us->addr & 0xFF);
				us->rootflag=0;
			}
			newRootgroupflag = rootgroupflag;
			us->cluster->selecttime=us->manet->curtime+us->cluster->timeSelect;
			if (us->level>0)
			{
				/* XXX set ttl? set level to the appropriate value by walking tree? */
				setLevel(us, 1);
			}
		}
	}

	/* try to find our root */
	newRoot = newParent;
	j = 0;
	foundroot = 0;
	while (!foundroot)
	{
		for (i = 0; i < stateVec->numNodes; i++)
		{
			if (stateVec->state[i].node == newRoot)
			{
				if (stateVec->state[i].parent == newRoot)
				{
					foundroot = 1;
				}
				newRoot = stateVec->state[i].parent;
				break;
			}
		}
		if (i == stateVec->numNodes)
		{
			fprintf(stderr,"node %d: No statevec element found for ancestor %u; assuming it is root\n",us->addr & 0xFF, newRoot & 0xFF);
			break;
		}
		if (j++ >= stateVec->numNodes)
		{
			fprintf(stderr,"node %d: Hit cycle looking for root.. going with %u\n",us->addr & 0xFF, newRoot & 0xFF);
			break;
		}
	}
	setRootTo(us, newRoot);

	/* set this after processing everything else, since
	 * the setRootTo() call may have set it. */
	us->rootgroupflag = newRootgroupflag;

	/* lock parent, root, rootgroup state? */
	us->cluster->locked = stateVec->lock;
	fprintf(stderr,"node %d: %s: clusterhead= %u root= %u.%u.%u.%u rootflag= %d rootgroupflag= %d %s\n", 
		us->addr & 0xFF, __func__, PRINTCOORD(us->clusterhead), PRINTADDR(us->cluster->root),
		us->rootflag, us->rootgroupflag,
		us->cluster->locked ? "LOCKED" : "UNLOCKED");

	IDSStateFree(stateVec);
}

static neighbor* newNeighbor(manetNode *us, ManetAddr addr)
{
	neighbor *n;
#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: new neighbor struct for %d\n",us->addr & 0xFF, addr & 0xFF);
#endif
	n=neighborInsert(us,addr,0);
	n->cluster=(clusteringNeighbor*)malloc(sizeof(*(n->cluster))+ (us->cluster->hopCountWindowSize * sizeof(n->cluster->hopCountWindow[0])));
	n->cluster->hopCountWindow=(int*)((char*)(n->cluster)+sizeof(*(n->cluster)));
	n->flags=0;
	n->level=0;
	n->lastheard=us->manet->curtime;
	n->cluster->lastsequencenum=-1;
	n->cluster->hopCountPos=0;
	n->cluster->hopCountNum=0;
	n->cluster->sequentialCount=0;
	n->cluster->rootflag=0;
	n->cluster->level=0;
	n->cluster->rootGroupRadius=-1;
	n->hopcount=255;
	return n;
}

static void helloRec(manetNode *us, packet *p)
{
	/*
	Are we in floating root? Well, leave. 
		set select time to now + selecttime
		clear root flag
		clear coordinator
		clear desiredcoordinator
		level=0


	do we need to repeat this packet?   (have we seen it?)
		check packets hopcount with its TTL.  
			repeat it if it needs to go further

	Do the neighbors thing with the packet.
	*/

	packetHello *hp;
	neighbor *n, *sender;
	int i;
	int updateRootGroupRadius=0;

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: got a hello from %d distance %d ttl= %d\n",
		us->addr & 0xFF, 
		p->src & 0xFF,
		p->hopcount,
		p->ttl);
#endif

	if (p->src==us->addr)		/* One should not listen to strange nodes...  */
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: ignoring hello from self\n",
			us->addr & 0xFF);
#endif
		return;
	}

	if (us->cluster->floatingrootflag
	    && !us->cluster->locked)   /* we just got a packet, we can't possibly be floating anymore */
	{
		us->cluster->floatingrootflag=0;
		us->rootflag=0;
		setRoot(us);
		us->cluster->selecttime=us->manet->curtime+us->cluster->timeSelect;
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: set selecttime because nolonger floating root\n",us->addr & 0xFF);
#endif
		us->color=0;
	}

	hp=helloUnmarshal(us,p);

	if (hp==NULL)            /* unmarshal failed  */
	{
		fprintf(stderr,"node %d: hello unmarshal failed\n",
			us->addr & 0xFF);
		return;
	}

	n=neighborSearch(us,p->src,0);   /* note we're flattening everything to level 0 in the neighbor list  */

	if (n==NULL)
	{
		n=newNeighbor(us,p->src);

		/* new neighbor, check dynamic root group radius */
		DYNRG_RADIUS_DEBUG("Got a new neighbor, updating dynamic root group\n"); 
		updateRootGroupRadius=1;
	}

	/* have we seen this packet?  Chuck it... */
#warning sequence number wraparound
	if (n->cluster->lastsequencenum >= hp->sequencenum)
	{
#ifdef DEBUG_INTERIM2
#if 0
		fprintf(stderr,"node %d: ignoring duplicate or out-of-sequence hello: seq= %d last= %d\n",
			us->addr & 0xFF,
			hp->sequencenum,
			n->cluster->lastsequencenum);
#endif
#endif
		helloFree(hp);
		return;
	}

	if (p->hopcount>1)
		sender=neighborSearch(us,hp->route[hp->routelen-1],0);
	else
		sender=n;

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: got a hello from %d via %d seq num %d distance %d routelen= %d ttl= %d root= %d rootflag= %d rootDistance= %x\n",us->addr & 0xFF, p->src & 0xFF, ((hp->routelen<1)?(p->src):(hp->route[hp->routelen-1])) & 0xFF, hp->sequencenum, p->hopcount,hp->routelen,p->ttl,hp->root & 0xFF, hp->rootflag,hp->rootDistance);
#endif

	/* should we repeat this packet?   */
	/*  note that the repeat elimination has already been done.  WE havn't seen this pkt before.
	 * firsthop and source is a 1 hop neighbor, or n'th hop and sender is a 1 hop neighbor
	 * AND
	 *   if a neighbor has repeated it, and that neighbor's neighbors don't cover our neighbors...
	 */
	if ((p->hopcount < p->ttl))
	{
		packetHello *rep;
		packet *cpy;
		
		rep=helloUnmarshal(us,p);
		if (rep == NULL)
		{
			fprintf(stderr, "node %d: helloUnmarshal failed after succeeding once?!?\n",
				us->addr & 0xFF);
		}
		else
		{
			rep->route[rep->routelen++]=us->addr;
#ifdef DEBUG_INTERIM2
//		fprintf(stderr,"node %d: repeating packet from %d TTL %d hopcount %d\n",us->addr & 0xFF, p->src & 0xFF, p->ttl, p->hopcount);
#endif
			cpy=helloMarshal(us,rep);
			cpy->hopcount=p->hopcount+1;
			cpy->ttl=p->ttl;
			cpy->src=p->src;
			cpy->type=PACKET_INTERIM2_HELLO;
			cpy->dst=NODE_BROADCAST;
			packetSend(us,cpy,PACKET_REPEAT);
			packetFree(cpy);
			helloFree(rep);
		}
	}

	// XXX On receiving the first packet (after missing some), we reset sequentialCount to 0,
	// and we don't set NEIGHBOR_HEARD until sequentialCount > minSequentialCount, so it seems
	// we need minSequentialCount+2 consecutive HELLOs to set HEARD.  This is contrary to the
	// live.conf comments, which indicate that only minSequentialCount are needed.  -dkindred
	if (hp->sequencenum==(n->cluster->lastsequencenum+1))    /* Is this packet the next sequential one after the previous packet? */
	{
		n->cluster->sequentialCount++;   /* OK, we have this many packets in a row (none dropped)   */
		if ((n->cluster->sequentialCount>us->cluster->minSequentialCount) || 
			(n->hopcount>1))     /* If its a multi-hop neighbor, don't require the minimum sequence.  (Should it require that the forwarding node has met the minimum sequence?   */  
		{
			n->flags|=NEIGHBOR_HEARD;
		}
	}
	else
		n->cluster->sequentialCount=0;

	n->cluster->lastsequencenum=hp->sequencenum;

	n->lastheard=us->manet->curtime;
	n->cluster->level=hp->level;
	n->cluster->ttl=p->ttl;
	n->cluster->totSymmetricNeighbors=hp->totSymmetricNeighbors;
	/* note that, while n->cluster->routelen is "generally" equal to
	 * hopcount-1, this is not always true, because we update the route
	 * on every received hello, while hopcount is set to the lowest 
	 * hopcount of any recently received hello (within the hopcount window)
	 */
	n->cluster->routelen=hp->routelen;
	memcpy(n->cluster->route,hp->route,sizeof(hp->route[0])*hp->routelen);

	n->cluster->coordinator=hp->coordinator;
	n->cluster->desiredCoordinator=hp->desiredCoordinator;
	n->cluster->rootflag=hp->rootflag;
	n->cluster->rootDistance=hp->rootDistance;
	n->cluster->root=hp->root;
        n->cluster->rootGroupRadius=hp->rootGroupRadius;

	{	/* update hopcount.  We use the smallest hopcount of the last INTERIM2_HOPCOUNTWINDOW packets received */
		/* Note that duplicate hellos are discarded above, so this calculation 
		 * applies only to the first copy received.  So we could in theory receive
		 * hellos from this neighbor via a 3-hop path consistently earlier than 
		 * another copy we get from some 2-hop path, and we'd call the neighbor
		 * hopcount 3.  But maybe that's just as well, since the 3-hop path
		 * seems to be working better.
		 */
		int j;

		n->cluster->hopCountWindow[n->cluster->hopCountPos++]=p->hopcount;
		if (n->cluster->hopCountPos>=us->cluster->hopCountWindowSize)
			n->cluster->hopCountPos=0;
		if(n->cluster->hopCountNum<(us->cluster->hopCountWindowSize-1))
			n->cluster->hopCountNum++;

		int oldHopCount=n->hopcount;
		n->hopcount=n->cluster->hopCountWindow[0];
		for(j=1;j<n->cluster->hopCountNum;j++)    /* find minium hopcount...  */
			if (n->cluster->hopCountWindow[j]<n->hopcount )
				n->hopcount=n->cluster->hopCountWindow[j];
		assert(n->hopcount > 0);

		/* hop count changed, check dynamic root group radius */
		if(oldHopCount!=n->hopcount) 
		{
			DYNRG_RADIUS_DEBUG("neighbor hop count changed, updating dynamic root group\n"); 
			updateRootGroupRadius=1;
		}
	}

	if (us->cluster->root == n->addr)
	{
		
		/* this node is our root -- use its value of rootGroupRadius */
		if (n->cluster->rootGroupRadius >= 0)
		{
			us->cluster->rootGroupRadius =
				n->cluster->rootGroupRadius;
		}
		else
		{
			/* root doesn't say what to use -- revert to default */
			us->cluster->rootGroupRadius =
				us->cluster->rootGroupRadiusMax;
		}
		checkRootGroupMembership(us);

		/* update rootDistance */
		if (us->cluster->rootDistance != n->hopcount)
		{
#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: %s() HELLO from root (%u), changing rootDistance from %d to %d\n",
				us->addr & 0xFF, 
				__func__,
				us->cluster->root & 0xFF,
				us->cluster->rootDistance,
				n->hopcount);
#endif
			setRootDistance(us, n->hopcount);
		}
		/* It's possible this node wasn't in the neighbor list when we
		 * chose it as root, in which case NEIGHBOR_ROOT might not
		 * be set yet. */
		n->flags |= NEIGHBOR_ROOT;
	}

	if (n->cluster->coordinator==us->addr)   /* do they feel they are our child?  */
		n->flags|=NEIGHBOR_CHILD;
	else
		n->flags&=(~NEIGHBOR_CHILD);

	if (!us->cluster->locked)
	{
		if ((us->clusterhead==n) && (n->cluster->level<=us->level))     /* our coordinator has surrendered its coordinator-hood */
		{
#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: set selecttime, we outrank CH\n",us->addr & 0xFF);
#endif
			us->cluster->selecttime=us->manet->curtime;
		}
		if ((us->clusterhead==n) && ((n->hopcount) > ((us->level>0)?(us->cluster->ttls[us->level]):1)))   /* our coordinator has moved too far away  */
		{
#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: set selecttime, CH too far away.  dist= %d our TTL= %d\n",us->addr & 0xFF,n->hopcount,us->cluster->ttls[us->level]);
#endif
			us->cluster->selecttime=us->manet->curtime;
		}
	}


#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: hello from %d lists %d neighbors:",
		us->addr & 0xFF, p->src & 0xFF, hp->numNeighbors);
#endif
	int hears=0;
	for(i=0;i<hp->numNeighbors;i++)
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr," %d", hp->addressList[i] & 0xFF);
#endif
		if (hp->addressList[i]==us->addr)
			hears=1;
	}
#ifdef DEBUG_INTERIM2
	fprintf(stderr," [hears=%d]\n", hears);
#endif

	if (hears)
	{
		if (!(n->flags & NEIGHBOR_HEARS))     /* completely new neighbor...  */
		{
			/* we only want to set the select time if we have no coordinator, and are not a coordinator
			**  EXCEPT watch the level stuff...
			*/

#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: new neighbor %d level %d\n",us->addr & 0xFF, n->addr & 0xFF,n->cluster->level);
#endif
			if (us->clusterhead==NULL && !us->cluster->locked)
			{
				if (((us->level==0) && (n->hopcount==0))   /* if its a new multi-hop neighbor, and we're level 0, then its not going to change our choice of coordinator, so do not set select time  */
					|| (us->level>0))
				{
#ifdef DEBUG_INTERIM2
					fprintf(stderr,"node %d: set selecttime  leaf CH is multi-hop\n",us->addr & 0xFF);
#endif
					us->cluster->selecttime=us->manet->curtime + us->cluster->timeSelect;
				}
			}
		}
		n->flags|=NEIGHBOR_HEARS;
	}
	else
		n->flags&=~NEIGHBOR_HEARS;

	if (us->clusterhead && p->src == us->clusterhead->addr
	    && !us->cluster->locked)
	{
		/* hello from our clusterhead -- we may have a root change */
		setRoot(us);
	}

#if 0
	if (hp->helloPayloadLen)
		fprintf(stderr, "node %d: helloRec(): got hello from node %d with %d-byte payload: \"%.*s\"\n",
			us->addr & 0xFF,
			n->addr & 0xFF,
			hp->helloPayloadLen,
			hp->helloPayloadLen,
			hp->helloPayload);
#endif
	if (us->cluster->helloCallback)
		(us->cluster->helloCallback)(us,us->clusterhead, n, hp->helloPayload,hp->helloPayloadLen);

	helloFree(hp);

	if(updateRootGroupRadius) setRootGroupRadius(us); 
}


static void helloSend(manetNode *us, void *data)
{
	packet *p;
	packetHello *hp, realhp;
	int noise;
	neighbor *n;
	int ttl;

	/* first, reschedule...  */
	noise=(int)(RAND_U01()*100.0)-50;
	/* if we are a leaf, and have a CH, send at 1/2 or 1/4 normal rate?
	 */
	timerSet(us,(eventCallback*)helloSend,(((us->level==0) && (us->clusterhead!=NULL))?us->cluster->leafscale:1)*us->cluster->timeHello+noise,NULL);
#if 0
	fprintf(stderr, "node %u: helloSend(): curtime= %lld - scheduling next hello for %d + %d = %d ms from now\n",
		us->addr & 0xFF, us->manet->curtime, 
		us->cluster->timeHello, noise, 
		us->cluster->timeHello + noise);
#endif

	selectCoordinator(us);

	hp=&realhp;
	hp->numNeighbors=0;
	hp->totSymmetricNeighbors=0;
	hp->sequencenum=us->cluster->sequencenum++;
	hp->desiredCoordinator=(us->cluster->desiredCoordinator)?(us->cluster->desiredCoordinator->addr):(NODE_BROADCAST);
	hp->coordinator=(us->clusterhead)?(us->clusterhead->addr):(NODE_BROADCAST);
	hp->level=us->level;
	hp->routelen=0;
	hp->rootflag=us->rootflag;
	hp->helloPayload=us->cluster->helloPayload;
	hp->helloPayloadLen=us->cluster->helloPayloadLen;
	hp->helloPayloadMalloced=0;
	ttl=us->level==0?1:us->cluster->ttls[us->level];

	if(us->rootflag) ttl=us->cluster->maxTTL; 

	assert(ttl > 0);

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if ((n->flags & NEIGHBOR_HEARD) && (n->flags & NEIGHBOR_HEARS) && ((n->hopcount-1) <= ttl))
		{
			/* XXX check for overflow of addressList[] */
			hp->addressList[hp->numNeighbors++]=n->addr;
			if (n->level <= us->level)
				hp->totSymmetricNeighbors++;
		}
	}

	hp->root=us->cluster->root;
	hp->rootDistance=us->cluster->rootDistance;
	if (hp->rootflag)
	{
		hp->rootGroupRadius=us->cluster->rootGroupRadius;
	}
	else
	{
		hp->rootGroupRadius=-1;
	}

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if ((n->flags & NEIGHBOR_HEARD) && (!(n->flags & NEIGHBOR_HEARS) && ((n->hopcount-1) <= ttl)))
			hp->addressList[hp->numNeighbors++]=n->addr;
	}

	p=helloMarshal(us,hp);
	p->type=PACKET_INTERIM2_HELLO;
	p->dst=NODE_BROADCAST;
	p->ttl=ttl;

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: sending level %d HELLO TTL= %d coord= %d desired= %d root= %d rootflag= %d rootDistance= %d totSymmetricNeighbors= %d  , neighbors= ",us->addr & 0xFF, hp->level,p->ttl,hp->coordinator & 0xFF,hp->desiredCoordinator & 0xFF,hp->root & 0xFF, hp->rootflag, hp->rootDistance,hp->totSymmetricNeighbors);

	for(int i=0;i<hp->numNeighbors;i++)
		fprintf(stderr," %d", hp->addressList[i] & 0xFF);
	fprintf(stderr,"\n");
#endif
	
	packetSend(us,p,PACKET_ORIGIN);
	packetFree(p);
}

/* Called regularly, to remove old entries from the neghbor list
 */
static void helloTimeout(manetNode *us, void *data)
{
	neighbor *n,*d;

	timerSet(us,helloTimeout,us->cluster->timeHelloTimeout/2,NULL);

	n=us->neighborlist;
	while(n)
	{
		d=n;
		n=n->next;

		/* If this neighbor is a leaf, multiply the timeout window by leafscale  */

		if ((us->manet->curtime - d->lastheard) > (((d->cluster?(d->cluster->level==0):0)?us->cluster->leafscale:1)*us->cluster->timeHelloTimeout))
		{

			if (d->flags & NEIGHBOR_HEARS)
			{
				/* lost a neighbor. */

				/* update root group radius */
				DYNRG_RADIUS_DEBUG("Lost a neighbor, updating dynamic root group\n"); 
				setRootGroupRadius(us); 
			}

			/* was it our coordinator?   */
			if (d==us->clusterhead && !us->cluster->locked)
			{
				setClusterHead(us, NULL, "CH went away");
				us->cluster->selecttime=us->manet->curtime;
			}

			/* was it our root? */
			if (d->addr == us->cluster->root)
			{
				setRootDistance(us, 0xFF);
#ifdef DEBUG_INTERIM2
				fprintf(stderr,"node %d: root (%d) is no longer neighbor, rootDistance set to 0x%x\n",
					us->addr & 0xFF, d->addr & 0xFF, us->cluster->rootDistance);
#endif
			}
			if (d==us->clusterhead)
			{
				/* node is still our clusterhead (because clustering is locked). 
				 * don't delete -- just mark as "distant". */
				d->hopcount=255;
				d->flags &= ~NEIGHBOR_HEARS;
				d->flags &= ~NEIGHBOR_HEARD;
			}
			else
			{
				neighborDelete(us,d);
			}
		}
	}

	selectCoordinator(us);

        /* XXX more efficient to just search for a single level-0 neighbor
         * rather than counting all and checking ==0.  but profile before
	 * optimizing.  -dkindred */
	if (neighborCount(us,0)==0 && !us->cluster->locked)        /* did we just lose all our level 0 neighbors?  We're floating...  */
	{
		if (!us->cluster->floatingrootflag)
		{
			fprintf(stderr, "node %d: lost all my neighbors; becoming floating root\n", us->addr & 0xFF);
		}
		us->rootflag=INTERIM2_ROOT;
		us->color=0;
		setLevel(us,0);
		setClusterHead(us, NULL, "floating root");
		us->cluster->floatingrootflag=1;
		setRoot(us);
	}

#if 0
	int haveroot=0;
	for(n=us->neighborlist;n;n=n->next)
		if (n->cluster->rootflag)
			haveroot=1;

/* This increases our availability greatly, but also increases our traffic greatly 
 * (since we have a lot more packets being sent at maxTTL), and causes a lot of 
 * (transient) partitions with multiple roots
 */
		/* if we no longer see a root, and have no coordinator...  then lets be root */
	if ((!haveroot) && (us->clusterhead==NULL) && !us->cluster->locked)
	{
		us->rootflag=INTERIM2_ROOT;
		us->cluster->ttls[us->level]=us->cluster->maxTTL;
		setRoot(us);
	}
#endif
}

/* Called by simulation code once on each node to setup 
*/
void nodeInit(manetNode *us)
{
	int i;
	us->clusterhead=NULL;
        us->neighborlist=NULL;
	us->level = -1;		/* so setLevel below sees a "change" */

	us->cluster=(clusteringState*)malloc(sizeof(clusteringState));
	us->cluster->hopCountWindowSize=configSetInt(us->manet->conf,"interim_hopCountWindow",INTERIM2_HOPCOUNTWINDOW);
	us->cluster->desireList=(neighbor**)malloc(sizeof(neighbor*)*INTERIM2_MAXNEIGHBORS);
	us->cluster->useList=(neighbor**)malloc(sizeof(neighbor*)*INTERIM2_MAXNEIGHBORS);
	us->cluster->assignedList=(neighbor**)malloc(sizeof(neighbor*)*INTERIM2_MAXNEIGHBORS);
	us->cluster->floatingrootflag=1;
	us->cluster->ttls[0]=1;
	for (i = 1; i < MAXLEVEL; i++) us->cluster->ttls[i] = -1;
        setLevel(us,0);
	us->cluster->sequencenum=107;
	us->cluster->root=us->addr;
	us->cluster->locked=0;
	us->cluster->desiredCoordinator=NULL;
	us->cluster->maxTTL=configSetInt(us->manet->conf,"interim_maxTTL",55);
	us->cluster->helloCallback=NULL;
	us->cluster->helloPayload=NULL;
	us->cluster->helloPayloadLen=0;

	us->cluster->timeSelect=configSetInt(us->manet->conf,"interim_timeSelect",INTERIM2_TIME_SELECT);
	us->cluster->timeHello=configSetInt(us->manet->conf,"interim_timeHello",INTERIM2_TIME_HELLO);
	us->cluster->timeHelloTimeout=configSetInt(us->manet->conf,"interim_timeHelloTimeout",INTERIM2_TIME_HELLO_TIMEOUT);
	us->cluster->timeUndesired=configSetInt(us->manet->conf,"interim_timeUndesired",INTERIM2_UNDESIREDTIME);
	us->cluster->timePromotion=configSetInt(us->manet->conf,"interim_timePromotion",INTERIM2_PROMOTIONTIME);
	us->cluster->minSequentialCount=configSetInt(us->manet->conf,"interim_minsequentialcount",INTERIM2_MINSEQUENTIALCOUNT);
	us->cluster->leafscale=configSetInt(us->manet->conf,"interim_leafscale",INTERIM2_LEAFSCALE);
	us->cluster->enrouteCHEnable=configSetInt(us->manet->conf,"interim_enroutechenable",INTERIM2_ENROUTECHENABLE);
	us->cluster->gmClusterEnable=configSetInt(us->manet->conf,"interim_gmclusterenable",0);
	us->cluster->gmClusterTTL=configSetInt(us->manet->conf,"interim_gmclusterttl",us->cluster->maxTTL/2);
	us->cluster->rootGroupRadiusMax=configSetInt(us->manet->conf,"interim_rootgroupradiusmax",INTERIM2_ROOTGROUPRADIUS);
	us->cluster->rootGroupRadius=us->cluster->rootGroupRadiusMax;
	us->cluster->rootGroupMinSize=configSetInt(us->manet->conf,"interim_rootgroupsizemin",INTERIM2_ROOTGROUPMINSIZE); 
#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: maxTTL= %d\n",us->addr&0xFF,us->cluster->maxTTL);
	fprintf(stderr,"node %d: timeSelect= %d\n",us->addr&0xFF,us->cluster->timeSelect);
	fprintf(stderr,"node %d: timeHello= %d\n",us->addr&0xFF,us->cluster->timeHello);
	fprintf(stderr,"node %d: timeHelloTimeout= %d\n",us->addr&0xFF,us->cluster->timeHelloTimeout);
	fprintf(stderr,"node %d: timeUndesired= %d\n",us->addr&0xFF,us->cluster->timeUndesired);
	fprintf(stderr,"node %d: timePromotion= %d\n",us->addr&0xFF,us->cluster->timePromotion);
	fprintf(stderr,"node %d: minSequentialCount= %d\n",us->addr&0xFF,us->cluster->minSequentialCount);
	fprintf(stderr,"node %d: leafscale= %d\n",us->addr&0xFF,us->cluster->leafscale);
	fprintf(stderr,"node %d: enrouteCHEnable= %d\n",us->addr&0xFF,us->cluster->enrouteCHEnable);
	fprintf(stderr,"node %d: gmClusterEnable= %d\n",us->addr&0xFF,us->cluster->gmClusterEnable);
	fprintf(stderr,"node %d: gmClusterTTL= %d\n",us->addr&0xFF,us->cluster->gmClusterTTL);
	fprintf(stderr,"node %d: rootGroupRadius= %d\n",us->addr&0xFF,us->cluster->rootGroupRadius);
#endif

	us->cluster->rootDistance=255; /* will be changed in setRootDistance */
	setRootDistance(us, 0);
	us->cluster->lastDesiredTime=us->manet->curtime;
	us->cluster->lastPromotionTime=us->manet->curtime;
	us->cluster->selecttime=us->manet->curtime+us->cluster->timeSelect;
	us->cluster->nailedroot=0;

	if (us->cluster->maxTTL<1)
		us->cluster->maxTTL=255;
	us->rootflag=INTERIM2_ROOT;
	setRoot(us);

#ifdef USE_RNG
	if (rnd==NULL)
	{
		long seed = configSearchInt(us->manet->conf,"interim_randomseed");
		rnd= new RNG(seed);
# ifdef DEBUG_INTERIM2
		if (seed)
		{
			fprintf(stderr, "interim2 %s: initializing RNG object with seed %ld\n",
				__func__, seed);
		}
		else
		{
			fprintf(stderr, "interim2 %s: initializing RNG object with nondeterministic seed\n",
				__func__);
		}
# endif
	}
#else
# ifdef DEBUG_INTERIM2
	fprintf(stderr, "interim2 %s: using drand48() as RNG\n", __func__);
# endif
#endif

	manetPacketHandlerSet(us, PACKET_INTERIM2_HELLO, helloRec);
	manetPacketHandlerSet(us, PACKET_INTERIM2_ROOT, rootRec);
	manetPacketHandlerSet(us, PACKET_STATEVECT, stateRec);
	manetPacketHandlerSet(us, PACKET_STATEVECT_NOFLOOD, stateRec);
	manetPacketHandlerSet(us, PACKET_STATEVECT_FLOOD, stateRec);

	helloSend(us,NULL);
	helloTimeout(us,NULL);

#ifdef MODULE_GMCLUSTER
	if (us->cluster->gmClusterEnable)
		gmclusterNodeInit(us);
#endif 

}

void nodeFree(manetNode *us)
{
	free(us->cluster->assignedList);
	free(us->cluster->desireList);
	free(us->cluster->useList);
	free(us->cluster);
}

static neighbor *listsearch(neighbor *list[], int len, ManetAddr key)
{
	int i;

	for(i=0;i<len;i++)
		if (list[i]->addr==key)
			return list[i];
	return NULL;
}


static neighbor *listEvaluate(manetNode *us, neighbor *list[], int len)
{

#ifdef MODULE_GMCLUSTER		
	int numMatch=0;
	int i;

/* useList is all neighbors which may be our CH.
 * if gmclusterEnable is true, and our ttl (us->cluster->ttls[us->level]) is less than us->cluster->gmclusterTTL,
 *  then we only want to use neighbors which are in our gmcluster group
 * otherwise, (if we are a CH outside the gmcluster radius, or we have no gmcluster neighbrs)
 * we will use any on the use list.
 */

	
	if ((us->cluster->gmClusterEnable) && (us->cluster->ttls[us->level] < us->cluster->gmClusterTTL))    /* if our TTL is within the gmcluster radius */
	{
		i=0;		/* this is like a selection sort */
		while(i<len)
		{
			if (gmclusterInMyGroup(us,list[i]))
			{
				neighbor *tmp;

				tmp=list[i];                 /* swap the current entry with the numMatch'th entry  */
				list[i]=list[numMatch];
				list[numMatch]=tmp;
				
				numMatch++;
			}
			i++; 
		}
	}

	/* now the list is partitioned into two groups, the first 
	 * numMatch neighbors match our group, and the rest do not.
	 * numMatch may be 0...
	 */
	 if (numMatch>0)
	 {
		qsort(list,numMatch,sizeof(list[0]),neighborCompare);
		return list[0];
	 }
#endif
/*
 * If not using gmcluster module, just return the normally sorted list.
 * If it is defined and we get here, then we have no group neighbors so we
 * should just act normally
 */
	qsort(list,len,sizeof(list[0]),neighborCompare);
	return list[0];
			
}

/* Return the first neighbor en route to 'n' that is on the given list,
 * or 'n' if none is found.  If 'recurse', repeat the search with the
 * newly selected node.
 */
static neighbor *
findNeighborEnRoute(manetNode *us, neighbor* n, 
		    neighbor *list[], int listLen,
		    const char *roleForDebugMessage,
		    int recurse)
{
	int i;
	for(i=n->cluster->routelen-1;i>=0;i--)
	{
		neighbor *alternate;
		/* The "alternate->hopcount < n->hopcount" check is necessary
		 * here to ensure that the search for alternates terminates.  
		 * The node's hopcount is generally equal to routelen-1, except
		 * that hopcount is set (in helloRec()) to the lowest hopcount
		 * in the last few (hopCountWindowSize) hellos, while the 
		 * route is updated with every (first received copy of a) 
		 * hello.
		 */
		if ((alternate=listsearch(list,listLen,n->cluster->route[i]))!=NULL
		    && alternate->hopcount < n->hopcount)
		{
#ifdef DEBUG_INTERIM2
			fprintf(stderr,
				"node %d: switching %s from %d (hopcount %d routelen %d) to en-route node %d (hopcount %d routelen %d)\n",
				us->addr & 0xFF,
				roleForDebugMessage,
				n->addr & 0xFF,
				n->hopcount,
				n->cluster->routelen,
				alternate->addr & 0xFF,
				alternate->hopcount,
				alternate->cluster->routelen);
#endif
			return (recurse ? 
				findNeighborEnRoute(us, 
						    alternate,
						    list,
						    listLen,
						    roleForDebugMessage,
						    recurse)
				: alternate);
		}
	}
	return n;
}

/* choose a coordinator and desired coordinator.  
 *  called by helloSend if selecttime is in the past.
 */
static void selectCoordinator(manetNode *us)
{
	int numSymNeighbors[MAXLEVEL];
	int totalSymNeighbors;
	neighbor *n;
	neighbor *desiredCoord;
	neighbor *coord;
	int neighborDesiresUs;
	int i;
	CommunicationsPositionWeight key,*cpw;
	int maxdesirelevel;

	neighbor **desireList=us->cluster->desireList;
	int numDesire=0;
	neighbor **useList=us->cluster->useList;
	int numUse=0;
	neighbor **assignedList=us->cluster->assignedList;
	int numAssigned=0;

	memset(numSymNeighbors,0,sizeof(numSymNeighbors));
	neighborDesiresUs=0;
	desiredCoord=NULL;
	coord=NULL;
	maxdesirelevel=0;
	totalSymNeighbors=0;

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: calling selectCoordinator,  level %d Using %d Selected %d Desires %d  select time %lld  desired= %d maxdesirelevel= %d\n",us->addr & 0xFF,us->level,PRINTCOORD(us->clusterhead),PRINTCOORD(coord),PRINTCOORD(desiredCoord),us->cluster->selecttime,neighborDesiresUs,maxdesirelevel);
#endif

	if (us->cluster->locked)
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr, "node %d: skipping selectCoordinator since clustering state is locked\n", us->addr & 0xFF);
#endif
		return;
	}

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		for(i=0;i<COORDINATOR_MAXVAL;i++)     /* copy all the API's position weight values into our struct */
		{
			key.addr=n->addr;
			key.position=(IDSPositionType)i;
			cpw=packetApiPositionWeightSearchList(us,&key);
			if (cpw && cpw->weight != COMMUNICATIONSPOSITIONWEIGHT_DEFAULT)
			{
				n->cluster->weights[i]=cpw->weight;
// fprintf(stderr,"node %d: neighbor %u.%u.%u.%u  has non-default weight of %d in position %s\n",us->addr & 0xFF, PRINTADDR(n->addr),n->cluster->weights[i],idsPosition2Str(key.position)); 
			}
			else
				n->cluster->weights[i]=COMMUNICATIONSPOSITIONWEIGHT_DEFAULT;
// fprintf(stderr,"node %d: looking at neighbor %u.%u.%u.%u  position= %s weight= %d\n",us->addr & 0xFF, PRINTADDR(n->addr),idsPosition2Str(key.position),n->cluster->weights[i]);
		}


		if ((n->flags & NEIGHBOR_HEARS)    /* n is a symmetric neighbor...  */
			&& ((n->hopcount-1) <= us->cluster->ttls[us->level]))   /* and n is within our search TTL...  */
		{
			numSymNeighbors[n->cluster->level]++;
			totalSymNeighbors++;

// fprintf(stderr,"node %d: looking at neighbor %u.%u.%u.%u  hopcount= %d  ROOTweight= %d\n",us->addr & 0xFF, PRINTADDR(n->addr),n->hopcount, n->cluster->weights[COORDINATOR_ROOT]);
			if (n->cluster->weights[COORDINATOR_ROOT]==COMMUNICATIONSPOSITIONWEIGHT_ASSIGNED)
				if (((us->level==0) && (n->hopcount==1)) || 
					(us->level>0))
					if (numAssigned<INTERIM2_MAXNEIGHBORS) assignedList[numAssigned++]=n;

			if ((n->cluster->desiredCoordinator==us->addr) &&      /* n desires us...  */
				(n->cluster->weights[COORDINATOR_ROOT]!=COMMUNICATIONSPOSITIONWEIGHT_ASSIGNED))
			{
				neighborDesiresUs=1;
				if (n->cluster->level>maxdesirelevel)
					maxdesirelevel=n->cluster->level;
			}

			if (n->cluster->level >= us->level)
				if (n->cluster->weights[COORDINATOR_NEIGHBORHOOD]!=COMMUNICATIONSPOSITIONWEIGHT_BANNED)
				{
					if (((us->level==0) && (n->hopcount==1)) || 
						(us->level>0))
						if (numDesire<INTERIM2_MAXNEIGHBORS) desireList[numDesire++]=n;
				}

			if (n->cluster->level>us->level)
			{
				if (n->cluster->weights[COORDINATOR_NEIGHBORHOOD]!=COMMUNICATIONSPOSITIONWEIGHT_BANNED)
				{
					if (
						((n->hopcount-1) <= us->cluster->ttls[us->level])
						&& (((us->level==0) && (n->hopcount==1)) || (us->level>0))
						)
						if (numUse<INTERIM2_MAXNEIGHBORS) useList[numUse++]=n;
				}
			}
		}
	}

	if (numUse>0)
	{
		coord=listEvaluate(us,useList,numUse);

#ifdef DEBUG_INTERIM2
		{
			int ii;
			fprintf(stderr,"node %d: use list ",us->addr & 0xFF);
			for(ii=0;ii<numUse;ii++)
				fprintf(stderr,"%d ",useList[ii]->addr & 0xFF);
			fprintf(stderr,"\n");
		}
#endif
	}
	else
		coord=NULL;

	if (numDesire>0)
	{
		desiredCoord=listEvaluate(us,desireList,numDesire);

#ifdef DEBUG_INTERIM2
		{
			int ii;
			fprintf(stderr,"node %d: desire list ",us->addr & 0xFF);
			for(ii=0;ii<numDesire;ii++)
				fprintf(stderr,"%d ",desireList[ii]->addr & 0xFF);
			fprintf(stderr,"\n");
		}
#endif
	}
	else
		desiredCoord=NULL;

	if (numAssigned>0)    /* if we have any neighbors which are assigned to be root nodes...  */
	{
		int j;
		desiredCoord=listEvaluate(us,assignedList,numAssigned);

		/* Check if we are also assigned, and can break the tie...  */
		for(j=numAssigned-1;j>=0;j--)
			if (((us->level==0) && (assignedList[j]->hopcount==1)) || 
				(us->level>0))
				coord=assignedList[j];

#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: assigned list ",us->addr & 0xFF);
		for(j=0;j<numAssigned;j++)
			fprintf(stderr,"%d ",assignedList[j]->addr & 0xFF);
		fprintf(stderr,"\n");
#endif
	}

	key.addr=us->addr;
	key.position=COORDINATOR_ROOT;
	cpw=packetApiPositionWeightSearchList(us,&key);
	if ((cpw) && (cpw->weight==COMMUNICATIONSPOSITIONWEIGHT_ASSIGNED))     /* if we are assigned to root...  */
	{
		/* make the lowest assigned root neighbor the clusterhead */
		neighbor *lowestAddr=NULL;
		for(int j=0;j<numAssigned;j++) 
		{
			if(!lowestAddr) lowestAddr=assignedList[0];
			else lowestAddr=lowestAddr->addr<assignedList[j]->addr?lowestAddr:assignedList[j];
		}
		if(lowestAddr) coord=lowestAddr; 

		if ((numAssigned>0) && (coord->addr < us->addr))
		{	/* If we're assigned root, but there is another assigned root, and its ID is less than ours, then we use it.    */
		}
		else
		{
			us->rootflag=INTERIM2_ROOTNAILED;
			setClusterHead(us, NULL, "assigned to root");
			us->color=0;
			us->cluster->selecttime=0;

			coord=NULL;
			desiredCoord=NULL;
		}
		us->cluster->ttls[us->level]=us->cluster->maxTTL;   /* override TTL...  */
	}


#if 0

	/* setting the coordinator flag is really setting the level to the greatest level were this node wins the election.
	   clearing the coordinator flag is dropping the level, when we lose an election.  */


	/* FIXME:
		what is the correct definition of number of symmetric neighbors?
			The total number for all the levels the node is present in?  
			Current def is that for level n, the numsymneighbors is the total for level 0 to n

		TTL explosion:  what about when a node increases its TTL too quickly, and ends up spanning 3/4 of the manet?
		desperate node: again, high TTL caused by being too far away.
		doomed-to-be-asymmetric: node A has increased its TTL till it found a neighbor.  node B has increased its TTL further, and is now visible to node A, but node A's TTL is insufficient to reach node B.
		desire loops:  if node A desires node B and node B desires node A, then they both climb in level without any coordinators actually being selected.

		Lets say node A and node B start at level 0, node A is desired by another node, and node A 
		desires node B.  So, both node A and node B become level 1.  Then node B becomes level 2,
		with node A as its child.  But, node B was desired by node A at level 0, and now that 
		node B is a coordinator at level 1, it dosn't desire node A at level 0.  So node B is 
		no longer desired at 0, and fails its election to level 1.  Node B really should stay
		as a level 2 coordinator, even though it is not desired at level 0.

		greatest degree for a level is sometimes /way/ accross the manet.  How can we put the
		distance in there too?  HELLO packets have a hopcount...

	while
	{
		determine desired coordinator for this level.  It is irrespective of coordinator flags   (ours or neighbors)

		is select time in the past?
		{
			determine values of coordinator flag for this level
				if we are desired, and there are no neighbors with coordinator flags, then set
				if we are not desired, and havn't been for DESIREDTIME, then clear

			if coordinator flag is set, then we are coordinator, and go onto next level.
			if coordinator flag is clear, then select a neighbor which has its coordinator flag set
			if there are no neighbors with coordinator flag set, we are orphaned
		}
	}

	*/

#endif

	if (us->cluster->enrouteCHEnable)
	{
		/* Is there another desirable node on the route to our desired node?  Perhaps we should desire it instead.
		 */
		if (desiredCoord)
		{
			desiredCoord =
				findNeighborEnRoute(us,
						    desiredCoord,
						    desireList,
						    numDesire,
						    "desiredCoord",
						    1);
		}
		/* likewise for our actual clusterhead */
		if (coord)
		{
			coord = findNeighborEnRoute(us,
						    coord,
						    useList,
						    numUse,
						    "coord",
						    1);
		}
	}

#if 1
	/* if desired coord is our level, and we have more symmetric neighbors than it does, then don't desire it.  */
	if ((desiredCoord) && (desiredCoord->cluster->level==us->level) && (totalSymNeighbors>desiredCoord->cluster->totSymmetricNeighbors))
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: no desire, rule 1\n",us->addr & 0xFF);
#endif
		desiredCoord=NULL;
	}


	/* if desired coord is our level, and number of neighbors is equal, and our addr is less than desired neighbor's, don't desire it */
	/* This is intended for the case where a partition has only 2 nodes in it, to avoid their both desiring the other */
	if ((desiredCoord) && (desiredCoord->cluster->level==us->level) && (totalSymNeighbors==desiredCoord->cluster->totSymmetricNeighbors) && (us->addr < desiredCoord->addr)
//	 && (totalSymNeighbors==1)   /* Why was this added in version 1.24?  for node 129 in test3.  */
//	 && (rootDistance!=0xFF) && (desiredCoord->cluster->rootDistance!=0xFF) && (rootDistance < desiredCoord->cluster->rootDistance)   /* Why was this added in version 1.24?  for node 129 in test3.  */
		)
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: no desire, rule 2\n",us->addr & 0xFF);
#endif
		desiredCoord=NULL;
	}

#endif

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: total neighbors %d\n",us->addr & 0xFF, totalSymNeighbors);
	for(i=0;i<(us->level+2);i++)
	{
		fprintf(stderr,"node %d: level %d neighbors %d: ",us->addr & 0xFF, i,numSymNeighbors[i]);
		for(n=us->neighborlist;n!=NULL;n=n->next)
			if ((n->flags & NEIGHBOR_HEARS) && (n->cluster->level==i))
				fprintf(stderr," %d(%d)%c",n->addr & 0xFF,n->cluster->totSymmetricNeighbors,(n->cluster->desiredCoordinator==us->addr)?'d':' ');
		fprintf(stderr,"\n");
	}
#endif
	
#ifdef DEBUG_INTERIM2
	fprintf(stderr,"coord= %u level= %d neighbors= %d selecttimeset= %d\n",coord ? coord->addr & 0xFF : 255,us->level,numSymNeighbors[us->level],(us->cluster->selecttime>0) && (us->cluster->selecttime < us->manet->curtime));

	if (desiredCoord)
	{
	  fprintf(stderr,"node %d: desiredCoord= %u.%u.%u.%u coordroutelen= %d coordroute= ",
		  us->addr & 0xFF, PRINTADDR(desiredCoord->addr),
		  desiredCoord->cluster->routelen);
	  for(i=0;i<desiredCoord->cluster->routelen;i++)
	  	fprintf(stderr," %d ",desiredCoord->cluster->route[i] & 0xFF);
	  fprintf(stderr,"\n");
	}
#endif

	/* do we have no neighbors at our level?  increase TTL, and clear select time, unless TTL is big enough for us to be root   */

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"coord= %u level= %d neighbors= %d selecttimeset= %d\n",coord ? coord->addr & 0xFF : 255,us->level,numSymNeighbors[us->level],(us->cluster->selecttime>0) && (us->cluster->selecttime < us->manet->curtime));
#endif
	if ((coord==NULL) && (numSymNeighbors[us->level]==0)  && (us->cluster->selecttime>0) && (us->cluster->selecttime < us->manet->curtime))
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: no peers TTL= %d\n",us->addr & 0xFF,us->cluster->ttls[us->level]);
#endif	
		if (us->cluster->ttls[us->level]<us->cluster->maxTTL)
		{
			if (us->cluster->ttls[us->level])
				us->cluster->ttls[us->level]*=2;
			else
				us->cluster->ttls[us->level]=1;
			if (us->cluster->ttls[us->level]>us->cluster->maxTTL)
				us->cluster->ttls[us->level]=us->cluster->maxTTL;
#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: set selecttime time, no peers and increasing TTL to %d\n",us->addr & 0xFF,us->cluster->ttls[us->level]);
#endif
			us->cluster->selecttime=us->manet->curtime + us->cluster->timeSelect;
		}
		else
		{
#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: we're root!\n",us->addr & 0xFF);
#endif
			us->rootflag=INTERIM2_ROOT;
			setClusterHead(us, NULL, "becoming root");
			us->color=0;
			us->cluster->selecttime=0;
		}
	}

	if (neighborDesiresUs)
	{
		int newlevel=maxdesirelevel+1;
		int haveneighbor=1;

		for(i=0;i<maxdesirelevel;i++)
			if (numSymNeighbors[i]==0)
				haveneighbor=0;

		if ((haveneighbor) && ((us->manet->curtime - us->cluster->lastPromotionTime) > us->cluster->timePromotion))
		{
			setLevel(us, newlevel);	/* sets lastPromotionTime */
			if ((us->clusterhead) && (us->clusterhead->cluster->level<=us->level))   /* did we just outrank our clusterhead?  */
			{
				setClusterHead(us, NULL, "outranked CH");
				us->cluster->selecttime=us->manet->curtime;
			}
		}
		us->cluster->lastDesiredTime=us->manet->curtime;
	}
	else
	{
		if ((us->manet->curtime - us->cluster->lastDesiredTime) > us->cluster->timeUndesired)
		{
			/* We're no longer desired by anyone, and
			 * it's been a while since we were.
			 */
			assert(maxdesirelevel == 0);
			setLevel(us, 0);
			us->cluster->lastDesiredTime=us->manet->curtime;
		}
	}
	assert(us->cluster->ttls[us->level] >= 0);

	if ((coord) && 
		(((us->cluster->selecttime>0) && (us->cluster->selecttime < us->manet->curtime)) ||
		(us->rootflag)))
	{
		setClusterHead(us, coord, "selected");
		us->cluster->selecttime=0;
#if 1
		if ((us->rootflag) && (us->rootflag!=INTERIM2_ROOTNAILED))
		{
#ifdef DEBUG_INTERIM2
			fprintf(stderr,"node %d: surrendering root to %d\n",us->addr & 0xFF, us->clusterhead->addr & 0xFF);
#endif
			rootSend(us,us->clusterhead);
		}
#endif
		us->rootflag=0;                /* Surrender our rootness */

	}
	us->cluster->desiredCoordinator=desiredCoord;

	/* set our root based on rootflag and what our clusterhead says */
	setRoot(us);

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d:  level %d Using %d Selected %d Desires %d  select time %lld  desired= %d maxdesirelevel= %d\n",us->addr & 0xFF,us->level,PRINTCOORD(us->clusterhead),PRINTCOORD(coord),PRINTCOORD(desiredCoord),us->cluster->selecttime,neighborDesiresUs,maxdesirelevel);
#endif
} /* end selectCoordinator() */


static char *neighborStr(char *buf, size_t buflen, neighbor *n)
{
	if (n)
	{
		snprintf(buf, buflen, "%d.%d.%d.%d", PRINTADDR(n->addr));
	}
	else
	{
		snprintf(buf, buflen, "NONE");
	}
	return buf;
}

/* set us->clusterhead 
 */
static void setClusterHead(manetNode *us, neighbor *ch, const char *reason)
{
	if (us->clusterhead != ch)
	{
		char newbuf[80];
		char oldbuf[80];
		fprintf(stderr, "node %d: time= %lld clusterhead changed from %s to %s (%s)\n",
			us->addr & 0xFF, 
			us->manet->curtime,
			neighborStr(oldbuf, sizeof(oldbuf), us->clusterhead),
			neighborStr(newbuf, sizeof(newbuf), ch),
			reason ? reason : "unknown reason");
		us->clusterhead=ch;
	}
}


/* set the values of
 *   us->cluster->root
 *   us->cluster->rootDistance
 *   for each neighbor, n->flags (set/clear NEIGHBOR_ROOT)
 * based on us->rootflag, us->clusterhead->rootflag, us->clusterhead->root
 */
static void setRoot(manetNode *us)
{
	if (us->rootflag)
	{
		/* we are root */
		setRootTo(us, us->addr);
	}
	else if (us->clusterhead)
	{
		/* we're not root.  see what our parent has to say */
		if (us->clusterhead->cluster->rootflag)
		{
			/* parent is root */
			setRootTo(us, us->clusterhead->addr);
		}
		else if (us->clusterhead->cluster->root == us->addr)
		{
			/* uh oh, parent thinks we're root!
			 * This can happen, e.g., when we're 
			 * surrendering our rootness to parent.
			 * Should be transient.  For now, we'll
			 * just call our parent root, and hope
			 * they will come to agree soon.
			 */
#ifdef DEBUG_INTERIM2
			fprintf(stderr, "node %d: parent (%d) thinks we're root. we'll call parent root.\n",
				us->addr & 0xFF, us->clusterhead->addr & 0xFF);
#endif
			setRootTo(us, us->clusterhead->addr);
		}
		else
		{
			/* parent is not root -- use parent's root */
			setRootTo(us, us->clusterhead->cluster->root);
		}
	}
	else
	{
		/* We're not root but have no clusterhead!
		 * I'm not sure if this can happen.  -dkindred
		 */
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: rootflag=0 and no clusterhead!\n",
			us->addr & 0xFF);		
#endif
		setRootTo(us, NODE_BROADCAST);
	}

	/* check invariants */

	/* us->rootflag is set iff us->cluster->root is us->addr */
	if (us->rootflag) assert(us->cluster->root == us->addr);
	if (us->cluster->root==us->addr) assert(us->rootflag);

	/* rootDistance == 0 iff we are root */
	if (us->cluster->root == us->addr) assert(us->cluster->rootDistance == 0);
	if (us->cluster->rootDistance == 0) assert(us->cluster->root == us->addr);

#ifdef DEBUG_INTERIM2
	fprintf(stderr,"node %d: setRoot: root=%u distance=%d\n",
		us->addr & 0xFF, us->cluster->root & 0xFF, us->cluster->rootDistance);
#endif
}

/* set the values of
 *   us->cluster->root
 *   us->cluster->rootDistance
 *   for each neighbor, n->flags (set/clear NEIGHBOR_ROOT)
 * to reflect the new root specified
 */
static void setRootTo(manetNode *us, ManetAddr newRoot)
{
	if (us->cluster->root != newRoot)
	{
		int newRootDistance=0xFF;
		int oldRootNeighbors=0;
		neighbor *nbr=NULL;
		neighbor *rootNbr=NULL;
		for(nbr=us->neighborlist;nbr!=NULL;nbr=nbr->next) 
		{
			if (nbr->flags & NEIGHBOR_ROOT) 
			{
				oldRootNeighbors++;
				assert(nbr->addr == us->cluster->root);
				assert(nbr->hopcount == us->cluster->rootDistance);
				nbr->flags &= (~NEIGHBOR_ROOT);
			}
			if (nbr->addr == newRoot)
			{
				nbr->flags |= NEIGHBOR_ROOT;
				rootNbr = nbr;
				newRootDistance = nbr->hopcount;
			}
		}
		if (us->cluster->root == NODE_BROADCAST
		    || us->cluster->root == us->addr)
		{
			assert(oldRootNeighbors == 0);
		}
		else
		{
			/* It's currently possible for root to not be
			 * a neighbor (our clusterhead said this is
			 * the root, but we haven't seen a HELLO).
			 * So oldRootNeighbors==0 is possible here.
			 * XXX may want to change this and create a neighbor record
			 *     for the root if none exists. 
			 */
			assert(oldRootNeighbors <= 1);
		}
		/* reset to our default root group radius unless root says 
		 * otherwise */
		us->cluster->rootGroupRadius = 
			us->cluster->rootGroupRadiusMax;
		if (rootNbr && rootNbr->cluster->rootGroupRadius >= 0)
		{
			us->cluster->rootGroupRadius =
				rootNbr->cluster->rootGroupRadius;
		}
		if (newRoot == us->addr) 
		{
			newRootDistance = 0;
		}
		fprintf(stderr, "node %d: time= %lld root changed from %d (distance %d) to %d (distance %d)\n",
			us->addr & 0xFF,
			us->manet->curtime,
			us->cluster->root & 0xFF, 
			us->cluster->rootDistance,
			newRoot & 0xFF,
			newRootDistance);
		us->cluster->root = newRoot;
		setRootDistance(us, newRootDistance);
	}
}

static int isRootGroupEligible(manetNode *us, ManetAddr addr)
{
	int eligible = 0;
	const CommunicationsPositionWeight *cpw;
	CommunicationsPositionWeight key;
	memset(&key,0,sizeof(key));
	key.addr=us->addr;
	key.position=COORDINATOR_ROOTGROUP;
	cpw=packetApiPositionWeightSearchList(us,&key);
	if (cpw)
	{
		/* currently, default is ineligible.
		 * note that HierarchyClient.h knows about this
		 * policy too. Should probably have a configurable
		 * minimum threshold and allow API clients to get it. 
		 */
		eligible = (cpw->weight > COMMUNICATIONSPOSITIONWEIGHT_DEFAULT);
	}
	return eligible;
}

/* return zero if no change, 1 if joining root group, -1 if leaving */
static int checkRootGroupMembership(manetNode *us)
{
	/* XXX we're only checking eligibility when distance to root changes
	 *     or the root group radius changes.
	 *
	 *     The API allows clients to modify posweight info, which could
	 *     change our eligibility, but there's currently no callback
	 *     mechanism to notify interim2 that this has happened.
	 */
	int ret = 0;
	int newRootGroupFlag = (us->cluster->rootDistance
				<= us->cluster->rootGroupRadius);
	int eligible = isRootGroupEligible(us, us->addr);
	newRootGroupFlag = newRootGroupFlag && eligible;
	if (newRootGroupFlag != us->rootgroupflag)
	{
		ret = (newRootGroupFlag ? 1 : -1);
#ifdef DEBUG_INTERIM2
		const char *joinString = 
			(newRootGroupFlag
			 ? " (joining root group)"
			 : " (leaving root group)");

		fprintf(stderr,"node %d: rootDistance= %d rootGroupRadius= %d",
			us->addr & 0xFF,
			us->cluster->rootDistance,
			us->cluster->rootGroupRadius);

		if (us->cluster->locked)
		{
			fprintf(stderr, " (NOT %s because clustering is locked)\n",
				joinString);
		}
		else
		{
			fprintf(stderr, " (%s)\n", joinString);
		}
		fprintf(stderr, "\n");
#endif
	}
	if (us->cluster->locked)
	{
		/* locked down -- no clustering changes allowed */
		ret = 0;
	}
	else
	{
		us->rootgroupflag = newRootGroupFlag;
	}
	return 1;
}

static void setRootDistance(manetNode *us, int newDistance)
{
	/* always set us->cluster->root before calling setRootDistance() */
	if (newDistance == 0)
	{
		assert(us->cluster->root == us->addr);
	} 
	else
	{
		assert(us->cluster->root != us->addr);
	}

	if (newDistance != us->cluster->rootDistance)
	{
#ifdef DEBUG_INTERIM2
		fprintf(stderr,"node %d: rootDistance changed %d to %d\n",
			us->addr & 0xFF,
			us->cluster->rootDistance,
			newDistance);
#endif
		us->cluster->rootDistance = newDistance;
		checkRootGroupMembership(us);
	}
}

/* returns >0 if a is better than b
 */
static int neighborCompare(const void *bp,const void *ap)
{
	const neighbor *a=*(const neighbor * const *)ap;
	const neighbor *b=*(const neighbor * const *)bp;

		/* any node is better than NULL  */
	if ((a!=NULL) & (b==NULL))
		return 1;
	if ((a==NULL) & (b!=NULL))
		return -1;

	if (a->cluster->weights[COORDINATOR_ROOT] > b->cluster->weights[COORDINATOR_ROOT])
		return 1;
	if (b->cluster->weights[COORDINATOR_ROOT] > a->cluster->weights[COORDINATOR_ROOT])
		return -1;

#if 0
/* Do not use...  prevents a root from being selected when degree is very high
*/
	/* if we have root distance data...  closer to root is more desireable
	 */
	if ((a->cluster->rootDistance < 0xFF) && (b->cluster->rootDistance < 0xFF))
	{
		if (a->cluster->rootDistance < b->cluster->rootDistance)
			return 1;
		if (b->cluster->rootDistance < a->cluster->rootDistance)
			return -1;
	}
#endif
		/* node with greater number of symmetric neighbors is better */

	if (a->cluster->totSymmetricNeighbors > b->cluster->totSymmetricNeighbors)    
		return 1;
	if (b->cluster->totSymmetricNeighbors > a->cluster->totSymmetricNeighbors)    
		return -1;

#if 1
		/* degree ties.  OK...  closer node is better  */

	if (a->hopcount < b->hopcount)
		return 1;
	if (b->hopcount < a->hopcount)
		return -1;
#endif

		/* hopcount ties.  OK...  least id is better.  */
	if (a->addr > b->addr)
		return 1;
	else
		return -1;
	
	return 0;
}

void interim2PayloadCallbackSet(manetNode *us, helloHello *helloCallback)
{
	us->cluster->helloCallback=helloCallback;
}

void interim2PayloadSet(manetNode *us, unsigned char *payload, int payloadLen)
{
	us->cluster->helloPayload=payload;
	us->cluster->helloPayloadLen=payloadLen;
	if (us->cluster->helloPayloadLen==0)
		us->cluster->helloPayload=NULL;
}

static void setRootGroupRadius(manetNode *us)
{
	/* If not using dynamic radius or we are not root, do not set radius */
	if(us->cluster->rootGroupMinSize<=0 || us->cluster->root!=us->addr) 
	{
		DYNRG_RADIUS_DEBUG("We are not root or dynamic root group radius is not configured\n"); 
		return;
	}

	us->cluster->rootGroupRadius=1; 
	while(us->cluster->rootGroupRadius<=us->cluster->rootGroupRadiusMax)
	{
		int numRootGroupEligibleInRadius=0; 	
		int numRootGroupEligible=0; 	
		int numRootGroupNodesInRadius=0;
		int totalNeighborNumInRadius=0;
		int totalNeighborNum=0;

		DYNRG_RADIUS_DEBUG("Looking for RG at least %d eligible nodes at radius %d\n", 
				us->cluster->rootGroupMinSize, us->cluster->rootGroupRadius); 

		for(neighbor *n=us->neighborlist; n!=NULL; n=n->next)
		{
			totalNeighborNum++; 

			if(isRootGroupEligible(us, n->addr)) numRootGroupEligible++;

			if(n->hopcount<=us->cluster->rootGroupRadius) 
			{
				totalNeighborNumInRadius++; 

				if(isRootGroupEligible(us, n->addr)) numRootGroupEligibleInRadius++;
				if(n->cluster->rootDistance<=us->cluster->rootGroupRadius) numRootGroupNodesInRadius++;
			}
		}

		DYNRG_RADIUS_DEBUG("Within radius %d: RG nodes: %d, RG eligible nodes: %d, total neighbors: %d. Within all neighbors: nodes %d, rg eligible %d.\n", 
				us->cluster->rootGroupRadius, numRootGroupNodesInRadius, numRootGroupEligibleInRadius, 
				totalNeighborNumInRadius, totalNeighborNum, numRootGroupEligible); 

		if(numRootGroupNodesInRadius>=us->cluster->rootGroupMinSize)
		{
			DYNRG_RADIUS_DEBUG("Found enough root group nodes %d in root group radius %d.\n", 
					numRootGroupNodesInRadius, us->cluster->rootGroupRadius); 
			break;
		}
		if(us->cluster->rootGroupRadius==us->cluster->rootGroupRadiusMax)
		{
			DYNRG_RADIUS_DEBUG("Hit maximum root group radius, %d, found  %d root group nodes when I want at least %d\n", 
					us->cluster->rootGroupRadiusMax, numRootGroupNodesInRadius, us->cluster->rootGroupMinSize); 
			break;
		}
		if(numRootGroupEligibleInRadius==numRootGroupEligible)
		{
			DYNRG_RADIUS_DEBUG("All eligible root group nodes will be in the root group at radius %d. Stopping radius expansion.\n", 
					numRootGroupEligibleInRadius); 
			break;
		}

		us->cluster->rootGroupRadius++; 

		if(us->cluster->rootGroupRadius>2)
		{
			fprintf(stderr, "WARNING: root group radius is greater than 2. This means that the root group may be larger than the minimum size stated in the conf file\n"); 
			fprintf(stderr, "WARNING: 	as the root currently does not keep track of non-neighbor nodes with hopcount > 2\n"); 
		}
	}
}
