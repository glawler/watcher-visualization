#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "graphics.h"

#include <set>
using namespace std;

#include "rng.h"
#include "graphcluster.h"
#include "node.h"
#include "routing.h"
#include "flood.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: graphcluster.cpp,v 1.33 2007/04/23 18:51:07 dkindred Exp $";

typedef struct
{
        int onehopcount;          /* number of neighbors */
        int symcount;             /* number of symmetric neighbors */
        int sequencenum;
        ManetAddr clusterhead;
        ManetAddr addresslist[40];
} packetHello;

/* Tree building packet
*/
typedef struct
{
	ManetAddr rootid;
	int root_seqnum;

	ManetAddr srcid;
	ManetAddr parentid;
	int root_distance;
	int treeseq;
} packetBannerjeeTree;

/* This is the max size of a sub-tree and neighbor list.  It should never
** actually get larger than 2K...  (where K is the min cluster size, for graphcluster)
*/
#define MAXKIDS 200

typedef struct
{
        ManetAddr rootid;
        int root_seqnum;
	ManetAddr clusterheadroute[MAXKIDS];
	int clusterheadroutelen;

        int level;
	int seq;
} packetDaddy;

typedef struct
{
	ManetAddr rootid;
	int root_seqnum;
	int root_distance;

	int subtreesize;
	int numneighbors;            /* set of neighbor nodes which the subtree has */
	ManetAddr list[MAXKIDS];     /* first subtreesize entrys are the kids.  rest are the neighbors  */
	int routelen;
} packetGraphclusterTerm;


typedef struct
{
	ManetAddr rootid;
	int root_seqnum;

	ManetAddr clusterhead;
	int chflag;
	ManetAddr clusterheadroute[MAXKIDS];
	int clusterheadroutelen;
} packetGraphclusterCluster;

static RNG *rnd=NULL;

typedef struct clusteringNeighbor
{
	ManetAddr rootid;
	int root_seqnum;
	int treeseq;

	int root_distance;
        ManetAddr parent;
	int level;

	int clusterheadflag;
	ManetAddr clusterhead;
	int termvalid;
	packetGraphclusterTerm term;
} clusteringNeighbor;

typedef enum {NOTREEYET,FLOATINGROOT,TREEINIT,WAITCLUSTER,WAITTERM,HIERARCHYINIT,HIERARCHYREADY} clusterState;

/* Additional state information on the node which banerjee needs
*/
typedef struct clusteringState
{
	ManetAddr rootid;
	int root_seqnum;
	int privateroot_seqnum;

	neighbor *parent;
	neighbor clusterhead;    /* note this is not a pointer */
	neighbor *tmpclusterhead;

	ManetAddr clusterheadroute[MAXKIDS];
	int clusterheadroutelen;

	clusterState state;
	int chflag;     /* if set, means we're a CH  */

	destime lasttreerec,lastinittreesent,firsttermrec;
	destime lastterm;
	destime lastcluster;     /* Time last cluster packet was sent.   */
	int treeseq;
	int hellosequencenum;
} clusteringState;


static void sendTree(manetNode *us);
static void sendTreeTimer(manetNode *us, void *data);
static int findParent(manetNode *us);
static void sendTermTimer(manetNode *us, void *data);
static void clusterKids(manetNode *us);

static void helloPacket(manetNode *us, packet *p);
static void gotTree(manetNode *us, packet *p);
static void gotTerm(manetNode *us, packet *p);
static void gotCluster(manetNode *us, packet *p);
static void gotDaddy(manetNode *us, packet *p);

/* Send a really boring hello packet   (This needs to be in a separate module, but must be individually enabled by different hierarchy algorithms)
*/

static void helloSend(manetNode *us,void *data)
{
	packet *p;
        packetHello *hp;

	neighbor *n;
        unsigned int ncount,nsymneighbors;
	int noise;

	        /* first, reschedule...  */
        noise=(int)(rnd->rand_u01()*100.0)-50;
        noise=0;
        timerSet(us,helloSend,TIME_HELLO+noise,NULL);

	p=packetMalloc(us,sizeof(packetHello));
        p->type=PACKET_GRAPHCLUSTER_HELLO;
        p->dst=NODE_BROADCAST;
        p->ttl=0;
        hp=(packetHello*)p->data;
        hp->sequencenum=us->cluster->hellosequencenum++;
        ncount=0;
        nsymneighbors=0;

	if (us->clusterhead)
		hp->clusterhead=us->clusterhead->addr;
	else	
		hp->clusterhead=NODE_BROADCAST;

	n=us->neighborlist;
#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d:  Send Hello  ",us->addr & 0xFF);
#endif
	while(n)
	{
		if ((n->level==0) && (n->flags & NEIGHBOR_HEARD) && (ncount<(sizeof(hp->addresslist)/sizeof(hp->addresslist[0]))))
		{
			hp->addresslist[ncount]=n->addr;
			ncount++;
			if (n->flags & NEIGHBOR_HEARS)
				nsymneighbors++;
#ifdef DEBUG_HELLO
			fprintf(stderr,"%d  ",n->addr & 0xFF);
#endif
		}
		n=n->next;
	}
#ifdef DEBUG_HELLO
	fprintf(stderr,"\n");
#endif

	hp->onehopcount=ncount;
	hp->symcount=nsymneighbors;

	packetSend(us,p, PACKET_ORIGIN);
	packetFree(p);
}

