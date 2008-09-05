#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>

#include "idsCommunications.h"
#include "demolib.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example detector, to show how to use the Infrastructure API.
 * It detects demos.
 *
 * Every 2 seconds, this detector will send a message to its coordinator.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demodetector.c,v 1.20 2007/08/17 19:43:45 dkindred Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
	CommunicationsStatePtr cs;
	int rootflag;
	int reportperiod;
} detector;

/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void detectorSend(detector *st)
{
	MessageInfoPtr mi;
	CommunicationsDestination dst;
	char buff[8192];
	static int messageid=0;
	xmlDocPtr doc;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	printf("detectorSend: sending report %d\n",messageid);

	doc = xmlNewDoc((unsigned char const*)"1.0");
	doc->children=xmlNewDocNode(doc,NULL,(unsigned char const*)"report",NULL);
	sprintf(buff,"%d.%d.%d.%d",PRINTADDR(communicationsNodeAddress(st->cs)));
	xmlSetProp(doc->children,(unsigned char*)"origin",(unsigned char*)buff);
	xmlSetProp(doc->children,(unsigned char*)"type",(unsigned char*)"leaf");
	sprintf(buff,"%u",messageid);
	xmlSetProp(doc->children,(unsigned char*)"num",(unsigned char*)buff);

	mi=messageInfoCreate(st->cs,IDSCOMMUNICATIONS_MESSAGE_DEMO_REPORT,dst,detectorMessageStatus,(void*)messageid);
	messageInfoPayloadSet(mi,doc);
	messageInfoSend(mi);

	xmlFreeDoc(doc);

	messageid++;
}

static detector *detectorInit(ManetAddr us, int reportperiod)
{
	detector *st;

	st=(detector*)malloc(sizeof(*st));
	st->cs=communicationsInit(us);
        communicationsNameSet(st->cs,"demodetector","");
	st->reportperiod=reportperiod;

	if (st->cs==NULL)
		return NULL;

	communicationsNeighborRegister(st->cs,detectorNeighborUpdate,st);
	return st;
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and break out every 2 seconds to
 * send messages.
 *
 * The API is not threadsafe!
 */
static void selectLoop(detector *dt)
{
	fd_set readfds,writefds;
        int maxfd;
	int rc;
	int apifd;
	struct timeval nextreport,curtime;
	struct timeval timeout;

	gettimeofday(&nextreport,NULL);
	nextreport.tv_sec+=2;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;

		apifd=communicationsReturnFD(dt->cs);
		if (apifd>0)
		{
			FD_SET(apifd,&readfds);
			GETMAXFD(maxfd,apifd);
		}

		gettimeofday(&curtime,NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			if (apifd<0)
				communicationsReadReady(dt->cs);
			detectorSend(dt);
			timeout.tv_sec=dt->reportperiod/1000;
			timeout.tv_usec=(dt->reportperiod % 1000) * 1000;
			timeradd(&curtime,&timeout,&nextreport);
		}
		timersub(&nextreport,&curtime,&timeout);
#if 0
		fprintf(stderr,"entering select.  timeout= %d\n",timeout.tv_sec);
#endif
		rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

		if (rc>0)
		{
			if ((apifd>0) && (FD_ISSET(apifd,&readfds)))
			{
#if 0
				fprintf(stderr,"API fd readable\n");
#endif
				communicationsReadReady(dt->cs);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	detector *dt;
	unsigned int us=0;
	int reportperiod=2000;

	if (argc>1)
		us=communicationsHostnameLookup(argv[1]);

	if (argc>2)
		 sscanf(argv[2],"%d",&reportperiod);

	dt=detectorInit(us,reportperiod);	/* In a real detector, us=0.  */

	if (dt==NULL)
	{
		fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
		exit(1);
	}
	printf("%s: starting\n",argv[0]);
	selectLoop(dt);

    return 0;
}
