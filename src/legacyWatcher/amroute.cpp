/*  TELCORDIA TECHNOLOGIES PROPRIETARY - INTERNAL USE ONLY
 *  This file contains proprietary information that shall be distributed,
 *  routed or made available only within Telcordia, except with written
 *  permission of Telcordia.
 */
/* DAG: mods to work with revised neighborlist.cpp 2005-09-27 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "des.h"
#include "hello.h"
#include "routing.h"
#include "amroute.h"

#include "node.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: amroute.cpp,v 1.22 2007/06/27 22:08:47 mheyman Exp $";

/* random integer in [0, x-1] */
#define	uniform(_x)	((int) ((_x) != 0 ? random() % (_x) : 0))

void nodeGotPacket(manetNode *us, packet *p);

/* knobs in milliticks (default values) */
static int firstBuild = 1000;
static int avgBuildDelay = 1000;
static int rebuildEverything = 12000;
static int joinReqInterval = 5000;
static int maxTTL = 5;
static int joinAckWait = 500;
static int treeCreateInterval = 6000;
static int treeCreateCopies = 0;
int treeExamineInterval = 1000;
static int treeExaminePeriodic = 1;
int treeExamineOnChange = 0;
static int participants = 0;

/* output knobs */
int notifypacket = 1;
static int notifynode = 1;	/* join/leave group, logical core */
int notifyneighbor = 1;
int notifytree = 1;


/* AMRoute packet types */

static void nodeGotJoinReq(manetNode *, packet_amroute *);
static void nodeGotJoinAck(manetNode *, packet_amroute *);
static void nodeGotJoinNak(manetNode *, packet_amroute *);
static void nodeGotTreeCreate(manetNode *, packet_amroute *);
static void nodeGotTreeCreateNak(manetNode *, packet_amroute *);
static void nodeGotDataTraffic(manetNode *, packet_amroute *);