/* Called when a really boring hello packet arrives
*/
static void helloPacket(manetNode *us, packet *p)
{
	neighbor *n;
	packetHello *hp=(packetHello*)p->data;
	int i;

        if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
        {
                return;
        }

	if (nodePacketSearch(us,p)!=NULL)    /* if we've already seen this packet, ignore it */
	{
		return;
	}
	nodePacketInsert(us,p);

	n=neighborSearch(us,p->src,0);
	if (n==NULL)
	{
		n=neighborInsert(us,p->src,0);
		n->clusterhead=NODE_BROADCAST;
		n->clusterheadflag=0;
		n->cluster=NULL;
		n->hopcount=0;
	}
	n->lastheard=us->manet->curtime;

	n->flags|=NEIGHBOR_HEARD;    /* we heard them...  */

//	n->clusterhead=hp->clusterhead;

#ifdef DEBUG_HELLO
	fprintf(stderr,"node %d: Rec Hello from %d  hears ",us->addr & 0xFF,p->src & 0xFF);
#endif
	for(i=0;i<hp->onehopcount;i++)
	{
#ifdef DEBUG_HELLO
		fprintf(stderr,"%d ",hp->addresslist[i] & 0xFF);
#endif
		if (hp->addresslist[i]==us->addr)
		{
#ifdef DEBUG_HELLO
			fprintf(stderr,"(us) ");
#endif
			if (!(n->flags & NEIGHBOR_HEARS))
			{
#ifdef DEBUG_HELLO
//				fprintf(stderr,"node %d: just got a new symmetric link.\n",us->addr & 0xFF);    /* got a new symmetric link... call the OLSR stuff */
#endif
				n->flags|=NEIGHBOR_HEARS;   /* they listed us, so they hear us  */
				n->firstheard=us->manet->curtime;
			}
		}
	}
#ifdef DEBUG_HELLO
		fprintf(stderr,"\n");
#endif

	/* we have a neighbor now...  leave FLOATINGROOT */
	if ((n->flags & NEIGHBOR_HEARS) && (us->cluster->state==FLOATINGROOT))
	{
		fprintf(stderr,"node %d: not floating\n",us->addr);

		us->cluster->state=NOTREEYET;
		us->cluster->lasttreerec=us->manet->curtime;
		us->cluster->lastinittreesent=us->manet->curtime;
		us->cluster->rootid=NODE_BROADCAST;
		us->rootflag=0;
	}	
}

/* Hello timeout timer
** called every HELLO interval, to remove HEARD and HEARS flags from entries from the neighbor list
** which we havn't heard a HELLO from recently enough
*/
void helloTimeout(manetNode *us,void *data)
{
        neighbor *n;
	neighbor *d;

        timerSet(us,helloTimeout,TIME_HELLO_TIMEOUT,NULL);

        n=us->neighborlist;
        while(n)
        {
                if ((n->level>=0) && (us->manet->curtime-n->lastheard) > TIME_HELLO_TIMEOUT)
                {
#ifdef DEBUG_HELLO
                        if (n->flags & NEIGHBOR_HEARS)
                                fprintf(stderr,"node %d: node %d timed out\n",us->addr & 0xFF,n->addr & 0xFF);   /* lost an edge, call the OLSR stuff */
#endif
			d=n;
			n=n->next;
			if (us->cluster->parent==d)
				us->cluster->parent=NULL;
			neighborDelete(us,d);
                }
		else
                	n=n->next;
        }

		/* if we have no neighbors, transition into FLOATINGROOT, set root flag  */
	if (us->neighborlist==NULL)
	{
#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: we're floating root\n",us->addr & 0xFF);      /* "I'm floating!!!" - Dr. Membrane   */
#endif
		us->cluster->state=FLOATINGROOT;
		us->cluster->rootid=NODE_BROADCAST;
		us->rootflag=1;
		us->cluster->clusterheadroutelen=0;
		us->cluster->parent=NULL;
		us->level=0;
		us->clusterhead=NULL;
		return;
	}

	if ((us->cluster->parent==NULL) && (us->cluster->rootid!=us->addr))    /* mobility removed our parent (and we're not root)...  find a new one from neighbors on current rootnode and rootid  if there isn't one, go to NOTREEYET  */
	{
		findParent(us);
		if (us->cluster->parent==NULL)
		{
			us->cluster->rootid=NODE_BROADCAST;     /* lost our parent...  go back into treeinit, to find a new one */
			us->cluster->state=NOTREEYET;      /* XXX or should this be treeinit?   */
		}
		else
		{
			us->cluster->state=WAITCLUSTER;
			sendTree(us);
		}
	}
}

static void clusterReset(manetNode *us,ManetAddr rootid, int root_seqnum)
{
	neighbor *n;

	us->cluster->state=TREEINIT;
	us->cluster->rootid=rootid;                               /* init from start...  */
	us->cluster->root_seqnum=root_seqnum;
	us->cluster->chflag=0;

	us->cluster->parent=NULL;
	us->cluster->firsttermrec=0;
	us->cluster->lastcluster=0;
	us->cluster->lastterm=0;

	/* nuke neighbor list */
	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if (n->cluster)
		{
			free(n->cluster);
			n->cluster=NULL;
		}
		n->hopcount=0;
	}
	if (us->cluster->rootid!=us->addr)
		us->rootflag=0;
}

void nodeInit(manetNode *us)
{
	int noise;

	us->clusterhead=NULL;
        us->neighborlist=NULL;
        us->level=0;
        us->rootflag=0;

	us->cluster=(clusteringState*)malloc(sizeof(*us->cluster));

	if (rnd==NULL)
		rnd= new RNG(42);

	clusterReset(us,NODE_BROADCAST,0);
        us->cluster->hellosequencenum=0;
	us->cluster->state=NOTREEYET;
	us->cluster->lasttreerec=0;
	us->cluster->lastinittreesent=us->manet->curtime;
	us->cluster->privateroot_seqnum=0;
	us->rootflag=0;

	noise=(int)(rnd->rand_u01()*4000.0)+500;
	sendTreeTimer(us,NULL);
	sendTermTimer(us,NULL);

	manetPacketHandlerSet(us, PACKET_GRAPHCLUSTER_HELLO, helloPacket);
	manetPacketHandlerSet(us, PACKET_GRAPHCLUSTER_TREE, gotTree);
	manetPacketHandlerSet(us, PACKET_GRAPHCLUSTER_TERM, gotTerm);
	manetPacketHandlerSet(us, PACKET_GRAPHCLUSTER_CLUSTER, gotCluster);
	manetPacketHandlerSet(us, PACKET_GRAPHCLUSTER_DADDY, gotDaddy);

	helloSend(us,NULL);
	helloTimeout(us,NULL);
}

void nodeFree(manetNode *n)
{
}


