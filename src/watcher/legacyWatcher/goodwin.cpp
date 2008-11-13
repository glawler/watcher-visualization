#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>

#include "idsCommunications.h"
#include "demolib.h"
#include "watchermovement.h"
#include "watcherGPS.h"
#include "config.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example a detector, to show how to use the Infrastructure API.
 * It detects demos.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: goodwin.cpp,v 1.23 2007/07/18 15:35:16 mheyman Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
	CommunicationsStatePtr cs;
	int reportperiod;
	int logFD;
	int apiFD;
	int watcherMovementEnable;
	void *watcherMovementState;
	int watcherMovementFD;
	manetNode *nod;

	struct detector *next;
} detector;

static void detectorSend(detector *dt)
{
	unsigned char payload[32];
	int payloadlen=0;
	WatcherGPS location;

	location.lat=dt->nod->y / 80000.0;
	location.lon=dt->nod->x / 80000.0;
	location.alt=0.0;
	payloadlen=watcherGPSMarshal(payload,sizeof(payload),&location);
	
	communicationsLogMessage(dt->cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS,payload,payloadlen);
}

static void detectorStatusUpdate(void *data,ApiStatus *as)
{
}

static void neighborUpdate(void *data, CommunicationsNeighbor *cn)
{
}

static void positionUpdate(void *data, IDSPositionType position, IDSPositionStatus status)
{
}

void gotMessage(void *data,const struct MessageInfo *mi)
{
#ifdef DEBUG_GOODWIN
	fprintf(stderr,"got a message...\n");
#endif
}

detector *detectorInit(ManetAddr us, int logfd, int watcherMovementEnable)
{
	detector *st;

	st=(detector*)malloc(sizeof(*st));
	st->logFD=logfd;
	st->reportperiod=5000;
	st->next=NULL;
	st->watcherMovementEnable=watcherMovementEnable;
	st->watcherMovementState=NULL;
	st->watcherMovementFD=-1;

	st->cs=communicationsInit(us);

	if (st->cs==NULL)
	{
		return NULL;
	}
	communicationsNameSet(st->cs,"goodwin","");

	communicationsLogEnable(st->cs,st->logFD);

	communicationsNeighborRegister(st->cs,neighborUpdate,st);
	communicationsStatusRegister(st->cs,5*1000,detectorStatusUpdate,st);
	idsPositionRegister(st->cs,COORDINATOR_NEIGHBORHOOD,IDSPOSITION_INFORM,positionUpdate,st);
	idsPositionRegister(st->cs,COORDINATOR_REGIONAL,IDSPOSITION_INFORM,positionUpdate,st);
	idsPositionRegister(st->cs,COORDINATOR_ROOT,IDSPOSITION_INFORM,positionUpdate,st);
	idsPositionRegister(st->cs,COORDINATOR_ROOTGROUP,IDSPOSITION_INFORM,positionUpdate,st);
	messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL_REMOVE,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE_REMOVE,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH_EDGE,gotMessage,st);
	messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL_REMOVE,gotMessage,st);
        messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_PROPERTY,gotMessage,st);

	if (!(st->watcherMovementEnable))
		messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS,gotMessage,st);

	return st;
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and break out every 2 seconds to
 * send messages.
 *
 * The API is not threadsafe!
 */
