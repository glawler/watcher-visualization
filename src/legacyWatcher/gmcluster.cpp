#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h> 		/* for INT_MAX */
#include <sstream>		/* for stream logging */
#include <errno.h>
#include <netinet/in.h>		/* for htonl() ntohl() */

#include <sys/socket.h>		/* for int_ntoa() */
#include <netinet/in.h>		/* for int_ntoa() */
#include <arpa/inet.h>		/* for int_ntoa() */

#include <sys/time.h>		/* for gettimeofday */

#include "gmcluster.h"
#include "rng.h"
#include "graphics.h"
#include "node.h"
#include "minilogger.h" 		
#include "nodeVelocityData.h"
#include "watcherGPS.h"
#include "marshal.h"


#ifdef MODULE_GMCLUSTER
	#include "interim2.h"
#else
	#include "hello.h"
#endif

using namespace std; 

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: gmcluster.cpp,v 1.24 2007/08/29 04:33:39 dkindred Exp $"; 

/* A clustering algorithm gets the field us->cluster for its private per-node data
 * It will be inited to NULL, and can be malloced in nodeInit() (below).
 */
typedef struct groupClusterState
{
	int antennaRadius; 
	nodeVelocityData *velocity; 

	#define HELLOGPSDATAOFFSET 0
	#define HELLOGPSDATALEN (4*5)
	#define HELLOGIDDATAOFFSET HELLOGPSDATALEN
	#define HELLOGIDDATALEN (2)
	unsigned char helloPacketData[HELLOGPSDATALEN + HELLOGIDDATALEN];  // lat=4, lon=4, alt=4, timestamp=(4*2), gid=2. 

	double acceptableLET; 	// minimum acceptable LET for neighbors. 

	int groupIdFD; 		// Used to talk to group displayer, will go away when Tommy fixes API. 

	unsigned short curGid;   	// current group id. 
	int gidHeartbeatInterval; 	// how often we send or expect to recv a gid packet. 
	int heartbeatTimeoutFactor;     // factor * interval == when we reset the gid. 
	time_t lastGidHeartbeat; 	// last time we heard or sent a gid packet. 

	unsigned short curGidSeqno; 
	unsigned short seqno; 

	int routeHopCntComputeInterval; // How often to call nodeGetHopAndNbrCount() to compute hop count and nbr count.
	int curRouteHopCnt;		// filled in by nodeGetHopAndNbrCount() every routeHopCntComputeInterval seconds. 
	int curRouteNbrCnt; 		// filled in by nodeGetHopAndNbrCount() every routeHopCntComputeInterval seconds.
	
#define MAX_GM_NODES 256
	unsigned char localGroup[MAX_GM_NODES/8]; // nbrs who are LET acceptable
#define HIGH_ORDER_BIT (1<<7)

} groupClusterState;

/* A clustering algorithm gets the field neighbor->cluster for private per-neighbor data
 * When a neighbor is malloced, neighbor->cluster is set to NULL.  
 * when a neighbor is freed, if it is non-NULL, it will be freed as well.
 */
typedef struct groupClusteringNeighbor
{
	nodeVelocityData *velocity; 
	unsigned short curGid; 

} groupClusteringNeighbor;

typedef struct gidPacket
{
#define SIZEOFGIDPACKET (sizeof(unsigned short) + sizeof(unsigned short))
	unsigned short gid; 
	unsigned short seqno; 

} gidPacket; 

/*
 * Forward decls
 */
/*
 * Hello API functions
 */
static void gotHello(manetNode *us, neighbor *ch, neighbor *src, const unsigned char *payload, int payloadLen);

/* 
 * Others
 */
static void gpsPacketArrive(manetNode *us, packet *p);			// handle GPS message
static void groupBuild(manetNode *us, void *data); 			// build bitmap of acceptable neghbors according to current velocity data
static void startClusteringAlgorithm(manetNode *us, void *data); 	// clear data, set intial localgroup bitmap
static void groupIdDisplay(manetNode *us); 				// Display the current GID somehow (default in the watcher as label). 
static void logLocalGroup(manetNode *us, ostringstream &logStr); 	  // write a space separted list of bits (by index) which are set to logStr 
static void sendGidPacket(manetNode *us, const gidPacket &gid, const bool &forward); 			 // send out a gid packet
static int unmarshalGidPacket(const char *data, gidPacket &gid); 
static void gidPacketRecv(manetNode *us, packet *p); 			// handle rec't of gid packet