/* This callback is for transmitting the buildtree packet regularly.
** A buildtree packet is only sent by the root.
** Child nodes only send buildtree packets in reply to hearing one.  Thus, the buildtree packet indicates connectivity to the hierarchy
**
** The tree building packet is sent as a keepalive.
*/
static void sendTreeTimer(manetNode *us, void *data)
{
	int noise;

	noise=(int)(rnd->rand_u01()*TIME_TREEREXMIT/2)-(TIME_TREEREXMIT/4);
	timerSet(us,sendTreeTimer,TIME_TREEREXMIT+noise,NULL);

#ifdef DEBUG_GRAPHCLUSTER
//	fprintf(stderr,"node %d: time= %lld next init tree at %lld or by root at %lld   root= %d\n",us->addr & 0xFF,us->manet->curtime,us->cluster->lasttreerec+(TIME_INITTREE*2), us->cluster->lastinittreesent+TIME_INITTREE,us->cluster->rootid);
#endif

	if (((us->manet->curtime - us->cluster->lasttreerec ) > (TIME_INITTREE*2)) ||     /* too long ago...  we're new root!  */
	     ((us->cluster->rootid==us->addr) && (us->manet->curtime - us->cluster->lastinittreesent ) > (TIME_INITTREE)))     /* too long ago...  and we're root already!  */
	{
		us->cluster->lasttreerec=us->manet->curtime;
		us->cluster->lastinittreesent=us->manet->curtime;
		us->cluster->state=TREEINIT;
		clusterReset(us,us->addr,us->cluster->privateroot_seqnum++);

		us->cluster->clusterheadroutelen=0;
#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: time= %lld root sending init tree next tree at %lld\n",us->addr & 0xFF,us->manet->curtime, us->cluster->lastinittreesent+TIME_INITTREE);
#endif
		sendTree(us);
		return;
	}

	if ((us->cluster->state!=TREEINIT))         /* are we in a legal state to rexmit?  */
		return;

	if ((us->manet->curtime - us->cluster->lasttreerec ) < TIME_TREEREXMIT)           /* too soon to rexmit?   */
	{
		return;
	}

	sendTree(us);          /* rexmit!  */
}

static void sendTermTimer(manetNode *us,void *data)
{
	timerSet(us,sendTermTimer,TIME_TERM/4, NULL);

	if ((us->cluster->state==TREEINIT) || (us->cluster->state==NOTREEYET) || (us->cluster->state==FLOATINGROOT) || (us->cluster->state==HIERARCHYINIT))
		return;

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: sendTermTimer: state= %d!\n",us->addr,us->cluster->state);
#endif

	if ((us->cluster->state==WAITTERM)  && ((us->cluster->firsttermrec>0)?((us->manet->curtime-us->cluster->firsttermrec)>TIME_TERM*3):0))
	{
		neighbor *n;

		fprintf(stderr,"node %d: sendTermTimer: aborting the missing kids!\n",us->addr);

		for(n=us->neighborlist;n!=NULL;n=n->next)
		{
			if ((n->level==0) && (n->cluster) && (n->cluster->parent==us->addr)
				&& (!n->cluster->termvalid))
			{
				n->cluster->parent=NODE_BROADCAST;
			}
		}
		/*
		walk list of neighbors, and mark the children which we have not gotten a TERM from given up on.
			(perhaps by erasing their neighbor->cluster->parent field, so they are no longer our children)
		Then, clusterkids will do the transition from WAITTERM to WAITCLUSTER, and call the clusterkids algorithm
		*/
	}

	clusterKids(us);    /* This will send a term packet, if appropriate...  */
}

/* union neigh into the list pointed to by list, which has count entries
** returns new count
*/
static int unionNeighbor(ManetAddr *list, ManetAddr neigh,int count)
{
	int i;

	for(i=0;i<count;i++)
		if (list[i]==neigh)
			return count;

	list[count++]=neigh;
	return count;
}

/* Compute our level, from the levels our children advertised in their DADDY packets
*/
static void updateLevel(manetNode *us)
{
	neighbor *n;
	int level=0;
	int havekids=0;
		                                          /* compute our level, from the level of our kids.  */
	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if ((n->clusterhead==us->addr) && (n->level>=0))
		{
#ifdef DEBUG_GRAPHCLUSTER
			fprintf(stderr,"node %d: updateLevel neighbor %d level %d parent %d\n",us->addr,n->addr & 0xFF, n->cluster?(n->cluster->level):-1,n->cluster?(n->cluster->parent & 0xFF):NODE_BROADCAST);
#endif
			if ((n->cluster) && (n->cluster->level>level))            /* find max level of all the DADDYs we've received */
				level=n->cluster->level;

			havekids=1;
		}
	}
	if (((us->cluster->chflag) || (us->cluster->rootid==us->addr)) && (havekids))    /* if we're a CH, then our level is one above the max level of our kids */
		level++;

	us->level=level;
}

static void sendTerm(manetNode *us)
{
	packet *p;
	packetGraphclusterTerm *pt;
	neighbor *n;
	ManetAddr neighbors[MAXKIDS];
	int numneighbors=0;
	int i;

	if (us->cluster->state!=WAITCLUSTER)
		return;

	if (((us->manet->curtime - us->cluster->lastterm) < TIME_TERM))
		return;
	us->cluster->lastterm=us->manet->curtime;

	if (us->cluster->parent==NULL)     /* our parent fell out of the neighbor list.  we're dead till next algorithm execution  */
		return;

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: in sendTerm...  state= %d parent= %d\n",us->addr & 0xFF,us->cluster->state,us->cluster->parent?us->cluster->parent->addr:NODE_BROADCAST);
#endif

	/* If we're sending a TERM, then we're definately not a root */
	us->rootflag=0;

	p=packetMalloc(us,sizeof(*pt));
	pt=(packetGraphclusterTerm*)p->data;

	p->type=PACKET_GRAPHCLUSTER_TERM;
	p->dst=us->cluster->parent->addr;
	p->ttl=0;

	pt->subtreesize=0;

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if ((n->level!=0) || (n->cluster==NULL))
			continue;

		numneighbors=unionNeighbor(neighbors,n->addr,numneighbors);

		if ((n->cluster->parent==us->addr) && (n->level==0) && (n->cluster->rootid==us->cluster->rootid) && (n->cluster->root_seqnum==us->cluster->root_seqnum) && (n->cluster->termvalid))
		{
#ifdef DEBUG_GRAPHCLUSTER
			fprintf(stderr,"node %d: child neighbor %d level %d\n",us->addr & 0xFF,n->addr & 0xFF,n->level);
#endif
			if (n->cluster)     /* if we got a TERM from it...  */
			{
				if ((n->cluster->clusterheadflag==0) && (n->cluster->clusterhead==NODE_BROADCAST))
				{
					for(i=0;i<n->cluster->term.subtreesize;i++)            /* add neighbor's child list to child list */
					{
						pt->list[pt->subtreesize++]=n->cluster->term.list[i];
						if (pt->subtreesize >= (signed int)(sizeof(pt->list)/sizeof(pt->list[0])))
							return;
					}

					for(i=0;i<n->cluster->term.numneighbors;i++)          /* union neighbor's neighborlist to neighbor list */
						numneighbors=unionNeighbor(neighbors,n->cluster->term.list[n->cluster->term.subtreesize+i],numneighbors);
				}
				if (n->cluster->clusterhead==NODE_BROADCAST)
				{
					pt->list[pt->subtreesize++]=n->addr;             /* add neighbor to child list  */
				}
			}
		}
	}

	pt->numneighbors=numneighbors;
	for(i=0;i<numneighbors;i++)
	{
		pt->list[pt->subtreesize+i]=neighbors[i];
		if (pt->subtreesize >= (signed int)(sizeof(pt->list)/sizeof(pt->list[0])))
			return;
	}

	pt->rootid=us->cluster->rootid;
	pt->root_seqnum=us->cluster->root_seqnum;
	pt->root_distance=us->cluster->parent->cluster->root_distance+1;

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: sending term to %d subtreesize= %d numneighbors= %d root_distance= %d\n",us->addr & 0xFF,p->dst & 0xFF, pt->subtreesize,pt->numneighbors,pt->root_distance);
#endif

	packetSend(us,p, PACKET_ORIGIN);
	packetFree(p);
}

