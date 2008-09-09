#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "rng.h"
#include "graphics.h"
#include "bft.h"
#include "node.h"
#include "routing.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: bft.cpp,v 1.12 2007/04/23 18:51:07 dkindred Exp $";

/* Tree building packet
*/
typedef struct
{
	ManetAddr srcid;
	ManetAddr parentid;
	ManetAddr rootid;
	int root_seqnum;
	int root_distance;
	int terminateflag;
	int treeseq;
	int level;
} packetBFTTree;

typedef struct clusteringNeighbor
{
        ManetAddr rootid;
        int root_seqnum;
        int treeseq;

        int root_distance;
	int level;
        ManetAddr parent;

} clusteringNeighbor;


static RNG *rnd=NULL;


static void sendTreeInit(manetNode *us, void *data);
static void sendTree(manetNode *us);
static void sendTreeTimer(manetNode *us, void *data);
static int findCH(manetNode *us);

static void helloPacket(manetNode *us, packet *p);
static void gotTree(manetNode *us, packet *p);

/* Send a really boring hello packet
*/

static void helloSend(manetNode *us, void *data)
{
	packet *p;
        packet_hello *hp;

	neighbor *n;
        unsigned int ncount,nsymneighbors;
	int noise;

	        /* first, reschedule...  */
        noise=(int)(rnd->rand_u01()*100.0)-50;
        noise=0;
        timerSet(us,helloSend,TIME_HELLO+noise, NULL);

	p=packetMalloc(us,sizeof(packet_hello));
        p->type=PACKET_BFT_HELLO;
        p->dst=NODE_BROADCAST;
        p->ttl=0;
        hp=(packet_hello*)p->data;
        hp->sequencenum=us->cluster->hellosequencenum++;
        ncount=0;
        nsymneighbors=0;

	if (us->clusterhead)
		hp->clusterhead=us->clusterhead->addr;
	else    
		hp->clusterhead=NODE_BROADCAST;

	n=us->neighborlist;
	while(n)
	{
		if ((n->level==0) && (n->flags & NEIGHBOR_HEARD) && (ncount<(sizeof(hp->addresslist)/sizeof(hp->addresslist[0]))))
		{
			hp->addresslist[ncount]=n->addr;
			ncount++;
			if (n->flags & NEIGHBOR_HEARS)
				nsymneighbors++;
#ifdef DEBUG_HELLO
			fprintf(stderr,"node %d:    %d\n",us->addr & 0xFF,n->addr & 0xFF);
#endif
		}
		n=n->next;
	}

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
	packet_hello *hp=(packet_hello*)p->data;
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
		n=neighborInsert(us,p->src,0);
	n->lastheard=us->manet->curtime;

	n->flags|=NEIGHBOR_HEARD;    /* we heard them...  */

	if (p->hopcount < n->hopcount)          /* we actually want the minimum hopcount...  */
		n->hopcount=p->hopcount;

	if ((hp->clusterhead!=NODE_BROADCAST) && (n->clusterhead!=us->addr))
		n->clusterhead=hp->clusterhead;

	for(i=0;i<hp->onehopcount;i++)
	{
		if (hp->addresslist[i]==us->addr)
		{
			if (!(n->flags & NEIGHBOR_HEARS))
			{
#ifdef DEBUG_LINKSTATE
				fprintf(stderr,"node %d: just got a new symmetric link.\n",us->addr & 0xFF);    /* 
got a new symmetric link... call the OLSR stuff */
#endif
				n->flags|=NEIGHBOR_HEARS;   /* they listed us, so they hear us  */
				n->firstheard=us->manet->curtime;
			}
		}
	}

	if (n->clusterhead==us->addr)
		n->flags|=NEIGHBOR_PARENT;
	else
		n->flags&=~NEIGHBOR_PARENT;
}

/* Hello timeout timer
** called every HELLO interval, to remove HEARD and HEARS flags from entries from the neighbor list
** which we havn't heard a HELLO from recently enough
*/
static void helloTimeout(manetNode *us, void *data)
{
        neighbor *n;
	neighbor *d;

        timerSet(us,helloTimeout,TIME_HELLO_TIMEOUT, NULL);

        n=us->neighborlist;
        while(n)
        {
                if ((us->manet->curtime-n->lastheard) > TIME_HELLO_TIMEOUT)
                {
#ifdef DEBUG_LINKSTATE
                        if (n->flags & NEIGHBOR_HEARD)
                                fprintf(stderr,"node %d: node %d timed out\n",us->addr & 0xFF,n->addr & 0xFF);   /* lost an edge, call the OLSR stuff */
#endif
			d=n;
			n=n->next;
			neighborDelete(us,d);
                }
		else
                	n=n->next;
        }

	if ((us->clusterhead==NULL) && (us->cluster->rootid!=us->addr))    /* mobility removed our clusterhead...  find a new one from neighbors  */
	{
		findCH(us);
		if (us->clusterhead==NULL)
			us->cluster->rootid=0x0FFFFFF;     /* go forth and find a new branch of the tree */
		sendTree(us);
	}
}