static void nodeGetHopAndNbrCount(manetNode *us, void *data); 		// periodically fill out hop count and nbr count acc'd to kernel routing tables.

/* If a clustering algorithm wants random numbers, it needs its own private random number generator
 */
//static RNG *rnd=NULL;

/* 
 * This function is not currently called. If it was, it would set the CH to the neighbor with the greatest LET. 
 */
void setClusterHead(manetNode *us)
{
	LOGTRACE_ENTER("setClusterHead(...)"); 

	if(!us->neighborlist) 
	{
		LOGTRACE_EXIT("setClusterHead(...)");
		return; 
	}

	neighbor *n = NULL; 

	if(us->groupCluster->curGid == (us->addr & 0xFF)) 	// we are group leadaer - all bow before the mighthy Zod!
	{
		for(n = us->neighborlist; n != NULL; n = n->next) { n->clusterheadflag = 0; }

		// set us to ROOT here, if that is possible.

		for(n = us->neighborlist; n != NULL; n = n->next) { n->flags &=~NEIGHBOR_PARENT; n->flags |= NEIGHBOR_CHILD; } 

		// check for non-group nbrs to connect to. 
		long double maxLet = -1;
		for(n = us->neighborlist; n != NULL; n = n->next)
		{
			if(n->groupCluster && n->groupCluster->velocity)
			{
				double let = us->groupCluster->velocity->getLinkExpirationTime(us->groupCluster->antennaRadius, *n->groupCluster->velocity);
				if(let > maxLet && let < us->groupCluster->acceptableLET)
				{
					if(us->clusterhead)
					{
						us->clusterhead->flags &= ~NEIGHBOR_PARENT;
						us->clusterhead->clusterheadflag = 0;
					}
					us->clusterhead = n; 
					us->clusterhead->clusterheadflag = 1;
					us->clusterhead->flags |= NEIGHBOR_PARENT;
					LOGINFO("CHALGO: Set CH to %d because we are group leader and have non-group nbrs", us->clusterhead->addr & 0xFF);
					LOGTRACE_EXIT("setClusterHead(...)");
					return;
				}
			}
		}

		us->clusterhead = NULL; 
		LOGINFO("CHALGO: Set CH to NULL because we are group leader and have no non-group nbrs"); 
		LOGTRACE_EXIT("setClusterHead(...)");
		return; 
	}

	// is one of our nbrs group leader? 
	for(n = us->neighborlist; n != NULL; n = n->next) 
	{
		if((n->addr & 0xFF) == us->groupCluster->curGid)
		{
			if(us->clusterhead) 
			{ 
				us->clusterhead->flags &= ~NEIGHBOR_PARENT; 
				us->clusterhead->clusterheadflag = 0;	
			}
			us->clusterhead = n; 
			us->clusterhead->clusterheadflag = 1;
			us->clusterhead->flags |= NEIGHBOR_PARENT;

			LOGINFO("CHALGO: Set CH to %d because it is our nbr and it is group leader", (us->clusterhead->addr & 0xFF));	
			LOGTRACE_EXIT("setClusterHead(...)");
			return;
		}
	}

	// group leader is not one hop nbr. Choose greatest LET of all nbrs. 
	long double maxLet = -1; 
	us->clusterhead = NULL;
	for(n = us->neighborlist; n != NULL; n = n->next)
	{
		n->clusterheadflag = 0; 
		n->flags &=~NEIGHBOR_PARENT; 
		if(n->groupCluster && n->groupCluster->velocity)
		{
			double let = us->groupCluster->velocity->getLinkExpirationTime(us->groupCluster->antennaRadius, *n->groupCluster->velocity);
			if(let > maxLet)
			{
				us->clusterhead = n;
				maxLet = let;
			}
		}
	}

	if(us->clusterhead)
	{
		LOGINFO("CHALGO: Set CH to %d", (us->clusterhead->addr & 0xFF)); 
		us->clusterhead->clusterheadflag = 1; 
		us->clusterhead->flags |= NEIGHBOR_PARENT; 

		LOGINFO("CHALGO: Set CH to %d because it is my greatest LET nbr", (us->clusterhead->addr & 0xFF));
		LOGTRACE_EXIT("setClusterHead(...)");
		return;
	}

	// no nbrs. what to do? 
	if(us->clusterhead)
	{ 
		us->clusterhead->flags &= ~NEIGHBOR_PARENT; 
		us->clusterhead->clusterheadflag = 0;
	}
	us->clusterhead = NULL;
	LOGINFO("CHALGO: Set CH to NULL because I have no nbrs"); 


	LOGTRACE_EXIT("setClusterHead(...)");
}
#ifndef MODULE_GMCLUSTER
void nodeInit(manetNode *us)
{
	gmclusterNodeInit(us); 
}
#endif /* !MODULE_GMCLUSTER */