/* we have figured out, or been told, our CH.  lets tell the kids!
**
**  we actually only want to tell kids which are not ALLOCATED!
*/
static void sendCluster(manetNode *us)
{
	packet *p;
	packetGraphclusterCluster *pt;
	neighbor *n;

	if ((us->cluster->state!=HIERARCHYINIT)) //  && (us->cluster->state!=WAITCLUSTER))
		return;

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if ((n->level==0) && (n->cluster) && (n->cluster->parent==us->addr) && (n->cluster->clusterhead!=NODE_BROADCAST))             /* if this is our kid, and we know its clusterhead...  */
		{
			p=packetMalloc(us,sizeof(*pt));
			pt=(packetGraphclusterCluster*)p->data;

			pt->chflag=n->cluster->clusterheadflag;            /* chflag is clusterhead flag  */
			pt->clusterhead=n->cluster->clusterhead;

			if (us->cluster->chflag)         /* if we're a clusterhead, then the clusterhead route is to us  */
				pt->clusterheadroutelen=0;
			else
			{
				memcpy(pt->clusterheadroute,us->cluster->clusterheadroute,sizeof(us->cluster->clusterheadroute));      /* otherwise, its to our CH */
				pt->clusterheadroutelen=us->cluster->clusterheadroutelen;
			}

			p->type=PACKET_GRAPHCLUSTER_CLUSTER;
			p->dst=n->addr;
			p->ttl=0;
			pt->rootid=us->cluster->rootid;
			pt->root_seqnum=us->cluster->root_seqnum;

#ifdef DEBUG_GRAPHCLUSTER
			fprintf(stderr,"node %d: sending a cluster to %d  CH= %d CHflag= %d  clusterheadroutelen= %d\n",us->addr & 0xFF,p->dst & 0xFF,pt->clusterhead & 0xFF,pt->chflag,pt->clusterheadroutelen);
#endif

			packetSend(us,p, PACKET_ORIGIN);
			packetFree(p);
		}
	}
}

/* The proper way to think about the name of this function is to imagine a 5 year old
** suddenly realizing their Daddy around, running accross the room screaming "DADDY!!" and 
** glomping onto said Father's legs.
**
** Thus parent nodes are informed of the presence and level of their child nodes. (Needed
** for multi-hop children.)
*/
static void sendDaddy(manetNode *us)
{
	packet *p;
	packetDaddy *pd;

	if (us->cluster->rootid==us->addr)
		return;
	if (us->clusterhead==NULL)
		return;
	if (us->cluster->parent==NULL)
		return;

	if ((us->cluster->state!=HIERARCHYINIT)  && (us->cluster->state!=HIERARCHYREADY))
		return;

	updateLevel(us);

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: sending DADDY to %d  level= %d\n",us->addr & 0xFF,us->clusterhead->addr & 0xFF,us->level);
#endif

	p=packetMalloc(us,sizeof(*pd));
	p->type=PACKET_GRAPHCLUSTER_DADDY;

	p->dst=us->clusterhead->addr;

	p->ttl=0;
	pd=(packetDaddy*)p->data;
	pd->level=us->level;
	pd->rootid=us->cluster->rootid;
	pd->root_seqnum=us->cluster->root_seqnum;
	pd->seq=us->cluster->treeseq++;

	memcpy(pd->clusterheadroute,us->cluster->clusterheadroute,sizeof(us->cluster->clusterheadroute));
	pd->clusterheadroutelen=us->cluster->clusterheadroutelen;

	packetSend(us,p, PACKET_ORIGIN);

	packetFree(p);
}

/* Our parent has sent us a cluster.  Lets tell our kids too!
*/
static void gotCluster(manetNode *us, packet *p)
{
	packetGraphclusterCluster *pt;
	neighbor *n;

        if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
        {
                return;
        }

	if (us->cluster->state!=WAITCLUSTER)
		return;

	if (us->cluster->parent==NULL)
		return;

	pt=(packetGraphclusterCluster*)p->data;

	if ((pt->rootid!=us->cluster->rootid) || (pt->root_seqnum!=us->cluster->root_seqnum))    /* make sure its the right tree...  */
		return;

	if (p->src!=us->cluster->parent->addr)     /* we care only about cluster packets from our parent  */
		return;

	if (p->dst!=us->addr)    /* make sure its for us.  */
		return;

	us->cluster->state=HIERARCHYINIT;

	if (pt->chflag)        /* parent made us a clusterhead (whee!) */
		us->cluster->chflag=1;

	memcpy(us->cluster->clusterheadroute,pt->clusterheadroute,sizeof(pt->clusterheadroute));
	us->cluster->clusterheadroutelen=pt->clusterheadroutelen;
	us->cluster->clusterheadroute[us->cluster->clusterheadroutelen++]=us->addr;

	for(n=us->neighborlist;n!=NULL;n=n->next)                                       /* for propigating CLUSTER packets down  */
		if ((n->level==0) && (n->cluster) && (n->cluster->parent==us->addr) && (n->cluster->clusterhead==NODE_BROADCAST))
			n->cluster->clusterhead=pt->clusterhead;
	
	if (us->cluster->rootid==us->addr)
		us->clusterhead=NULL;
	else
	{		
		us->clusterhead=&us->cluster->clusterhead;

		us->clusterhead->addr=pt->clusterhead;
		us->clusterhead->level=-1;
		us->clusterhead->next=NULL;
		us->clusterhead->clusterhead=NODE_BROADCAST;    /* we don't (and won't) know CH's CH.  however we're keeping the CH in a neighbor struct, so the field is present */
		us->clusterhead->firstheard=0;
		us->clusterhead->lastheard=us->manet->curtime;
		us->clusterhead->onehopdegree=0;
		us->clusterhead->flags=0;
		us->clusterhead->hopcount=p->hopcount;
	}

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: got a cluster from %d.  CH= %d routelen= %d ",us->addr & 0xFF,p->src & 0xFF,us->clusterhead?us->clusterhead->addr & 0xFF:us->addr & 0xFF,us->cluster->clusterheadroutelen);
	for(int i=0;i<us->cluster->clusterheadroutelen;i++)
		fprintf(stderr," %d",us->cluster->clusterheadroute[i]);
	fprintf(stderr,"\n");
#endif

	/* we then want to send our kids a clusterhead packet.  */
	if ((us->manet->curtime-us->cluster->lastcluster) > TIME_TREEREXMIT)
	{
		us->cluster->lastcluster=us->manet->curtime;
		sendCluster(us);
		sendDaddy(us);
	}
}

