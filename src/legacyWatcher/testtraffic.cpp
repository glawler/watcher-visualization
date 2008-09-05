#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "testtraffic.h"
#include "routing.h"
#include "flood.h"
#include "data.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: testtraffic.cpp,v 1.16 2006/06/23 21:41:17 tjohnson Exp $";

/*
 * This implements a simple evernode sends to every node once a second test traffic
 * pattern for testing.
 */

static void testtrafficData(manetNode *us, packet *p);
static void testtrafficAck(manetNode *us, packet *p);
void testtrafficTest(manetNode *us, void *data);

void testtrafficInit(manetNode *us)
{
	us->testtraffic=(testtrafficState*)malloc(sizeof(*us->testtraffic));

	manetPacketHandlerSet(us, PACKET_TESTTRAFFIC_DATA, testtrafficData);
	manetPacketHandlerSet(us, PACKET_TESTTRAFFIC_DATAACK, testtrafficAck);

	if (us->index==0)
		timerSet(us,testtrafficTest,30000,NULL);

	us->testtraffic->nextid=0;
}


/* Called when we get a packet...
 */
static void testtrafficData(manetNode *us, packet *p)
{
	packetTesttraffic *tp;
	tp=(packetTesttraffic*)p->data;

	fprintf(stderr,"node %d: testtraffic data, time= %lld got a packet src= %d dst= %d testid= %d\n",us->addr & 0xFF,us->manet->curtime,p->src & 0xFF,p->dst & 0xFF,tp->id);
}


static void testtrafficAck(manetNode *us, packet *p)
{
	DataPacketAck *ackpd;
	int i,c=0;

	ackpd=dataPacketAckUnmarshal(p);

	for(i=0;i<ackpd->destinationCount;i++)
		if (ackpd->destinationAck[i]==DATA_ACK)
			c++;
	fprintf(stderr,"node %d: testtraffic got an ack packet %d acked of %d\n",us->addr & 0xFF,c,ackpd->destinationCount);

	free(ackpd);
}

ManetAddr addrlist[]={
0xC0A80100 | 103,
0xC0A80100 | 104,
0xC0A80100 | 105,
0xC0A80100 | 106,
0xC0A80100 | 107,
0xC0A80100 | 108,
0xC0A80100 | 109,
0xC0A80100 | 110,
0xC0A80100 | 111,
0xC0A80100 | 112,
0xC0A80100 | 113,
0xC0A80100 | 114,
0xC0A80100 | 115,
0xC0A80100 | 116,
0xC0A80100 | 117,
0xC0A80100 | 118,
0xC0A80100 | 119,
};
void testtrafficTest(manetNode *us, void *data)
{
	packet *p;
	packetTesttraffic *tp;
	unsigned int i;
	unsigned int id;
	DataRoute routetype=DATA_ROUTE_AMBIENT;
	DataAckType   acktype=DATA_ACK_ENDTOEND;

	if (us->testtraffic->nextid<30000)
		timerSet(us,testtrafficTest,1000,NULL);

#if 0
	ManetAddr dst;
	for(i=0;i<(sizeof(addrlist)/sizeof(addrlist[0]));i++)
	{
//		dst=addrlist[i];
		dst=i;
#if 0
		if (dst==us->addr)	
			continue;
#endif

		p=packetMalloc(us,sizeof(*tp));
		tp=(packetTesttraffic*)p->data;
		tp->id=us->testtraffic->nextid;
		p->type=PACKET_TESTTRAFFIC_DATA;
		p->dst=dst;
		p->ttl=100;

		dataSend(us,p,routetype,acktype,&id);

		fprintf(stderr,"node %d: testtraffic sending test %d -> %d id %d len %d\n",us->addr & 0xFF,p->src & 0xFF,p->dst & 0xFF,tp->id,p->len);

		packetFree(p);
	}
#else
	{
		ManetAddr destlist[20];
		for(i=0;i<(sizeof(addrlist)/sizeof(addrlist[0]));i++)
			destlist[i]=i;

		p=packetMalloc(us,sizeof(*tp));
		tp=(packetTesttraffic*)p->data;
		tp->id=us->testtraffic->nextid;
		p->type=PACKET_TESTTRAFFIC_DATA;
		p->dst=100;
		p->ttl=100;
		dataSendMulti(us,p,addrlist,(sizeof(addrlist)/sizeof(addrlist[0])),routetype,acktype,&id);
		fprintf(stderr,"node %d: testtraffic sending test %d -> %d id %d len %d\n",us->addr & 0xFF,p->src & 0xFF,p->dst & 0xFF,tp->id,p->len);
	}
#endif
	us->testtraffic->nextid++;
}