/* Called by simulation code on each node to setup 
 */
void gmclusterNodeInit(manetNode *us)
{
	LOGTRACE_ENTER("gm::nodeInit(...)"); 

	us->clusterhead=NULL;
	us->neighborlist=NULL;
	us->level=0;
	us->rootflag=0;
	us->groupCluster=(struct groupClusterState*)malloc(sizeof(struct groupClusterState)); 
	assert(us->groupCluster); 
	memset(us->groupCluster, 0, sizeof(us->groupCluster)); 

	us->groupCluster->groupIdFD=-1; 
	us->groupCluster->curGid=MAX_GM_NODES+1; 

	int posHist, minLET;
	struct { const char *str; int *val; int def; } confs[] = 
	{
		{ "gmcluster_positionHistory", (int*)&posHist, GMCLUSTER_POSITIONHISTORY_DEFAULT },
		{ "antennaradius", (int*)&us->groupCluster->antennaRadius, GMCLUSTER_ANTENNARADIUS_DEFAULT },
		{ "gmcluster_acceptableLinkExpirationTime", &minLET, GMCLUSTER_ACCEPTABLE_LINK_EXPIRATION_TIME },
		{ "gmcluster_gidHeartbeatInterval", (int*)&us->groupCluster->gidHeartbeatInterval, DEFAULT_GID_HEARTBEAT_INTERVAL },
		{ "gmcluster_gidHeartbeatTimeoutFactor", (int*)&us->groupCluster->heartbeatTimeoutFactor, DEFAULT_GID_HEARTBEAT_TIMEOUT_FACTOR }	
	}; 
	for(size_t i = 0; i < sizeof(confs)/sizeof(confs[0]); i++)
	{
		long value;
		if(0 == (value = configSearchInt(us->manet->conf, confs[i].str)))
		{
			LOGWARN("Unable to parse config file value %s, using default %d", confs[i].str, confs[i].def); 
			value = confs[i].def;
		}
		*confs[i].val = value; 
		LOGDEBUG("Config: using %d for %s", *confs[i].val, confs[i].str); 
	}

	assert(posHist!=0);
	us->groupCluster->velocity = new nodeVelocityData(posHist); 
	us->groupCluster->acceptableLET = minLET; 

	/* setup logging level */
	const char *logLevel = configSearchStr(us->manet->conf, "gmcluster_loglevel"); 
	if(!logLevel) logLevel = "debug"; 
	SetLogLevel(logLevel); 
	LOGINFO("Loglevel set to %s", logLevel); 

	/* setup logging timestamp style - one of no, short, or long */
	const char *timeStampStyle = configSearchStr(us->manet->conf, "gmcluster_logtimestamp"); 
	if(!timeStampStyle) timeStampStyle = "long";
	SetLogTimestampType(timeStampStyle); 
	LOGINFO("LogTimeStampStyle set to %s", timeStampStyle); 

#ifdef MODULE_GMCLUSTER
	interim2PayloadCallbackSet(us,(helloHello*)gotHello);
	interim2PayloadSet(us, (unsigned char*)&us->groupCluster->helloPacketData, sizeof(us->groupCluster->helloPacketData));
#else 
	helloInit(us,NULL,NULL,(helloHello*)gotHello);
	helloPayloadSet(us, (unsigned char*)&us->groupCluster->helloPacketData, sizeof(us->groupCluster->helloPacketData));  
#endif 

	
	// listen for GPS messages. 
	// manetPacketHandlerSet(us, IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS, gpsPacketArrive);
	manetPacketHandlerSet(us, PACKET_GROUP_MOBILTY_GPS, gpsPacketArrive);
	manetPacketHandlerSet(us, PACKET_GROUP_ID_ASSERTION, gidPacketRecv); 

	timerSet(us, startClusteringAlgorithm, 3*1000, NULL);  // let the intial GPS stuff get sent around

	// Dont do this anymore. We are not using it. 
	// We keep the code around though in case it is useful in the future. 
	// timerSet(us, nodeGetHopAndNbrCount, 5*1000, NULL); 

	srand(time(NULL)); 

	LOGTRACE_EXIT("gm::nodeInit(...)"); 
}