static void clusterReset(manetNode *us)
{
	neighbor *n;
	
	fprintf(stderr,"node %d: clusterReset\n",us->addr & 0xFF);

        for(n=us->neighborlist;n!=NULL;n=n->next)
        {
                if (n->cluster)   
                {
			if ((n->cluster->rootid) != (us->cluster->rootid))
			{
				free(n->cluster);
				n->cluster=NULL;
			}
                }
        }
	us->level=0;
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

        us->cluster->hellosequencenum=0;
	us->cluster->mode=NOTREEYET;
	us->cluster->lasttreerec=us->manet->curtime;
	us->cluster->rootid=0x0FFFFF;
	us->cluster->treeseq=0;

	noise=(int)(rnd->rand_u01()*1000.0)+5000;
	timerSet(us,sendTreeInit,noise, NULL);
	sendTreeTimer(us, NULL);

	manetPacketHandlerSet(us, PACKET_BFT_HELLO,  helloPacket);
	manetPacketHandlerSet(us, PACKET_BFT_TREE,  gotTree);

	helloSend(us, NULL);
	helloTimeout(us, NULL);
}

void nodeFree(manetNode *n)
{
}

/* This callback is for transmitting the buildtree packet regularly.  But only in tree building state.  That check is in sendTree()
*/
static void sendTreeTimer(manetNode *us, void *data)
{
	timerSet(us,sendTreeTimer,TIME_TREE, NULL);

	if (us->neighborlist==NULL)
	{
#ifdef DEBUG_BFT
		fprintf(stderr,"node %d: we're floating root\n",us->addr & 0xFF);
#endif
		us->cluster->mode=FLOATINGROOT;
		us->cluster->rootid=NODE_BROADCAST;
		us->rootflag=1;
		us->level=0;
		us->clusterhead=NULL;
	}
	else
	{
		if (us->cluster->mode==FLOATINGROOT)
		{
			us->cluster->mode=NOTREEYET;
			us->cluster->lasttreerec=us->manet->curtime;
		}
	}

	if (us->rootflag)
	{
		us->cluster->root_seqnum++;
		sendTree(us);
	}
}

static void sendTreeInit(manetNode *us, void *data)
{
	int noise;

	/* OK, our timer went off.  If we have not heard a tree building packet, then we 
	** send one.
	*/
	noise=(int)(rnd->rand_u01()*3000.0)+(TIME_TREE*2);
	timerSet(us,sendTreeInit,noise, NULL);

	/* if we've gotten a tree, but it was too long ago, then we're back in NOTREEYET 
	*/
	if ((us->cluster->mode==TREEINIT) && ((us->manet->curtime - us->cluster->lasttreerec ) > (TIME_TREE*2)))
		us->cluster->mode=NOTREEYET;

	/* we may be in floating root...  */
	if (us->cluster->mode!=NOTREEYET)
		return;

	fprintf(stderr,"node %d: mode= %d time since last TREE= %lld\n",us->addr & 0xFF, us->cluster->mode, (us->manet->curtime - us->cluster->lasttreerec ));

	us->cluster->mode=TREEINIT;

#ifdef DEBUG_BFT
	fprintf(stderr,"node %d: time= %lld sending tree init\n",us->addr & 0xFF,us->manet->curtime);
#endif

	us->cluster->rootid=us->addr;
	us->cluster->root_seqnum++;
	us->rootflag=1;
	us->clusterhead=NULL;

	clusterReset(us);

	sendTree(us);
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
	packetBFTTree *pt;

	if (us->cluster->mode!=TREEINIT)
		return;

	us->cluster->lasttreesent=us->manet->curtime;

	p=packetMalloc(us,sizeof(*pt));
	pt=(packetBFTTree*)p->data;

	p->type=PACKET_BFT_TREE;
	p->dst=NODE_BROADCAST;
	p->ttl=0;

	pt->srcid=us->addr;

	if (us->clusterhead)             /* dunno about this */
		pt->parentid=us->clusterhead->addr;
	else
		pt->parentid=NODE_BROADCAST;            /* we have no parent yet */

	pt->treeseq=us->cluster->treeseq++;
	pt->rootid=us->cluster->rootid;
	pt->root_seqnum=us->cluster->root_seqnum;
	pt->level=us->level;
	if (us->cluster->rootid==us->addr)
		pt->root_distance=0;
	else
		pt->root_distance=us->clusterhead?(us->clusterhead->cluster->root_distance+1):0xFFFF;

#ifdef DEBUG_BFT
	fprintf(stderr,"node %d: sending tree.  root= %d root_distance= %d\n",us->addr & 0xFF,pt->rootid & 0xFF, pt->root_distance);
#endif

	packetSend(us,p, PACKET_ORIGIN);
	packetFree(p);
}


