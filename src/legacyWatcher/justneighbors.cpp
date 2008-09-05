#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "rng.h"
#include "node.h"

#include "hello.h"
#include "flood.h"
#include "marshal.h"
#include "apisupport.h"		/* for IDSStateUnmarshal() */

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: justneighbors.cpp,v 1.4 2007/07/20 04:45:17 dkindred Exp $";

/* Called by simulation code when a packet arrives at a node
*/
void nodeGotPacket(manetNode *us, packet *p)
{
}

static void gotNeighbor(manetNode *us, neighbor *n,int present)
{
	fprintf(stderr,"node %d: gotNeighbor: neighbor %d %s\n",us->addr & 0xFF, n->addr & 0xFF, present?"arrived":"departed");
}

/* callback from the hello module, which does a greatest degree thing, when it thinks
 * we should be a CH
 */
static void gotCH(manetNode *us, int present)
{
}

/* callback from the hello module, which does the greatest degree thing, when we get
 * a hello.  The hello may indicate who our CH is...
 */
static void gotHello(manetNode *us,neighbor *ch, neighbor *src, const unsigned char *payload, int payloadLen)

{
}

static void stateRec(manetNode *us, packet *p)
{
	neighbor *n,*next;
	IDSState *stateVec;
	unsigned int i;

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

	/* Delete all level==1 neighbors (hello neighbors are level 0).  We will recreate them if we still need them  */
	n=us->neighborlist;
	while(n)
	{
		next=n->next;
		if (n->level==1)
			neighborDelete(us,n);
		n=next;
	}

	for(n=us->neighborlist;n!=NULL;n=n->next)
		n->flags&=~(NEIGHBOR_PARENT|NEIGHBOR_CHILD);

	for (i = 0; i < stateVec->numNodes; i++)
	{
		ManetAddr node = stateVec->state[i].node;
		ManetAddr parent = stateVec->state[i].parent;
		int rootgroupflag = stateVec->state[i].rootgroupflag;

		fprintf(stderr,"%d.%d.%d.%d (rg:%s) to %d.%d.%d.%d\n",PRINTADDR(node),rootgroupflag ? "yes" : "no", PRINTADDR(parent));
		if (node==us->addr)
		{
			fprintf(stderr,"node %d: Got a state vector, CH= %d RG= %d\n",us->addr & 0xFF, parent & 0xFF, rootgroupflag);
			if (parent==us->addr)
			{
				fprintf(stderr,"node %d: state vector says we're root\n",us->addr & 0xFF);
				us->clusterhead=NULL;
				us->rootflag=1;
			}
			else
			{
				if (us->clusterhead)
					neighborDelete(us,us->clusterhead);
				us->clusterhead=neighborInsert(us,parent,1);
				us->clusterhead->hopcount=2;
#ifdef JUSTNEIGHBORS_DEBUG
				fprintf(stderr,"node %d: state vector says not root\n",us->addr & 0xFF);
#endif
				us->rootflag=0;
				us->clusterhead->flags|=NEIGHBOR_PARENT;
			}
			if (us->level>0)
				us->level=1;
			us->rootgroupflag=rootgroupflag;
		}
		if (parent==us->addr)
		{
			n=neighborInsert(us,node,1);
			n->hopcount=2;
			n->flags|=NEIGHBOR_CHILD;
		}
	}
	
	IDSStateFree(stateVec);
}

/* Called by simulation code to setup 
*/
void nodeInit(manetNode *us)
{
	us->clusterhead=NULL;
        us->neighborlist=NULL;
        us->level=0;
//        us->hellosequencenum=0;
        us->rootflag=0;

	helloInit(us,(helloNeighbor*)gotNeighbor,(helloCH*)gotCH,(helloHello*)gotHello);
	manetPacketHandlerSet(us, PACKET_STATEVECT, stateRec);
	manetPacketHandlerSet(us, PACKET_STATEVECT_NOFLOOD, stateRec);
	manetPacketHandlerSet(us, PACKET_STATEVECT_FLOOD, stateRec);
}

void nodeFree(manetNode *n)
{
	helloFree(n);
}