#ifndef MODULE_GMCLUSTER
void nodeFree(manetNode *n)
{
	gmclusterNodeFree(n);
}
#endif /* !MODULE_GMCLUSTER */

void gmclusterNodeFree(manetNode *n)
{
	LOGTRACE_ENTER("nodeFree(0x%x)", (unsigned int)n); 
	if(n && n->groupCluster)
	{
		if(n->groupCluster->velocity) delete n->groupCluster->velocity;
		if(n->groupCluster->groupIdFD>0) 
		{ 
			close(n->groupCluster->groupIdFD); 
			n->groupCluster->groupIdFD=-1; 
		}

		free(n->groupCluster); 
	}
	LOGTRACE_EXIT("nodeFree(0x%x)", (unsigned int)n); 
}

void gpsPacketArrive(manetNode *us, packet *p)
{
	LOGTRACE_ENTER("gpsPacketArrive(manetNode *us, packet *p)"); 

	WatcherGPS *gpsInfo = watcherGPSUnmarshal(p->data, p->len); 
	if(gpsInfo && us->groupCluster && us->groupCluster->velocity)
	{
		LOGINFO("GPS: Got GPS Info from hierachy node %d: %f, %f, %f, %lld", p->src & 0xff, gpsInfo->lat, gpsInfo->lon, gpsInfo->alt, gpsInfo->time); 

		if(p->src == us->addr) 	// it's us, update local velocity. 
		{
			LOGDEBUG("GPS: GPS info is about us");
			LOGDEBUG("GROUPALGO: Updating local velocity with GPS data (%f, %f, %f) @ %llu", gpsInfo->lat, gpsInfo->lon, gpsInfo->alt, gpsInfo->time);
			us->groupCluster->velocity->addDataPoint(nodePosition(gpsInfo->lat, gpsInfo->lon, gpsInfo->alt, gpsInfo->time));

			memcpy(&us->groupCluster->helloPacketData[HELLOGPSDATAOFFSET], p->data, p->len); 
		}
		else // we should not hear GPS messages from the other nodes. 
		{
			LOGINFO("Ignoring GPS packet from other node: %d", p->src & 0xFF);
		}
		free(gpsInfo); 
	}
	else
	{
		LOGWARN("GPS: Unable to parse hierachy gps packet"); 
	}

	LOGTRACE_EXIT("gpsPacketArrive(manetNode *us, packet *p)"); 
}