void selectLoop(detector *dtlist, Config *conf)
{
	fd_set readfds,writefds;
        int maxfd;
	int rc;
	struct timeval nextreport,curtime;
	struct timeval timeout;
	detector *dt;
	manet *themanet=NULL;
	int i;

	/* Make a fake manet structure, for calling the watchermovement stuff 
	 */
	themanet=(manet*)malloc(sizeof(*themanet));
	themanet->conf=conf;
	themanet->eventlist=NULL;
	themanet->ticklist=NULL;
	themanet->fdlist=NULL;
	themanet->linklayergraph=NULL;
	themanet->hierarchygraph=NULL;
	themanet->mobility=NULL;
	memset(themanet->callbackList,0,sizeof(themanet->callbackList));
	themanet->numnodes=0;
	for(dt=dtlist;dt;dt=dt->next)
		themanet->numnodes++;
	themanet->nlist=(manetNode*)malloc(sizeof(themanet->nlist[i])*themanet->numnodes);
	dt=dtlist;
	for(i=0;i<themanet->numnodes;i++)
	{
		themanet->nlist[i].index=i;
		themanet->nlist[i].addr=communicationsNodeAddress(dt->cs);
		themanet->nlist[i].netmask=ntohl(inet_addr("255.255.255.0"));
		themanet->nlist[i].bcastaddr=themanet->nlist[i].addr | (~themanet->nlist[i].netmask);
		dt->nod=&themanet->nlist[i];

		dt=dt->next;
	}

	if (dtlist->watcherMovementEnable)
		dtlist->watcherMovementState=watcherMovementInit(themanet);

	gettimeofday(&nextreport,NULL);
	nextreport.tv_sec+=2;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;

		for(dt=dtlist;dt;dt=dt->next)
		{
			dt->apiFD=communicationsReturnFD(dt->cs);
			if (dt->apiFD>0)
			{
				FD_SET(dt->apiFD,&readfds);
				GETMAXFD(maxfd,dt->apiFD);
			}
		}

		if (dtlist->watcherMovementEnable)
		{
			dtlist->watcherMovementFD=watcherMovementFD(dtlist->watcherMovementState);
			if (dtlist->watcherMovementFD>=0)    /* valid FD means watcherMovement is in IO mode (instead of polling mode)  */
			{
				FD_SET(dtlist->watcherMovementFD,&readfds);
				GETMAXFD(maxfd,dtlist->watcherMovementFD);
			}
		}

		gettimeofday(&curtime,NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			for(dt=dtlist;dt;dt=dt->next)
			{
				if (dt->apiFD<0)
				{
					fprintf(stderr,"attempting open on \n");
					communicationsReadReady(dt->cs);
				}
//				detectorSend(dt);
			}
			if ((dtlist->watcherMovementEnable) && (dtlist->watcherMovementFD<0))   /* invalid FD means watcherMovement is in polling mode  */
			{
				watcherMovementUpdate(dtlist->watcherMovementState,themanet);
				for(dt=dtlist;dt;dt=dt->next)
					detectorSend(dt);
			}

			timeout.tv_sec=dtlist->reportperiod/1000;
			timeout.tv_usec=(dtlist->reportperiod % 1000) * 1000;
			timeradd(&curtime,&timeout,&nextreport);
		}
		timersub(&nextreport,&curtime,&timeout);
#if 0
		fprintf(stderr,"entering select.  timeout= %d\n",timeout.tv_sec);
#endif
		rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

		if (rc>0)
		{
			for(dt=dtlist;dt;dt=dt->next)
			{
				if ((dt->apiFD>0) && (FD_ISSET(dt->apiFD,&readfds)))
				{
#if 0
					fprintf(stderr,"API fd readable\n");
#endif
					communicationsReadReady(dt->cs);
				}
			}

			if ((dtlist->watcherMovementEnable) && (dtlist->watcherMovementFD>=0))
			{
				if (FD_ISSET(dtlist->watcherMovementFD,&readfds))
				{
//					fprintf(stderr,"got GPS signal\n");
					if (watcherMovementRead(dtlist->watcherMovementState,themanet))
					{
						for(dt=dtlist;dt;dt=dt->next)
							detectorSend(dt);
					}
					else		
						fprintf(stderr,"GPS data not found\n");
				}
			}
		}
	}
}


void usage(void)
{
	fprintf(stderr,	"goodwin: Gathers log files for replaying with watcher (Nero Wolfe) later\n"
			"goodwin [-m] logfile ipaddr ipaddr ipaddr...\n"
			"-m flag enables calling the watcherMovement code to determine node locations\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	detector *dt=NULL,*dtlist=NULL;
	char *logname=NULL;
	int logfd;
	int i;
	int watcherMovementEnable=0;
	int ch;
	Config *conf=NULL;

	while ((ch = getopt(argc, argv, "?mc:")) != -1)
	switch (ch)
	{
		case 'm':
			watcherMovementEnable=1;
		break;
		case 'c':
			if(conf)
				configFree(conf);
            fprintf(stderr, "loading config: %s\n", optarg);
			conf=configLoad(optarg);
		break;
		case '?':
			usage();
		break;
	}
	argc -= optind;
	argv += optind;

	if (argc>0)
		logname=strdup(argv[0]);

	if ((logname) && (argc>1))
	{
		logfd=open(logname,O_WRONLY|O_CREAT|O_TRUNC,0666);
		if (logfd<0)	
		{
			perror("goodwin: open");
			exit(1);
		}
		for(i=1;i<argc;i++)
		{
			dt=detectorInit(communicationsHostnameLookup(argv[i]),logfd, watcherMovementEnable);	/* In a real detector, us=0.  */
			dt->next=dtlist;
			dtlist=dt;
		}
	}
	else
		usage();

	if (dt==NULL)
	{
		fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
		exit(1);
	}
	printf("%s: starting\n",argv[0]);
	selectLoop(dt,conf);

    return 0;
}