static int findCH(manetNode *us)
{
	neighbor *n,*oldparent;

	oldparent=us->clusterhead;
	n=us->neighborlist;
	us->clusterhead=NULL;

	while(n)
	{
		if ((n->cluster) && (n->cluster->parent!=us->addr) && (us->clusterhead?((n->cluster->root_distance < us->clusterhead->cluster->root_distance)):1))
			us->clusterhead=n;
		n=n->next;
	}
	return ((oldparent==us->clusterhead)?0:1);
}

static void acceptTree(manetNode *us, neighbor *n,packet *p)
{
	packetBFTTree *pt=(packetBFTTree*)p->data;
         
        n->lastheard=us->manet->curtime;
        
        if ((n->cluster) && (n->cluster->treeseq > pt->treeseq))    /* Tree packets arrived out of order? */
                return;
        
        if (n->cluster==NULL)  
                n->cluster=(clusteringNeighbor*)malloc(sizeof(clusteringNeighbor));

	n->cluster->parent=pt->parentid;
	if (n->cluster->parent==us->addr)
		n->flags|=NEIGHBOR_PARENT;
	else
		n->flags&=~NEIGHBOR_PARENT;
        n->cluster->root_distance=pt->root_distance;
        n->cluster->rootid=pt->rootid;
        n->cluster->root_seqnum=pt->root_seqnum;
	n->cluster->level=pt->level;
}

static void gotTree(manetNode *us, packet *p)
{
	neighbor *n;
	packetBFTTree *pt;
	int dflag=0;

	pt=(packetBFTTree*)p->data;

        if (p->src==us->addr)    /* one shouldn't listen to strange nodes. */
        {
                return;
        }

        if (nodePacketSearch(us,p)!=NULL)    /* if we've already seen this packet, ignore it */
        {
                return;
        }
        nodePacketInsert(us,p);

#ifdef DEBUG_BFT
	fprintf(stderr,"node %d: time= %lld got a tree packet src= %d dist= %d root= %d\n",us->addr & 0xFF,us->manet->curtime,p->src & 0xFF, pt->root_distance,pt->rootid & 0xFF);
#endif

	if (us->cluster->mode==NOTREEYET)
		us->cluster->mode=TREEINIT;

	us->cluster->lasttreerec=us->manet->curtime;

	assert(p->src!=us->addr);

	/* is this Tree packet for the instance we are currently honoring? */

	if ((pt->rootid == us->cluster->rootid) && (pt->root_seqnum==us->cluster->root_seqnum))
	{
		n=neighborSearch(us,p->src,0);
		if (n==NULL)
			n=neighborInsert(us,p->src,0);

		if (pt->parentid==us->addr)
			n->flags|=NEIGHBOR_CHILD;
		else
			n->flags&=~NEIGHBOR_CHILD;
		acceptTree(us,n,p);

		if (us->rootflag==0)
			findCH(us);

		if ((dflag) || ((us->cluster->lasttreesent+TIME_TREE)<=us->manet->curtime))
			sendTree(us);
	}
	else
	if ((pt->rootid < us->cluster->rootid) ||
  	  ((pt->rootid == us->cluster->rootid) && (pt->root_seqnum > us->cluster->root_seqnum)))     /* or is it higher priority?  */
	{
		us->cluster->rootid=pt->rootid;                               /* init from start...  */
		us->cluster->root_seqnum=pt->root_seqnum;
		us->rootflag=0;

		clusterReset(us);

		n=neighborSearch(us,p->src,0);
		if (n==NULL)
			n=neighborInsert(us,p->src,0);

		acceptTree(us,n,p);

		findCH(us);

			/* and, send a Tree packet with new data */
		sendTree(us);
	}

	/* a node's level is the level of its child node with the greatest level plus 1 */
	/* If a node has no children, its level is 0 */

	us->level=0;
#if 1
	for(n=us->neighborlist;n;n=n->next)
		if ((n->cluster) && (n->flags & NEIGHBOR_CHILD) && ((n->cluster->level+1) > us->level))
			us->level=n->cluster->level+1;
#endif
}