void gotHello(manetNode *us, neighbor *ch, neighbor *src, const unsigned char *payload, int payloadLen)
{
	LOGTRACE_ENTER("gotHello(...)"); 

	if(src && NULL == src->groupCluster)
	{
		// create the neighbor specfic clustering data if it does not exist.
		
		// mixing malloc() and new() can't be very good.
		src->groupCluster = (groupClusteringNeighbor*)malloc(sizeof(groupClusteringNeighbor));
		memset(src->groupCluster, 0, sizeof(groupClusteringNeighbor));
		src->groupCluster->velocity = new nodeVelocityData(us->groupCluster->velocity->getMaxHistorySize()); 
		LOGDEBUG("MEM: Created neighbor specific clustering data at 0x%x for nbr %d", (unsigned int)src->groupCluster, src->addr & 0xFF);
	}

	if(NULL == payload || 0 == payloadLen)
	{
		LOGDEBUG("Ignoring hello from node %d as it's got an empty payload", src->addr & 0xFF); 
		LOGTRACE_EXIT("gotHello(...)");
		return;

	}

	WatcherGPS *gpsInfo = watcherGPSUnmarshal((const void*)payload, payloadLen);

	if(!gpsInfo)
	{
		LOGWARN("Unable to unmarshall GPS information in the hierarchy HELLO packet"); 
		LOGTRACE_EXIT("gotHello(...)"); 
		return; 
	}

	LOGINFO("GPS: Got GPS Info from hierachy node %d: %f, %f, %f, %lld", src->addr & 0xff, gpsInfo->lat, gpsInfo->lon, gpsInfo->alt, gpsInfo->time); 

	if(gpsInfo->time == 0)
	{
		LOGDEBUG("GPS: Ignoring empty GPS data from neighbor");
		return; 
	}

	if(src && src->groupCluster && src->groupCluster->velocity)
	{
		nodePosition position(gpsInfo->lat, gpsInfo->lon, gpsInfo->alt, gpsInfo->time); 
		src->groupCluster->velocity->addDataPoint(position); 

		nodePosition pos;
		if(us->groupCluster->velocity->lastKnownPosition(pos) && (us->addr & 0xFF) != (src->addr & 0xFF))
		{
			nodeVelocity A, B; 
			src->groupCluster->velocity->lastKnownVelocity(B); 
			us->groupCluster->velocity->lastKnownVelocity(A); 
			LOGDEBUG("GROUPALGO: update: %d <--> %d: LET: %Lf dist: %Lf relVel: %Lf other(avg spd: %Lf avg dir: %Lf instant spd: %Lf, instant dir:%Lf) "
					"us(avg spd: %Lf avg dir: %Lf instant spd: %Lf, instant dir:%Lf)", 
					(us->addr & 0xFF), 
					(src->addr & 0xFF), 
					us->groupCluster->velocity->getLinkExpirationTime(us->groupCluster->antennaRadius, *src->groupCluster->velocity), 
					pos.distanceFrom(position), 
					A.relativeVelocity(B),
					src->groupCluster->velocity->getAverageSpeed(),
					src->groupCluster->velocity->getAverageDirection(),
					B.speed.speed,
					B.direction.direction,
					us->groupCluster->velocity->getAverageSpeed(),
					us->groupCluster->velocity->getAverageDirection(),
					A.speed.speed,
					A.direction.direction); 
		}
	}
	
	const unsigned char *bufPtr = &(((const unsigned char*)payload)[HELLOGIDDATAOFFSET]); 
	unsigned short nbrGid;
	UNMARSHALSHORT(bufPtr, nbrGid); 
	src->groupCluster->curGid = nbrGid; 

	LOGINFO("GROUPALGO: Neighbor %d claims to be in group %d according to recv'd HELLO", src->addr & 0xff, nbrGid); 

// 	setClusterHead(us); 

	LOGTRACE_EXIT("gotHello(...)");
}