/* Got a Daddy packet from one of our children
*/
static void gotDaddy(manetNode *us, packet *p)
{
	packetDaddy *pd;
	neighbor *n;
	int oldlevel;
	packet *cpy;

        if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
        {
                return;
        }

	pd=(packetDaddy*)p->data;

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: got a DADDY for %d src= %d hopcount %d level %d routelen= %d state= %d\n",us->addr & 0xFF,p->dst & 0xFF, p->src & 0xFF,p->hopcount,pd->level,pd->clusterheadroutelen,us->cluster->state);
#endif

	if ((pd->rootid!=us->cluster->rootid) || (pd->root_seqnum!=us->cluster->root_seqnum))
		return;

	if ((us->cluster->state==NOTREEYET) || (us->cluster->state==TREEINIT) || (us->cluster->state==WAITTERM) || (us->cluster->state==WAITCLUSTER))
		return;

	if (p->dst!=us->addr)
	{
		/* forward DADDY packet (DADDY packets are source routed by the route in the CLUSTER packet we got) */
		if (pd->clusterheadroute[pd->clusterheadroutelen - p->hopcount-1 ] == us->addr)   /* if we're this hop  */
		{
			cpy=packetCopy(p,0);                   /* send to next hop  */
			cpy->hopcount++;
			cpy->ttl--;

#ifdef DEBUG_GRAPHCLUSTER
			fprintf(stderr,"node %d: forwarding DADDY to %d\n",us->addr & 0xFF,cpy->dst);
#endif
			
			packetSend(us,cpy, PACKET_REPEAT);
			packetFree(cpy);
		}
		return;
	}

	assert(pd->level<=MAXLEVEL-1);
	assert(p->hopcount>0);

	n=neighborSearch(us,p->src,p->hopcount-1);
	if (n==NULL)
	{
		n=neighborInsert(us,p->src,p->hopcount-1);
		n->clusterheadflag=0;
		n->cluster=NULL;
	}
	if (n->cluster!=NULL)
		n->cluster->level=pd->level;
	else
	{
#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: why don't we have a cluster pointer for %d\n?",us->addr & 0xFF, p->src & 0xFF);
#endif
	}

		/* Why does it think its our kid if cluster pointer is invalid?   It needs to have gotten a cluster before its our kid */

	n->hopcount=p->hopcount;

#if 0
	if ((n->cluster)?(us->cluster->treeseq <pd->seq))
		return;
	n->cluster->treeseq=pd->treeseq;
#endif

	n->lastheard=us->manet->curtime;
	n->clusterhead=us->addr;

	if (us->cluster->rootid==us->addr)  
	{
		us->rootflag=1;
		us->clusterhead=NULL;
	}

	us->cluster->state=HIERARCHYREADY;

	oldlevel=us->level;
	updateLevel(us);

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: oldlevel= %d level= %d\n",us->addr & 0xFF, oldlevel,us->level);
#endif

	if (oldlevel!=us->level)
		sendDaddy(us);
}

/* K is the minimum cluster size.  2K is max cluster size
**
**  This will set our level.  Which is 0 if we are a leaf node, or max level of our kids + 1 
**  If we are a leaf node, we send our list of nodes and neighbors up
**  If we are not a leaf node, we need the term packet from every child to arrive
**   on the last term packet, we send our list of nodes and neighbors up
**
**  Eventually a parent will declare some clusters.  Sub-trees are always atomically allocated.  
**   So, a parent says to each of its kids "your CH is ___" the CH is either the parent, some grand parent, or the kid.
**   Those parent packets then get repeated down the tree.
*/
#define K 3

static void clusterKids(manetNode *us)
{
	neighbor *n;
	int childcount=0,validchildcount=0,neighborcount=0,validneighborcount=0;
	int subtreesize=0;    /* 0 because we don't count ourselves.  */
	int clusterflag=0;

	switch(us->cluster->state)
	{
		case NOTREEYET:
			return;
		break;
		case HIERARCHYINIT:
		case HIERARCHYREADY:
		default:
		break;
	}

	/* Check all our child nodes.  (These are breadth first tree children, not hierarchy children) */

	/* We will be here if we got a TREE.  if we have a TREE from each neighbor, and no children, we're done  (clusterkids, and go to WAITCLUSTER) */

	/* We will be here is we got a TERM.  if we have a TERM from every child, we're done (if we got here with a timeout, then the children we've given up on are already marked not-ours)   (clusterkids, and goto WAITCLUSTER) */

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: first term was %lld seconds ago.  firsttermrec== %lld\n",us->addr & 0xFF, us->manet->curtime - us->cluster->firsttermrec,us->cluster->firsttermrec);
#endif

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if (n->level==0)
		{
#ifdef DEBUG_GRAPHCLUSTER
			fprintf(stderr,"node %d: neighbor %d level= %d ",us->addr & 0xFF,n->addr & 0xFF,n->level);
#endif
			neighborcount++;
			if ((n->cluster) && (n->cluster->rootid==us->cluster->rootid) && (n->cluster->root_seqnum==us->cluster->root_seqnum))
			{
#ifdef DEBUG_GRAPHCLUSTER
				fprintf(stderr,"valid  ");
#endif
				validneighborcount++;
				if (n->cluster->parent==us->addr)
				{
#ifdef DEBUG_GRAPHCLUSTER
					fprintf(stderr,"child  ");
#endif
					childcount++;
					subtreesize++;
					if  ((n->cluster==NULL) || (n->cluster->termvalid==0))
					{
#ifdef DEBUG_GRAPHCLUSTER
						fprintf(stderr,"missing");
#endif
					}
					else
					{
						validchildcount++;
						subtreesize+=n->cluster->term.subtreesize; 
#ifdef DEBUG_GRAPHCLUSTER
						fprintf(stderr,"here");
#endif
					}
				}
#ifdef DEBUG_GRAPHCLUSTER
				if ((us->cluster->parent) && (us->cluster->parent->addr==n->addr))
					fprintf(stderr,"parent...  ");
#endif
			}
#ifdef DEBUG_GRAPHCLUSTER
			fprintf(stderr,"\n");
#endif
		}
	}
