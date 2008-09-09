/*  TELCORDIA TECHNOLOGIES PROPRIETARY - INTERNAL USE ONLY
 *  This file contains proprietary information that shall be distributed,
 *  routed or made available only within Telcordia, except with written
 *  permission of Telcordia.
 */


typedef struct {
    int n;
    long long int timeSinceOrig, timeSinceLastSent;
    long long int hopsSinceOrig, hopsSinceLastSent;
    } amroutePacketTypeStats;


typedef struct {
    char name[20];
    void (*nodeGotIt)(manetNode *, struct packet_amroute_s *);
    /* statistics */
    int originated, copiesSent, copiesForwarded;
    amroutePacketTypeStats received, ignored;
    } amroutePacketType;


typedef struct packet_amroute_s {
    ManetAddr origination;
    destime origTime;
    ManetAddr lastsender;
    destime lastSendTime;
    int hopsSinceOrig;
    int hopsSinceLastSent;
    ManetAddr finaldest;
    int sequenceNumber;
    amroutePacketType *type;
    packet *data;			/* data traffic payload */
    } packet_amroute;


#define	MESHNEIGHBOR	0
#define	TREENEIGHBOR	1
#define	NEIGHBORTYPES	2

typedef struct {
    int isNeighbor[NEIGHBORTYPES];
    destime neighborTime[NEIGHBORTYPES];
    int lastSeqNoJoinReq;		/* received */
    int lastSeqNoTreeCreate;		/* received */
    int nTreeCreateNak;			/* number received */
    } nodeInfo;
typedef struct clusteringState {
    int runningAMRoute;
    int initAMRoute;
    int nNeighbors[NEIGHBORTYPES];
    nodeInfo *nodes;
    ManetAddr logicalCore;
    int ttl;
    int joinAckReceived;
    destime lastTreeCreate;		/* sent or received */
    int treeMark;
    neighbor clusterhead;		/* used for graphics */
    } clusteringState;


/* utilities - amroutesend.c */
void sendAMRoutePacket(manetNode *, ManetAddr, int, amroutePacketType *, packet *);
void sendToNeighbors(int, manetNode *, amroutePacketType *, packet *, int, int);
void forwardToNeighbors(int, manetNode *, packet_amroute *, int, int);

/* utilities - neighborlist.c */
void addNeighbor(int, manetNode *, int);
void deleteNeighbor(int, manetNode *, int);
int includesNeighbor(int, manetNode *, int);
int addrToIndex(manet *, ManetAddr);


#define	INFINITE_TTL		255

#define	STATIC			/* placates C++ for now */