void groupBuild(manetNode *us, void *data)
{
	LOGTRACE_ENTER("GROUPALGO: groupBuild(...)"); 

	timerSet(us, groupBuild, us->groupCluster->gidHeartbeatInterval*1000, NULL); 	

	neighbor *n = NULL; 
	unsigned int leastId = (us->addr & 0xFF); 

	memset(&us->groupCluster->localGroup, 0, sizeof(us->groupCluster->localGroup)); 

	for(n = us->neighborlist; n != NULL; n = n->next)
	{
		if(n->groupCluster && n->groupCluster->velocity)
		{
			double let = us->groupCluster->velocity->getLinkExpirationTime(us->groupCluster->antennaRadius, *n->groupCluster->velocity); 
			if(let > us->groupCluster->acceptableLET)
			{
				LOGDEBUG("GROUPALGO: added %d.%d.%d.%d to set of acceptable neighbors", PRINTADDR(n->addr)); 
				us->groupCluster->localGroup[(n->addr & 0xFF)/8] |= HIGH_ORDER_BIT >> ((n->addr & 0xFF) % 8); 

				leastId = leastId > (n->addr & 0xFF) ? n->addr & 0xFF : leastId; 
			}
		}
	}

	// we are always in our own group
	us->groupCluster->localGroup[(us->addr & 0xFF)/8] |= (HIGH_ORDER_BIT >> ((us->addr & 0xFF) % 8)); 

	{
		ostringstream logStr; 
		logStr << "node " << (us->addr & 0xFF) << "'s current localGroup: "; 
		logLocalGroup(us, logStr); 
		LOGDEBUG("GROUPALGO: %s", logStr.str().c_str()); 
	}

	// if we haven't heard/sent a hb for too long, reset and start over. 
	time_t now = time(NULL); 
	if((us->groupCluster->lastGidHeartbeat + (us->groupCluster->heartbeatTimeoutFactor * us->groupCluster->gidHeartbeatInterval)) < now)  
	{
		us->groupCluster->lastGidHeartbeat=now;
		us->groupCluster->curGid=leastId; 

		unsigned char *dataPtr = &us->groupCluster->helloPacketData[HELLOGIDDATAOFFSET]; 
		MARSHALSHORT(dataPtr, us->groupCluster->curGid); 

		LOGDEBUG("GROUPALGO: heartbeat timeout resetting current gid to localhost id value, %d. Timeout at %lu", us->groupCluster->curGid, 
			(us->groupCluster->lastGidHeartbeat + (us->groupCluster->heartbeatTimeoutFactor * us->groupCluster->gidHeartbeatInterval))); 
	}

	if(leastId <= us->groupCluster->curGid)
	{
		LOGDEBUG("GROUPALGO: Setting gid to id %d as is it lower than our current gid of %d", leastId, us->groupCluster->curGid); 

		// send gid if we are the group leader
		if(leastId == (us->addr & 0xFF))
		{
			LOGDEBUG("GROUPALGO: Sending out heartbeat gid %d as we are the group leader", us->groupCluster->curGid); 
			gidPacket p;
			p.gid = leastId; 
			p.seqno = ++us->groupCluster->seqno;
			sendGidPacket(us, p, false); 
			us->groupCluster->curGidSeqno=p.seqno; 
		
			// if we are group leader, we are ROOT
			//if(us->clusterhead)
			//{
			//	us->clusterhead->flags &= ~NEIGHBOR_PARENT;
			//	us->clusterhead->clusterheadflag = 0;
			//}
			//us->clusterhead = NULL;
		}

		us->groupCluster->curGid = leastId;
		us->groupCluster->curGidSeqno = us->groupCluster->seqno; 
		us->groupCluster->lastGidHeartbeat=now; 

		unsigned char *dataPtr = &us->groupCluster->helloPacketData[HELLOGIDDATAOFFSET]; 
		MARSHALSHORT(dataPtr, us->groupCluster->curGid); 
	}

	LOGDEBUG("GROUPALGO Node %d currently thinks the gid is %d", (us->addr & 0xFF), us->groupCluster->curGid); 

	groupIdDisplay(us); 

	LOGTRACE_EXIT("GROUPALGO: groupBuild(...)"); 
}