#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: neighborcount= %d validneighborcount= %d childcount= %d validchildcount= %d\n",us->addr & 0xFF, neighborcount,validneighborcount,childcount,validchildcount);
#endif

	if (neighborcount==validneighborcount)
	{
		if (childcount==0)
			clusterflag=1;
	}
	if ((childcount>0) && (childcount==validchildcount))
		clusterflag=1;

	if ((clusterflag) && (us->cluster->state!=WAITCLUSTER))
	{
		us->cluster->state=WAITCLUSTER;

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: have all kids, subtreesize= %d\n",us->addr & 0xFF,subtreesize);
#endif

	if ((subtreesize<K) && (us->cluster->rootid!=us->addr))
	{
		/* our subtree is given to our parent...  parent will send down CH to use
		** we need to tell parent list of nodes in sub-tree, and list of neighboring nodes.
		*/

		us->cluster->state=WAITCLUSTER;

		us->cluster->chflag=0;

		for(n=us->neighborlist;n!=NULL;n=n->next)
			if ((n->cluster) && (n->cluster->parent==us->addr) && (n->level==0))
			{
				n->cluster->clusterheadflag=0;
				n->cluster->clusterhead=NODE_BROADCAST;      /* means we find out when we get the cluster packet */
			}

#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: not enough to be a cluster passing to parent  num= %d\n",us->addr & 0xFF,subtreesize);
#endif
		sendTerm(us);

		return;
	}

	if ((subtreesize<(2*K)) || (us->cluster->rootid==us->addr))    /* our subtree is the right size to be a cluster.  */
	{
		/* clusterhead is us all recursive kids are our kids;  */

		us->cluster->state=WAITCLUSTER;

		/* mark all kids allocated.  */
		for(n=us->neighborlist;n!=NULL;n=n->next)
			if ((n->cluster) && (n->level==0) && (n->cluster->parent==us->addr))
			{
				n->cluster->clusterheadflag=0;
				n->cluster->clusterhead=us->addr;
			}

		us->cluster->chflag=1;

		/* send parent a TERM which dosn't include the kids */
		if (us->cluster->rootid!=us->addr)
			sendTerm(us);
		else
		{
			us->cluster->state=HIERARCHYINIT;
		}

		/* send kids a cluster packet indicating we are CH  */
		sendCluster(us);
		sendTerm(us);

		return;
	}

	assert(us->cluster->rootid!=us->addr);

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: got %d kids, need more than 1 cluster.\n",us->addr & 0xFF,subtreesize);
#endif

	/* Otherwise, we must make one or more clusters from our kids, and pass the leftovers up.  
	** if there are leftovers, we must be in them.
	*/


	set<neighbor*> unpchildren, tmpclusterneighbors;
	set<ManetAddr> tmpcluster;

	for(n=us->neighborlist;n!=NULL;n=n->next)
		if ((n->cluster) && (n->cluster->parent==us->addr) && (n->level==0) && (n->cluster->termvalid))
		{
			unpchildren.insert(n);
			n->clusterheadflag=0;
			n->clusterhead=NODE_BROADCAST;
		}


	/* we are node u   */

	while(!unpchildren.empty())   /* while there are unprocessed children  */    /* line 5 */
	{
		int i;
		neighbor *v,*x,*t;
		set<neighbor*>::iterator ni;
		set<ManetAddr>::iterator tm;

		ni=unpchildren.begin();
		v=*ni;
		unpchildren.erase(v);

#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: neighbor %d  subtree= %d \n",us->addr,v->addr & 0xFF, v->cluster->term.subtreesize);
		for(i=0;i<v->cluster->term.subtreesize;i++)
			fprintf(stderr,"node %d:    node= %d\n",us->addr & 0xFF,v->cluster->term.list[i] & 0xFF);
		for(i=0;i<v->cluster->term.numneighbors;i++)
			fprintf(stderr,"node %d:    neighbor= %d\n",us->addr & 0xFF,v->cluster->term.list[v->cluster->term.subtreesize + i] & 0xFF);
#endif

		tmpcluster.clear();
		tmpclusterneighbors.clear();

		tmpclusterneighbors.insert(v);
		tmpcluster.insert(v->addr);                                /* line 6.   add v and v's subtree to tmpcluster */
		for(i=0;i<v->cluster->term.subtreesize;i++)
			tmpcluster.insert(v->cluster->term.list[i]);

		while(tmpcluster.size() < K)    /* line 7 */
		{
			/* find x, in unpchildren, such that one of x's neighbors is in tmpcluster.  */

			x=NULL;
			for(ni=unpchildren.begin() ; ni!=unpchildren.end();ni++)
			{
				t=*ni;

				for(i=0;i<t->cluster->term.numneighbors;i++)  
				{
					tm=tmpcluster.find(t->cluster->term.list[i+t->cluster->term.subtreesize]);   /* if some node in the subtree of x has a member of the tmpcluster as a neighbor...  */
					if (tm!=tmpcluster.end())
					{
						x=t;
						break;
					}
				}
				if (x)
					break;
			}

			if (x)
			{
				tmpcluster.insert(x->addr);                                /* line 8  */
				tmpclusterneighbors.insert(x); 
				for(i=0;i<x->cluster->term.subtreesize;i++)
					tmpcluster.insert(x->cluster->term.list[i]);

				unpchildren.erase(x);                                     /* line 9 */
			}
			else
				break;   /* note this breaks the while tmpcluster.size() < K loop   */
		}

		if (tmpcluster.size() >=K )
		{
#ifdef DEBUG_GRAPHCLUSTER
			/* tmpcluster is a cluster.  v is the clusterhead.  */
			fprintf(stderr,"node %d: node %d is CH, for ",us->addr & 0xFF,v->addr & 0xFF);
			for(tm=tmpcluster.begin();tm!=tmpcluster.end();tm++)
				fprintf(stderr,"%d, ",*tm);
			fprintf(stderr,"\n");
#endif

			/* set allocated bit on v on all the neighbors refered to in tmpcluster */
			for(ni=tmpclusterneighbors.begin();ni!=tmpclusterneighbors.end();ni++)
			{
				if ((*ni)->addr==v->addr)
				{
					(*ni)->cluster->clusterhead=us->addr;
					(*ni)->cluster->clusterheadflag=1;
				}
				else
				{
					(*ni)->cluster->clusterhead=v->addr;
					(*ni)->cluster->clusterheadflag=0;
				}
			}

		}
		else
		{
			/* partialcluster=tmpcluster;  */
			/* the right thing will happen when we call sendTerm, since it will look at the NEIGHBOR_ALLOCATED flags */
		}
	}

	}

	/* partial cluster is passed up in a term*/

	sendCluster(us);

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: xmit term state= %d\n",us->addr,us->cluster->state);
#endif
	/* send a TERM    (which will look at the flags, and send the children we didn't put into a cluster) */
	sendTerm(us);

	return;
}