static amroutePacketType joinReq = 
{ 
    "JOIN_REQ", nodeGotJoinReq, 
    0, 0, 0, 
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static amroutePacketType joinAck = 
{ 
    "JOIN_ACK", nodeGotJoinAck,
    0, 0, 0, 
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static amroutePacketType joinNak =
{ 
    "JOIN_NAK", nodeGotJoinNak,
    0, 0, 0, 
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static amroutePacketType treeCreate = 
{ 
    "TREE_CREATE", nodeGotTreeCreate,
    0, 0, 0, 
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static amroutePacketType treeCreateNak = 
{ 
    "TREE_CREATE_NAK", nodeGotTreeCreateNak,
    0, 0, 0, 
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static amroutePacketType dataTraffic = 
{ 
    "DATA_TRAFFIC", nodeGotDataTraffic,
    0, 0, 0, 
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 }
};
static amroutePacketType *amroutePacketTypes[] = {&joinReq, &joinAck, &joinNak,
        &treeCreate, &treeCreateNak, &dataTraffic, NULL};
void nodeInit(manetNode *us)
    {
    static int calledYet = 0;
    STATIC void amrouteInit(manet *);
    void periodicTreeExamine(manetNode *, void *);
    STATIC void AMRouteSetClusterHead(manetNode *, int);
    STATIC void nodeInit2(manetNode *, void *);

	manetPacketHandlerSet(us, PACKET_AMROUTE, nodeGotPacket);


    if (calledYet == 0)
        {
        amrouteInit(us->manet);
        if (treeExaminePeriodic != 0)
            periodicTreeExamine(us, NULL);
        calledYet = 1;
        }
    us->cluster = (clusteringState *) malloc(sizeof (clusteringState));
    (void) memset((void *) us->cluster, 0, sizeof (clusteringState));
    us->cluster->nodes = (nodeInfo *) malloc(us->manet->numnodes * sizeof (manetNode));
    (void) memset((void *) us->cluster->nodes, 0, us->manet->numnodes * sizeof (manetNode));
    if (participants == 0)
        helloInit(us, (helloNeighbor *) NULL,
                (helloCH *) AMRouteSetClusterHead, (helloHello *) NULL);
    else if (addrToIndex(us->manet, us->addr) % participants == 0)
        {
        us->cluster->runningAMRoute = 1;
        us->cluster->initAMRoute = 1;
        timerSet(us, nodeInit2,
                firstBuild + uniform(2 * avgBuildDelay),NULL);
        }

    }


STATIC void nodeInit2(manetNode *us, void *data)
    {
    STATIC void loseTreeConnectivity(manetNode *);
    STATIC void setLogicalCore(manetNode *, ManetAddr);
    STATIC void notifyCore(manetNode *);
    STATIC void sendJoinReq(manetNode *, void *);
    STATIC void sendTreeCreate(manetNode *, void *);

    if (us->cluster->runningAMRoute == 0)
        return;
    if (us->cluster->initAMRoute == 0 && us->manet->curtime
            - us->cluster->lastTreeCreate < rebuildEverything)
        return;
    us->cluster->initAMRoute = 0;
    /* ideally, cancel any pending sendJoinReq() and sendTreeCreate() */
    loseTreeConnectivity(us);
    us->cluster->logicalCore = us->addr;
    setLogicalCore(us, us->addr);
    notifyCore(us);
    sendJoinReq(us, NULL);
    sendTreeCreate(us, NULL);
    timerSet(us, nodeInit2, rebuildEverything, NULL);
    }
STATIC void amrouteInit(manet *m)
    {
    int x;
    char const *s;

    if ((x = configSearchInt(m->conf, "amroute_firstbuild")) > 0)
        firstBuild = x;
    if (configSearchStr(m->conf, "amroute_avgbuilddelay") != (char *) NULL &&
            (x = configSearchInt(m->conf, "amroute_avgbuilddelay")) >= 0)
        avgBuildDelay = x;
    if ((x = configSearchInt(m->conf, "amroute_rebuildeverything")) > 0)
        rebuildEverything = x;
    if ((x = configSearchInt(m->conf, "amroute_joinreqinterval")) > 0)
        joinReqInterval = x;
    if ((x = configSearchInt(m->conf, "amroute_maxttl")) > 0)
        maxTTL = x;
    if ((x = configSearchInt(m->conf, "amroute_joinackwait")) > 0)
        joinAckWait = x;
    if ((x = configSearchInt(m->conf, "amroute_treecreateinterval")) > 0)
        treeCreateInterval = x;
    if (configSearchStr(m->conf, "amroute_treecreatecopies") != (char *) NULL &&
            (x = configSearchInt(m->conf, "amroute_treecreatecopies")) >= 0)
        treeCreateCopies = x;
    if ((x = configSearchInt(m->conf, "amroute_treeexamineinterval")) > 0)
        treeExamineInterval = x;
    if ((s = configSearchStr(m->conf, "amroute_treeexamineperiodic")) != (char *) NULL &&
            (*s == 'F' || *s == 'f'))
        treeExaminePeriodic = 0;
    if ((s = configSearchStr(m->conf, "amroute_treeexamineonchange")) != (char *) NULL &&
            (*s == 'T' || *s == 't'))
        treeExamineOnChange = 1;
    if ((x = configSearchInt(m->conf, "amroute_participants")) >= 0)
        participants = x;
    if ((s = configSearchStr(m->conf, "amroute_notifypacket")) != (char *) NULL &&
            (*s == 'T' || *s == 't'))
        notifypacket = 1;
    if ((s = configSearchStr(m->conf, "amroute_notifynode")) != (char *) NULL &&
            (*s == 'T' || *s == 't'))
        notifynode = 1;
    if ((s = configSearchStr(m->conf, "amroute_notifyneighbor")) != (char *) NULL &&
            (*s == 'T' || *s == 't'))
        notifyneighbor = 1;
    if ((s = configSearchStr(m->conf, "amroute_notifytree")) != (char *) NULL &&
            (*s == 'T' || *s == 't'))
        notifytree = 1;

/* XXX Kludge to make amroute function in livenetwork environment  */
	if ((x=configSearchInt(m->conf,"numnodes")) >0 )
	m->numnodes=x;
    }
STATIC void AMRouteSetClusterHead(manetNode *us, int status)
    {
    int i;
    STATIC void notifyParticipant(manetNode *, const char *);
    STATIC void nodeInit2(manetNode *, void *data);

    if (status == 0 && us->cluster->runningAMRoute != 0)
        {
        us->cluster->runningAMRoute = 0;
        notifyParticipant(us, "leaving");
        sendToNeighbors(MESHNEIGHBOR, us, &joinNak, (packet *) NULL, 0, 0);
        for (i = 0; i < us->manet->numnodes; i++)
            {
            deleteNeighbor(TREENEIGHBOR, us, i);
            deleteNeighbor(MESHNEIGHBOR, us, i);	/* DAG: reordered */
            }
        }
    else if (status != 0 && us->cluster->runningAMRoute == 0)
        {
        us->cluster->runningAMRoute = 1;
        us->cluster->initAMRoute = 1;
        notifyParticipant(us, "joining");
        timerSet(us, nodeInit2, uniform(2 * avgBuildDelay), NULL);
        }
    else if (status == 0 && us->cluster->runningAMRoute == 0)
        notifyParticipant(us, "staying out of");
    else if (status != 0 && us->cluster->runningAMRoute != 0)
        notifyParticipant(us, "staying in");
    else
        notifyParticipant(us, "confused about");
    }


STATIC void notifyParticipant(manetNode *us, const char *action)
    {
    int i;

    if (notifynode == 0)
        return;
    (void) printf("time %lld: node %u is %s AMRoute group -> [",
            us->manet->curtime, us->addr, action);
    for (i = 0; i < us->manet->numnodes; i++)
        if (us->manet->nlist[i].cluster->runningAMRoute != 0)
            (void) printf(" %u", us->manet->nlist[i].addr);
    (void) printf(" ]\n");
    }
STATIC void sendJoinReq(manetNode *us, void *data)
    {
    STATIC void waitJoinAck(manetNode *, void *);

    if (us->cluster->logicalCore != us->addr || us->cluster->runningAMRoute == 0)
        return;
    us->cluster->joinAckReceived = 0;
    if (us->cluster->ttl < maxTTL)
        us->cluster->ttl++;
    sendAMRoutePacket(us, NODE_BROADCAST, us->cluster->ttl, &joinReq, (packet *) NULL);
    timerSet(us, waitJoinAck, joinAckWait, NULL);
    }


STATIC void waitJoinAck(manetNode *us, void *data)
    {
    STATIC void sendJoinReq(manetNode *, void *);

    if (us->cluster->joinAckReceived || us->cluster->ttl >= maxTTL)
        timerSet(us, sendJoinReq, joinReqInterval, NULL);
    else
        sendJoinReq(us, NULL);
    }


STATIC void sendTreeCreate(manetNode *us, void *data)
    {

    if (us->cluster->logicalCore != us->addr || us->cluster->runningAMRoute == 0)
        return;
    sendToNeighbors(MESHNEIGHBOR, us, &treeCreate, (packet *) NULL, treeCreateCopies, 1);
    us->cluster->lastTreeCreate = us->manet->curtime;
    timerSet(us, sendTreeCreate, treeCreateInterval, NULL);
    }


void amrouteSend(manetNode *us, packet *data)
    {
    if (data->dst != NODE_BROADCAST)
        sendAMRoutePacket(us, data->dst, INFINITE_TTL, &dataTraffic, data);
    else
        sendToNeighbors(TREENEIGHBOR, us, &dataTraffic, data, 0, 0);
    }


void nodeGotPacket(manetNode *us, packet *p)
    {
    packet_amroute *pamr;
    STATIC void incrementTypeStats(const packet_amroute *, destime,
            amroutePacketTypeStats *);
    void notifyPacket(manetNode *, packet_amroute *, const char *, int);

    if ((pamr = (packet_amroute *) p->data) == (packet_amroute *) NULL)
        return;
    pamr->hopsSinceOrig += p->hopcount;
    pamr->hopsSinceLastSent += p->hopcount;
    if (us->cluster->runningAMRoute != 0)
        {
        incrementTypeStats(pamr, us->manet->curtime, &pamr->type->received);
        if (notifypacket != 0)
            notifyPacket(us, pamr, "received", p->ttl);
        }
    else
        {
        incrementTypeStats(pamr, us->manet->curtime, &pamr->type->ignored);
        if (notifypacket != 0)
            notifyPacket(us, pamr, "ignoring", p->ttl);
        return;
        }
    (*pamr->type->nodeGotIt)(us, pamr);
    }


STATIC void incrementTypeStats(const packet_amroute *pamr, destime curtime,
        amroutePacketTypeStats *stats)
    {
    (stats->n)++;
    stats->timeSinceOrig += curtime - pamr->origTime;
    stats->timeSinceLastSent += curtime - pamr->lastSendTime;
    stats->hopsSinceOrig += pamr->hopsSinceOrig;
    stats->hopsSinceLastSent += pamr->hopsSinceLastSent;
    }


void notifyPacket(manetNode *us, packet_amroute *pamr, const char *action, int ttl)
    {
    (void) printf("time %lld: %u %s %s packet #%d; ", us->manet->curtime,
            us->addr, action, pamr->type->name, pamr->sequenceNumber);
    (void) printf("orig=%u sentby=%u ", pamr->origination, pamr->lastsender);
    if (pamr->finaldest != NODE_BROADCAST)
        (void) printf("dest=%u ", pamr->finaldest);
    else
        (void) printf("dest=* ");
    (void) printf("ttl=%d, hopcount=%d,%d\n", ttl, pamr->hopsSinceOrig,
            pamr->hopsSinceLastSent);
    }
STATIC void nodeGotJoinReq(manetNode *us, packet_amroute *pamr)
    {
    int origIndex;
    int newPacket;		/* new packet from this originator? */

    if ((origIndex = addrToIndex(us->manet, pamr->origination)) < 0)
        return;
    if (us->cluster->nodes[origIndex].lastSeqNoJoinReq < pamr->sequenceNumber)
        newPacket = 1;
    else
        newPacket = 0;
    if (newPacket != 0)		/* update last sequence number */
        us->cluster->nodes[origIndex].lastSeqNoJoinReq = pamr->sequenceNumber;
    /* ignore if logical core, not new packet, or already neighbor */
    if (pamr->origination != us->cluster->logicalCore && newPacket != 0 &&
            includesNeighbor(MESHNEIGHBOR, us, origIndex) == 0)
        {
        addNeighbor(MESHNEIGHBOR, us, origIndex);
        sendAMRoutePacket(us, pamr->origination, INFINITE_TTL, &joinAck, (packet *) NULL);
        }
    }


STATIC void nodeGotJoinAck(manetNode *us, packet_amroute *pamr)
    {
    int origIndex;

    if ((origIndex = addrToIndex(us->manet, pamr->origination)) < 0)
        return;
    if (includesNeighbor(MESHNEIGHBOR, us, origIndex))
        return;
    addNeighbor(MESHNEIGHBOR, us, origIndex);
    us->cluster->joinAckReceived = 1;
    }


STATIC void nodeGotJoinNak(manetNode *us, packet_amroute *pamr)
    {
    int origIndex;

    if ((origIndex = addrToIndex(us->manet, pamr->origination)) < 0)
        return;
    deleteNeighbor(TREENEIGHBOR, us, origIndex);
    deleteNeighbor(MESHNEIGHBOR, us, origIndex);	/* DAG: reordered */
    }
/* possible efficiency: nodes track TREE_CREATEs from losing logical
cores to construct tree faster if previous logical core goes away */

STATIC void nodeGotTreeCreate(manetNode *us, packet_amroute *pamr)
    {
    int origIndex, lastSenderIndex;
    int newPacket;		/* new packet from this originator? */
    STATIC ManetAddr resolveCore(ManetAddr, ManetAddr);
    STATIC void setLogicalCore(manetNode *, ManetAddr);
    STATIC void notifyCore(manetNode *);
    STATIC void acceptTreeUp(manetNode *, ManetAddr, int);
    STATIC void nodeInit2(manetNode *, void *data);

    if ((origIndex = addrToIndex(us->manet, pamr->origination)) < 0 ||
            (lastSenderIndex = addrToIndex(us->manet, pamr->lastsender)) < 0)
        return;
    if (us->cluster->nodes[origIndex].lastSeqNoTreeCreate < pamr->sequenceNumber)
        newPacket = 1;
    else
        newPacket = 0;
    if (newPacket != 0)		/* update last sequence number */
        us->cluster->nodes[origIndex].lastSeqNoTreeCreate = pamr->sequenceNumber;
    if (pamr->origination == us->cluster->logicalCore)
        /* packet is from logical core */ ;
    else if (resolveCore(pamr->origination, us->cluster->logicalCore) == 
            pamr->origination)
        {
        us->cluster->logicalCore = pamr->origination;
        setLogicalCore(us, pamr->origination);
        notifyCore(us);
        }
    else		/* prefer redundant link to inferior logical core */
        {		/* avoid risk of disconnected components */
        addNeighbor(TREENEIGHBOR, us, lastSenderIndex);
        return;
        }
    if (newPacket == 0)
        sendAMRoutePacket(us, pamr->lastsender, INFINITE_TTL, &treeCreateNak, (packet *) NULL);
    else
        {
        addNeighbor(TREENEIGHBOR, us, lastSenderIndex);
        acceptTreeUp(us, pamr->lastsender, us->manet->curtime - pamr->lastSendTime);
        forwardToNeighbors(MESHNEIGHBOR, us, pamr, treeCreateCopies, 1);
        us->cluster->lastTreeCreate = us->manet->curtime;
        timerSet(us, nodeInit2, rebuildEverything, NULL);
        }
    }


STATIC ManetAddr resolveCore(ManetAddr x, ManetAddr y)
    {
    return (x > y ? x : y);
    }
STATIC void notifyCore(manetNode *us)
    {
    if (notifynode != 0)
        (void) printf("time %lld: node %u's logical core is %u\n",
                us->manet->curtime, us->addr, us->cluster->logicalCore);
    }


STATIC void nodeGotTreeCreateNak(manetNode *us, packet_amroute *pamr)
    {
    int origIndex;

    if ((origIndex = addrToIndex(us->manet, pamr->origination)) < 0)
        return;
    deleteNeighbor(TREENEIGHBOR, us, origIndex);
    us->cluster->nodes[origIndex].nTreeCreateNak++;
    }


STATIC void nodeGotDataTraffic(manetNode *us, packet_amroute *pamr)
    {
    packetReReceive(us, pamr->data);
    if (pamr->finaldest != pamr->data->dst)
        forwardToNeighbors(TREENEIGHBOR, us, pamr, 0, 0);
    }


/* graphics */

STATIC void setLogicalCore(manetNode *us, ManetAddr addr)
    {
    if (us->addr == addr)
        us->rootflag = 1;
    else
        us->rootflag = 0;
    }


STATIC void acceptTreeUp(manetNode *us, ManetAddr addr, int hops)
    {
	fprintf(stderr,"node %d: add edge from %d to %d time= %lld\n",us->addr & 0xFF, us->addr & 0xFF, addr & 0xFF,us->manet->curtime);
    us->clusterhead = &us->cluster->clusterhead;
    us->clusterhead->addr = addr;
    us->clusterhead->hopcount = hops;
    us->clusterhead->level = -1;
    us->clusterhead->next = NULL;
    us->clusterhead->clusterhead = NODE_BROADCAST;
	/* we don't (and won't) know CH's CH.  however we're keeping the
	   CH in a neighbor struct, so the field is present */
    us->clusterhead->firstheard = 0;
    us->clusterhead->lastheard = us->manet->curtime;
    us->clusterhead->onehopdegree = 0;
    us->clusterhead->flags = 0;
    }


STATIC void loseTreeConnectivity(manetNode *us)
    {
	fprintf(stderr,"node %d: del edge from %d to %d   time= %lld\n",us->addr & 0xFF, us->addr & 0xFF, ((us->clusterhead)?us->clusterhead->addr:NODE_BROADCAST) & 0xFF,us->manet->curtime);
    us->clusterhead = NULL;
    }
void nodeFree(manetNode *us)
    {
    static int calledYet = 0;
    void treeTracker(manet *);
    void finalMessageStatistics(void);
    void finalTreeStatistics(manet *);

    if (calledYet == 0)
        {
        treeTracker(us->manet);
        finalMessageStatistics();
        finalTreeStatistics(us->manet);
        calledYet = 1;
        (void) fflush(stdout);
        }
#ifdef MODULE_ROUTING
    routeDumpBuffered(us, stderr);
#endif
    (void) free((void *) us->cluster->nodes);
    (void) free((void *) us->cluster);
    }


STATIC void finalMessageStatistics(void)
    {
    amroutePacketType **t, total;
    STATIC void statisticsAdd(const amroutePacketType *, amroutePacketType *);
    STATIC void statisticsPrint(const amroutePacketType *);

    (void) memset((void *) &total, 0, sizeof (amroutePacketType));
    (void) strcpy(total.name, "total");
    for (t = amroutePacketTypes; *t != NULL; t++)
        {
        statisticsAdd(*t, &total);
        statisticsPrint(*t);
        }
    statisticsPrint(&total);
    }


STATIC void statisticsAdd(const amroutePacketType *x, amroutePacketType *y)
    {
    STATIC void statisticsAdd2(const amroutePacketTypeStats *, amroutePacketTypeStats *);

    y->originated += x->originated;
    y->copiesSent += x->copiesSent;
    y->copiesForwarded += x->copiesForwarded;
    statisticsAdd2(&x->received, &y->received);
    statisticsAdd2(&x->ignored, &y->ignored);
    }


STATIC void statisticsAdd2(const amroutePacketTypeStats *x, amroutePacketTypeStats *y)
    {
    y->n += x->n;
    y->timeSinceOrig += x->timeSinceOrig;
    y->timeSinceLastSent += x->timeSinceLastSent;
    y->hopsSinceOrig += x->hopsSinceOrig;
    y->hopsSinceLastSent += x->hopsSinceLastSent;
    }
STATIC void statisticsPrint(const amroutePacketType *x)
    {
    STATIC void statisticsPrint2(const char *, const char *, const amroutePacketTypeStats *);

    (void) printf("\n");
    (void) printf("%s originated: %d\n", x->name, x->originated);
    (void) printf("%s copies sent: %d\n", x->name, x->copiesSent);
    (void) printf("%s copies forwarded: %d\n", x->name, x->copiesForwarded);
    statisticsPrint2(x->name, "received", &x->received);
    statisticsPrint2(x->name, "ignored", &x->ignored);
    }

#define	avg(_x, _y)	(((double) (_y)) != 0. ? (double) (_x) / (double) (_y) : 0.)

STATIC void statisticsPrint2(const char *name, const char *event,
        const amroutePacketTypeStats *stats)
    {
    (void) printf("%s packets %s: %d\n", name, event, stats->n);
    (void) printf("%s packets %s, total time since origination: %lld\n",
            name, event, stats->timeSinceOrig);
    (void) printf("%s packets %s, average time since origination: %f\n",
            name, event, avg(stats->timeSinceOrig, stats->n));
    (void) printf("%s packets %s, total time since last sent: %lld\n",
            name, event, stats->timeSinceLastSent);
    (void) printf("%s packets %s, average time since last sent: %f\n",
            name, event, avg(stats->timeSinceLastSent, stats->n));
    (void) printf("%s packets %s, total hops since origination: %lld\n",
            name, event, stats->hopsSinceOrig);
    (void) printf("%s packets %s, average hops since origination: %f\n",
            name, event, avg(stats->hopsSinceOrig, stats->n));
    (void) printf("%s packets %s, total hops since last sent: %lld\n",
            name, event, stats->hopsSinceLastSent);
    (void) printf("%s packets %s, average hops since last sent: %f\n",
            name, event, avg(stats->hopsSinceLastSent, stats->n));
    }