void gidPacketRecv(manetNode *us, packet *thePacket)
{
	LOGTRACE_ENTER("GROUPALGO: groupAssertionRecv(...)"); 

	// we hear our own gidPacket, ignore. 
	if(us->addr == thePacket->src)
	{
		LOGDEBUG("GROUPALGO: Ignoring group assertion from self"); 
		LOGTRACE_EXIT("GROUPALGO: groupAssertionRecv(...)");
		return; 
	}

	// only recv messages from nbrs who are currently in our group.
	if(! (us->groupCluster->localGroup[(thePacket->src & 0xFF)/8] & (HIGH_ORDER_BIT >> ((thePacket->src & 0xFF) % 8))) )
	{
		LOGDEBUG("GROUPALGO: Node %d ignoring group assertion from non-group neighbor %d", (us->addr & 0xFF), (thePacket->src & 0xFF)); 
		LOGTRACE_EXIT("GROUPALGO: groupAssertionRecv(...)");
		return; 
	}

	// unpack the message. 
	gidPacket gidp; 
	unmarshalGidPacket((const char*)thePacket->data, gidp); 

	if(us->groupCluster->curGid >= gidp.gid)
	{
		if(us->groupCluster->curGid == gidp.gid) 
		{ 
			LOGDEBUG("GROUPALGO: Recv'd gid heartbeat of %d", gidp.gid); 
		}

		time_t now = time(NULL); 
		if(us->groupCluster->curGid > gidp.gid)  
		{ 
			LOGDEBUG("GROUPALGO: Recv'd gid of less than our current gid, changing ours from %d to %d", us->groupCluster->curGid, gidp.gid); 
			struct timeval tv;
			gettimeofday(&tv, 0);
			struct tm *tm = gmtime(&now); 
			LOGDEBUG("GROUPALGO: Node %d got gid packet from new group leader. Will timeout in %d seconds at %02d:%02d:%02d.%02ld (now=%lu)", (us->addr & 0xFF), 
					us->groupCluster->gidHeartbeatInterval * us->groupCluster->heartbeatTimeoutFactor, 
					tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 10000, now); 

		}

		if(us->groupCluster->curGidSeqno != gidp.seqno)
		{
			if(us->groupCluster->curGidSeqno < gidp.seqno)
			{
				LOGDEBUG("GROUPALGO: Node %d forwarding packet gid=%d seq no=%d rec'vd from node %d", (us->addr & 0xFF), gidp.gid, gidp.seqno, (thePacket->src & 0xFF)); 
				sendGidPacket(us, gidp, true); 
/* interim2 sets the CH now */
#if 0
				// Use the flooded mature of the gid packet to build a CH tree to the group leader.
				// The sender of the gid packet is closer than us to the group leader, so make it our clusterhead.
				for(neighbor *n = us->neighborlist; n != NULL; n = n->next)
				{
					if(n->addr == thePacket->src) 
					{
						if(us->clusterhead) 
						{ 
							us->clusterhead->flags &= ~NEIGHBOR_PARENT; 
							us->clusterhead->clusterheadflag = 0;	
						}
						us->clusterhead = n; 
						us->clusterhead->clusterheadflag = 1;
						us->clusterhead->flags |= NEIGHBOR_PARENT;

						LOGINFO("CHALGO: Set CH to %d because it is closer to our group leader than we are", (us->clusterhead->addr & 0xFF));	
						break;
					}
				}
#endif 			
			}
		}
		else
		{
			LOGDEBUG("GROUPALGO: Dropping gid=%d, seqno=%d packet from %d as we've seen it before", gidp.gid, gidp.seqno, (thePacket->src & 0xFF)); 
		}

		us->groupCluster->curGid = gidp.gid; 	
		us->groupCluster->curGidSeqno = gidp.seqno; 
		us->groupCluster->lastGidHeartbeat = now; 

		unsigned char *dataPtr = &us->groupCluster->helloPacketData[HELLOGIDDATAOFFSET]; 
		MARSHALSHORT(dataPtr, us->groupCluster->curGid); 
	}
	else
	{
		LOGDEBUG("GROUPALGO: Dropping gid=%d, seqno=%d packet from %d as it's gid is higher than ourn", gidp.gid, gidp.seqno, (thePacket->src & 0xFF)); 
	}

	LOGTRACE_EXIT("GROUPALGO: groupAssertionRecv(...)"); 
}

void sendGidPacket(manetNode *us, const gidPacket &gid, const bool &forward)
{	
	packet *p;
	p=packetMalloc(us,SIZEOFGIDPACKET); 
	p->type=PACKET_GROUP_ID_ASSERTION;
	p->dst=NODE_BROADCAST;
	p->ttl=1;
	unsigned char *dataptr=(unsigned char*)p->data; 
	MARSHALSHORT(dataptr, gid.gid); 
	MARSHALSHORT(dataptr, gid.seqno); 
	packetSend(us, p, forward ? PACKET_REPEAT : PACKET_ORIGIN);
	packetFree(p);
}

int unmarshalGidPacket(const char *data, gidPacket &gid)
{
	const unsigned char *ptr = (const unsigned char*)data; 
	UNMARSHALSHORT(ptr, gid.gid); 
	UNMARSHALSHORT(ptr, gid.seqno); 
	return SIZEOFGIDPACKET; 
}

void startClusteringAlgorithm(manetNode *us, void *data)
{
	LOGTRACE_ENTER("GROUPALGO: startClusteringAlgorithm(manetNode *us, void *data)"); 
	LOGINFO("GROUPALGO: starting clustering algo"); 

	groupBuild(us, NULL); 
	
	LOGTRACE_EXIT("GROUPALGO: startClusteringAlgorithm(manetNode *us, void *data)"); 
}