/* This is called by sendTreeInit, for if we are going to initiate the
** tree.  Or, is called by gotTree in response to receiving a Tree building packet
** Also needs to be called every P time units, during tree building period.
**
** When do we set terminate flag?  When do we expect to receive terminate flags?
**  from kids, when they have their entire sub-tree, so we set our terminate flag
**  when we get one from every kid we have?
** 
** what about doing the sub tree thing to make clusters?
**
**   If we have no neighbors...  do not send a tree packet, we're a floating root
**   If we are a root node with no neighbors, and we get a neighbor, go into no root yet state.  A floating root rejoining the MANET should join existing tree
*/
static void sendTree(manetNode *us)
{
	packet *p;
	packetBannerjeeTree *pt;

	if ((us->cluster->rootid!=us->addr) && (us->cluster->parent==NULL))     /* if we're not root, and have no parent, don't send tree */
		return;

	p=packetMalloc(us,sizeof(*pt));
	pt=(packetBannerjeeTree*)p->data;

	p->type=PACKET_GRAPHCLUSTER_TREE;
	p->dst=NODE_BROADCAST;
	p->ttl=0;

	pt->srcid=us->addr;

	if (us->cluster->parent)             /* dunno about this */
		pt->parentid=us->cluster->parent->addr;
	else
		pt->parentid=NODE_BROADCAST;            /* we have no parent yet */

	pt->treeseq=us->cluster->treeseq++;
	pt->rootid=us->cluster->rootid;
	pt->root_seqnum=us->cluster->root_seqnum;

	if (us->cluster->rootid==us->addr)
	{
		pt->parentid=us->addr;
		pt->root_distance=0;
	}
	else
	{
		pt->parentid=us->cluster->parent?us->cluster->parent->addr:NODE_BROADCAST;
		pt->root_distance=us->cluster->parent?(us->cluster->parent->cluster->root_distance+1):0xF0F0;
	}

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: sending tree.  root= %d root_distance= %d  parent= %d \n",us->addr & 0xFF,pt->rootid & 0xFF,pt->root_distance,pt->parentid);
#endif

	packetSend(us,p, PACKET_ORIGIN);
	packetFree(p);
}


/* Walk list of neighbors, finding parent node in BFS
*/
static int findParent(manetNode *us)
{
	neighbor *n,*oldparent;
	int oldroot_distance;

	oldparent=us->cluster->parent;
	oldroot_distance=(us->cluster->parent) ? us->cluster->parent->cluster->root_distance : 0xFFFF;
	us->cluster->parent=NULL;

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
//		fprintf(stderr,"node %d: findParent: neighbor %d  cluster= %p\n",us->addr,n->addr,n->cluster);

		if ((n->flags & NEIGHBOR_HEARS) && (n->cluster) && (n->level==0) && (n->cluster->parent!=us->addr)
			&& (n->cluster->rootid== us->cluster->rootid) && (n->cluster->root_seqnum==us->cluster->root_seqnum)
			&& ((n==oldparent) || (n->cluster->root_distance < oldroot_distance))
			&& ((us->cluster->parent) ? (n->cluster->root_distance < us->cluster->parent->cluster->root_distance):1))
		{
			us->cluster->parent=n;
		}
	}

	if ((us->cluster->parent!=oldparent) || (us->cluster->parent==NULL))
		return 1;
	else
		return 0;
}

/* Called when we get a term packet.
**  we will add that term packet to our neighbor list
**  then call clusterKids.  Which may send a term to our parent, and/or send clusters to our kids
**
** The Nasty Problem, for which there are lots of Nasty Timeouts, which do Nasty Things, and have been incredibly Annoying...
**  a mobility event between when the tree nodes go out, and a node finds all its kids, and then when the kids send the
**  parent TERM packets.  The problem is that some of the kids not only won't send the TERM packets, but /can't.  This stalls
**  the algorithm, clusterKids can't complete.  Thus the timeouts so that clusterKids will eventually go "screw it" and drop
**  the missing children.  The problem is the top of the tree times out first, because the timeout is based on the first
**  TERM packet heard.  
*/
static void gotTerm(manetNode *us, packet *p)
{
	neighbor *n;
	packetGraphclusterTerm *pt=(packetGraphclusterTerm*)p->data;
	int newneighbor=0;

        if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
        {
                return;
        }

	if (p->dst!=us->addr)     /* TERM packets are unicast (one hop, not multi-hop).  */
		return;

	if (us->cluster->state==NOTREEYET)
		return;

	if ((pt->rootid!=us->cluster->rootid) || (pt->root_seqnum!=us->cluster->root_seqnum))     /* make sure this term is for this instance of the algorithm */
		return;

	if ((us->cluster->parent)?(pt->root_distance <= (us->cluster->parent->cluster->root_distance+1)):1)
	{
		if (us->cluster->rootid!=us->addr)
		{
			fprintf(stderr,"node %d: got too low a term from %d  we are xx it was %d\n",us->addr & 0xFF, p->src & 0xFF, pt->root_distance);
			return;
		}
	}

	if (us->cluster->state==TREEINIT)
		us->cluster->state=WAITTERM;

	/* if we get here in WAITCLUSTER, HIERARCHYINIT, HIERARCHYREADY, then
	**   if we are CH, adopt the kid
	**   if we are not CH, pass kid a CLUSTER pointing to our CH
	*/

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: got a term from %d  dst= %d root= %d root_seqnum= %d state= %d firsttermrec= %lld root_distance= %d\n",us->addr & 0xFF,p->src & 0xFF,p->dst & 0xFF, pt->rootid,pt->root_seqnum, us->cluster->state, us->cluster->firsttermrec,pt->root_distance);
#endif

	n=neighborSearch(us,p->src,0);
	if (n==NULL)              /* we only do the TERM thing with symmetric neighbors.  */
		return;
	if (n->cluster==NULL)
	{
		newneighbor=1;
		n->cluster=(clusteringNeighbor*)malloc(sizeof(*n->cluster));
		n->cluster->root_distance=0xFFFF;
		n->cluster->parent=us->addr;
		n->cluster->rootid=pt->rootid;
		n->cluster->root_seqnum=pt->root_seqnum;
		n->cluster->termvalid=0;
		n->cluster->clusterheadflag=0;
		n->cluster->clusterhead=NODE_BROADCAST;
		n->cluster->level=0;
	}
	n->cluster->parent=us->addr;

	/* should also adopt if we got an unexpected TERM in WAITTERM  */
#if 1
	if ((us->cluster->state==WAITCLUSTER) || (us->cluster->state==HIERARCHYINIT) || (us->cluster->state==HIERARCHYREADY) && (newneighbor))
	{
#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: got a term from %d  adopted\n",us->addr & 0xFF,p->src & 0xFF);
#endif
		us->cluster->chflag=1;             /* if we get a TERM out of the blue, adopt it and become a CH  */
		n->cluster->clusterhead=us->addr;
		n->cluster->parent=us->addr;
		n->cluster->clusterheadflag=0;
	}
#endif

	if (us->cluster->firsttermrec==0)
		us->cluster->firsttermrec=us->manet->curtime;

	n->cluster->term=*pt;
	n->cluster->termvalid=1;

	clusterKids(us);    /* do we have all our kids?   */
}

