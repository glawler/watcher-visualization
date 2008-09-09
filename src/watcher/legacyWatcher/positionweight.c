#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>


#include "demolib.h"
#include "idsCommunications.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example a detector, to show how to use the Infrastructure API.
 * It detects demos.
 *
 * Every 2 seconds, this detector will send a message to its coordinator.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: positionweight.c,v 1.10 2007/08/19 18:23:10 dkindred Exp $";

#define MESSAGETYPE 42

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
	CommunicationsStatePtr cs;
	int rootflag;
} detector;

/* This callback is called when a message is delivered to the destination, acked, and the ack
 * delivered to the source.  
 *
 * It is specified using the messageInfoCreate() API call, when a message is sent.
 */
void detectorMessageStatus(const struct MessageInfo *mi, void *messageStatusHandlerData,MessageStatus status)
{
	int msgnum=(int)messageStatusHandlerData;
	switch(status)
	{
		case MESSAGE_SUCCESSFUL:
			printf("message %d transmitted successfully\n",msgnum);
		break;
		default:
			printf("message %d lost (might have been delivered)\n",msgnum);
		break;
	}
}


/* This is called by the API when a neighbor node arrives or departs
 * It is defined using communicationsNeighborRegister().
 *
 * the CommunicationsNeighbor * arg is READ ONLY!
 */
void detectorNeighborUpdate(void *data, CommunicationsNeighbor *cn)
{
	switch(cn->state)
	{
		case COMMUNICATIONSNEIGHBOR_ARRIVING:
			printf("node %d.%d.%d.%d arriving.  distance= %d type= %d\n",PRINTADDR(cn->addr),cn->distance,cn->type);
		break;
		case COMMUNICATIONSNEIGHBOR_DEPARTING:
			printf("node %d.%d.%d.%d departing.  distance= %d type= %d\n",PRINTADDR(cn->addr),cn->distance,cn->type);
		break;
		default:
			printf("Bad neighbor update code!\n");
		break;
	}
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

int main(int argc, char *argv[])
{
	unsigned int us=0;
	CommunicationsStatePtr cs;
	CommunicationsPositionWeight cpw;

	if (argc>1)
		 us=communicationsHostnameLookup(argv[1]);

	cs=communicationsInit(us);

	if (cs==NULL)
	{
		fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
		exit(1);
	}
	communicationsNameSet(cs, "positionweight", "");

	cpw.addr=communicationsNodeAddress(cs);
	cpw.position=COORDINATOR_ROOT;
	cpw.weight=COMMUNICATIONSPOSITIONWEIGHT_BANNED;
	cpw.next=NULL;
	communicationsPositionWeightAdd(cs,&cpw);

    return 0;
}