void groupIdDisplay(manetNode *us)
{
	LOGTRACE_ENTER("GROUPALGO: groupIdDisplay(...)"); 

	static sockaddr_in addr; 
	if(addr.sin_family == 0)
	{
#define GROUPDISPDATAPORT 3234
		if(0 > (us->groupCluster->groupIdFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)))
		{
			LOGWARN("Unable to open group display socket"); 
			LOGTRACE_EXIT("GROUPALGO: groupIdDisplay(...)"); 
			return; 
		}
		addr.sin_family = AF_INET;
		addr.sin_port = htons(GROUPDISPDATAPORT);
		addr.sin_addr.s_addr = htonl(((127<<24) | 1));      /* localhost  127.0.0.1 */
		int optval = 1;
		if(setsockopt(us->groupCluster->groupIdFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
		{
			LOGDEBUG("stateInit: Failed setsockopt(SO_REUSEADDR) for api socket. \"%s\"(%d)\n", strerror(errno), errno);
			close(us->groupCluster->groupIdFD);
			us->groupCluster->groupIdFD = -1;
			LOGTRACE_EXIT("GROUPALGO: groupIdDisplay(...)");
			return;
		}

		LOGDEBUG("GROUPALGO: Connected to (group display) UDP port %d with file descriptor %d.", GROUPDISPDATAPORT, us->groupCluster->groupIdFD);
	}
	
	if(us && us->groupCluster)
	{
		char buf[64]; 
		memset(&buf, 0, sizeof(buf)); 
		int len = snprintf(buf, sizeof(buf), "%d", us->groupCluster->curGid); 
		sendto(us->groupCluster->groupIdFD, &buf, len, 0, (struct sockaddr*)&addr, (socklen_t)sizeof(addr)); 
		LOGDEBUG("GROUPALGO: Node %d wrote %s to group display socket", (us->addr & 0xFF), buf); 
	}

	LOGTRACE_EXIT("GROUPALGO: groupIdDisplay(...)");
}

void logLocalGroup(manetNode *us, ostringstream &logStr)
{
	for(size_t i = 0; i < MAX_GM_NODES/8; i++)
	{
		for(size_t k = 0; k < 8; k++)
		{
			if(us->groupCluster->localGroup[i] & (HIGH_ORDER_BIT >> k))
			{
				logStr << ((i*8)+k) << " "; 
			}
		}
	}
}

int gmclusterInMyGroup(const manetNode *us, const neighbor *n)
{
	// use GPS data if we have it (and it's from a one-hop neighbor)
	// otehrwise trust the last HELLO message GID we got from that neighbor
	if(n->hopcount == 1)
	{
		return (us->groupCluster->localGroup[(n->addr & 0xFF)/8] & (HIGH_ORDER_BIT >> ((n->addr & 0xFF) % 8))); 
	}
	return n->groupCluster->curGid == us->groupCluster->curGid; 
}

void nodeGetHopAndNbrCount(manetNode *us, void *data)
{
	LOGTRACE_ENTER("nodeGetHopNumber(...)"); 

	timerSet(us, nodeGetHopAndNbrCount, us->groupCluster->routeHopCntComputeInterval*1000, NULL);	

	FILE *fil;
	char line[1024];

	fil=fopen("/proc/net/route","r");
	if(fil == NULL) 
	{
		LOGWARN("Unable to open /proc/net/route to get route table information"); 
		LOGTRACE_EXIT("nodeGetHopNumber(...) --> ERROR"); 
		return; 
	}

	us->groupCluster->curRouteHopCnt=0;
	us->groupCluster->curRouteNbrCnt=0;

	// Line example:
	// Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT
	// eth0    0001A8C0        00000000        0001    0       0       0       00FFFFFF        0       0       0                                                                   // eth0    0000FEA9        00000000        0001    0       0       0       0000FFFF        0       0       0
	while(fgets(line,sizeof(line)-1,fil))
	{
		char iface[16];
		int flags,refcnt,use,metric,mtu,window;
		unsigned int dst,nexthop,mask;
		int rc;

		rc=sscanf(line,"%s\t%x\t%x\t%d\t%d\t%d\t%d\t%x\t%o\t%d\n",iface,&dst,&nexthop,&flags,&refcnt,&use,&metric,&mask,&mtu,&window);

		if ((rc==10) && ((ntohl(dst) & us->netmask) == (us->addr & us->netmask)))
		{
			fprintf(stderr,"%d.%d.%d.%d (%d.%d.%d.%d) -> %d.%d.%d.%d\n",PRINTADDR(ntohl(dst)),PRINTADDR(mask),PRINTADDR(nexthop));
			us->groupCluster->curRouteHopCnt+=metric; 
			us->groupCluster->curRouteNbrCnt++; 
		}
	}

	fclose(fil);

	LOGDEBUG("HOPCNT: Computed hop cnt: %d, nbr cnt: %d", us->groupCluster->curRouteHopCnt, us->groupCluster->curRouteNbrCnt); 
	LOGTRACE_EXIT("nodeGetHopNumber(...) --> OK");
	return;
}