/* Called on a neighbor if we are going to accept the tree that that neighbor just sent us
**
*/
static void acceptTree(manetNode *us, neighbor *n, packet *p)
{
	packetBannerjeeTree *pt=(packetBannerjeeTree*)p->data;

	n->lastheard=us->manet->curtime;

	if ((n->cluster) && (n->cluster->treeseq > pt->treeseq))    /* Tree packets arrived out of order? */
	{
		fprintf(stderr,"node %d: acceptTree: old tree\n",us->addr);
		return;
	}

	if (n->cluster==NULL)
		n->cluster=(clusteringNeighbor*)malloc(sizeof(clusteringNeighbor));

	n->cluster->parent=pt->parentid;
	n->cluster->rootid=pt->rootid;
	n->cluster->root_distance=pt->root_distance;
	n->cluster->root_seqnum=pt->root_seqnum;
	n->cluster->termvalid=0;
	n->cluster->clusterhead=NODE_BROADCAST;
	n->cluster->clusterheadflag=0;
	n->cluster->treeseq=pt->treeseq;
	n->cluster->level=0;

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: acceptTree: accepted  parent= %d root= %d root_distance= %d \n",us->addr,n->cluster->parent,n->cluster->rootid,n->cluster->root_distance);
#endif
}

static void gotTree(manetNode *us, packet *p)
{
	neighbor *n;
	packetBannerjeeTree *pt;
	int dflag=0;

        if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
        {
                return;
        }

	if (nodePacketSearch(us,p)!=NULL)    /* if we've already seen this packet, ignore it */
	{
		return;
	}
	nodePacketInsert(us,p);

#ifdef DEBUG_GRAPHCLUSTER
        fprintf(stderr,"node %d: got a tree from %d, state= %d current root= %d rootseq= %d \n",us->addr & 0xFF,p->src & 0xFF, us->cluster->state,us->cluster->rootid & 0xFF, us->cluster->root_seqnum);
#endif

	pt=(packetBannerjeeTree*)p->data;

	/* is this Tree packet for the instance we are currently honoring? */

	if ((pt->rootid == us->cluster->rootid) && (pt->root_seqnum==us->cluster->root_seqnum))
	{

		if (us->cluster->state!=TREEINIT)           /* illegal state for the current execution  */
			return;
		
		n=neighborSearch(us,p->src,0);
		if ((n==NULL) || (!(n->flags & NEIGHBOR_HEARS)))       /* we need to hear a symmetric HELLO before accepting TREE packets */
			return;

		acceptTree(us,n,p);

#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: cur= %lld  parent= %d\n",us->addr & 0xFF,us->manet->curtime,(us->cluster->parent)?(us->cluster->parent->addr & 0xFF):NODE_BROADCAST);
#endif
	}
	else
	if ( ((us->manet->curtime - us->cluster->lasttreerec) > (TIME_INITTREE*2) ) ||   /* too long ago... accept it  */
	   ((us->cluster->state==NOTREEYET)) ||        /* first tree we've heard...  */
	  ((pt->rootid < us->cluster->rootid) ||
  	  ((pt->rootid == us->cluster->rootid) && (pt->root_seqnum > us->cluster->root_seqnum))))     /* or is it higher priority?  */
	{

#ifdef DEBUG_GRAPHCLUSTER
		fprintf(stderr,"node %d: time= %lld restart from the top\n",us->addr & 0xFF,us->manet->curtime);
#endif

		clusterReset(us,pt->rootid,pt->root_seqnum);

		us->cluster->lasttreerec=us->manet->curtime;
		n=neighborSearch(us,p->src,0);

		if ((n==NULL) || (!(n->flags & NEIGHBOR_HEARD)))       /* we need to hear a symmetric HELLO before accepting TREE packets */
			return;
		acceptTree(us,n,p);
	}

	us->cluster->lasttreerec=us->manet->curtime;

	if (us->cluster->rootid!=us->addr)    /* XXX: is this check redundant?   */
		dflag=findParent(us);

#ifdef DEBUG_GRAPHCLUSTER
	fprintf(stderr,"node %d: time= %lld got a tree packet from %d parent= %d root= %d dist= %d\n",us->addr & 0xFF,us->manet->curtime,p->src & 0xFF,us->cluster->parent?us->cluster->parent->addr & 0xFF:NODE_BROADCAST,us->cluster->rootid & 0xFF,pt->root_distance);
#endif

	if (dflag)
		sendTree(us);

	/* walk list of neighbors.   Have we received a Tree packet from all of them.  Are they all = or closer to root?  Yes?  Set state to WAITCLUSTER, and send TERM  This is the leaf "short cut" to send TERMs ASAP. */
	clusterKids(us);
}
