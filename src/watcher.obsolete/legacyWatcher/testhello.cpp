#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "rng.h"
#include "node.h"

#include "hello.h"
#include "testhello.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: testhello.cpp,v 1.11 2006/11/29 21:33:34 tjohnson Exp $";

typedef struct clusteringState
{
	int helloCount;
	char helloPayload[128];
} clusteringState;


static void gotNeighbor(manetNode *us, neighbor *n,int present)
{
	fprintf(stderr,"node %d: gotNeighbor: neighbor %d %s\n",us->addr & 0xFF, n->addr & 0xFF, present?"arrived":"departed");

}

static void gotCH(manetNode *us, int present)
{
	static destime chDuration[200];

	fprintf(stderr,"node %d: gotCH: we're %s CH!\n",us->addr & 0xFF,present?"a":"not a");
	if (present)
	{
		us->level=1;
		chDuration[us->index]=us->manet->curtime;
	}
	else
	{
		us->level=0;
		if (chDuration[us->index]>0)
			fprintf(stderr,"node %d: CH duration %lld\n",us->addr & 0xFF,us->manet->curtime-chDuration[us->index]);
	}
}

static void gotHello(manetNode *us, neighbor *ch, neighbor *src, const unsigned char *payload, int payloadLen)
{
	static int hadCH[200];

	fprintf(stderr,"node %u: got a Hello from %d  CH= %d   payload= %s\n",us->addr & 0xFF, src->addr & 0xFF ,(ch?ch->addr:NODE_BROADCAST ) & 0xFF,payloadLen>0?payload:(const unsigned char*)"NULL" );

	if (src->helloCoordinator==us->addr)
		src->flags|=NEIGHBOR_CHILD;
	else
		src->flags&=~NEIGHBOR_CHILD;

	if (us->clusterhead!=ch)
	{
		neighbor *n;

		for(n=us->neighborlist;n;n=n->next)
			n->flags &=~NEIGHBOR_PARENT;

		us->clusterhead=ch;
		if (us->clusterhead)
			us->clusterhead->flags|=NEIGHBOR_PARENT;
	}


	if ((hadCH[us->index]) && (ch==NULL))
		fprintf(stderr,"node %d: lost CH\n",us->addr);

	if (ch)
		hadCH[us->index]=1;
		

	if (ch)
		ch->hopcount=1;
}

/* This must be called after helloInit
 */
static void payloadUpdate(manetNode *us, void *data)
{
        timerSet(us,payloadUpdate,1000,NULL);

	sprintf(us->cluster->helloPayload,"node %d.%d.%d.%d payload %d",PRINTADDR(us->addr),us->cluster->helloCount++);
	helloPayloadSet(us,(unsigned char*)us->cluster->helloPayload,strlen(us->cluster->helloPayload)+1);
}

/* Called by simulation code to setup 
*/
void nodeInit(manetNode *us)
{
	us->cluster=(clusteringState*)malloc(sizeof(clusteringState));
	us->cluster->helloCount=0;

	us->clusterhead=NULL;
        us->neighborlist=NULL;
        us->level=0;
//        us->hellosequencenum=0;
        us->rootflag=0;

	helloInit(us,(helloNeighbor*)gotNeighbor,(helloCH*)gotCH,(helloHello*)gotHello);
	payloadUpdate(us,NULL);
}

void nodeFree(manetNode *n)
{
	helloFree(n);
}
