#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "idsCommunications.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: glancer.c,v 1.13 2007/06/27 22:08:47 mheyman Exp $";

/* Program which connects to the local demon, and rattles off its neighbors.
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

static char *typetostr(CommunicationsNeighborType type)
{
	static char buff[1024];

	if (type==0)
		return "none";
	
	buff[0]=0;

	if (type & COMMUNICATIONSNEIGHBOR_ROOT)
		strcat(buff,"root");
	if (type & COMMUNICATIONSNEIGHBOR_PARENT)
		strcat(buff,"parent");
	if (type & COMMUNICATIONSNEIGHBOR_CHILD)
		strcat(buff,"child");
	return buff;
}

static void nameHandler(void *nameHandlerData, const struct ApiName * list)
{
	const ApiName *i;
	int *flag=(int*)nameHandlerData;

	printf("Clients:\n");
	printf("%5s %5s %7s %5s   %-40s\n", "send","rec","unacked","acked","name");

	for(i=list;i!=NULL;i=i->next)
		printf("%5d %5d %7d %5d   %-40s\n",i->messagesSent, i->messagesRec, i->messagesUnacked, i->messagesAcked, i->name);

	*flag=1;
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and break out every 2 seconds to
 * send messages.
 *
 * The API is not threadsafe!
 */
static void selectLoop(CommunicationsStatePtr cs, int *flag)
{
	fd_set readfds, writefds;
	int maxfd;
	int rc;
	int apifd;
	struct timeval nextreport, curtime;
	struct timeval timeout;

	gettimeofday(&nextreport, NULL);
	nextreport.tv_sec+=2;
	while(*flag==0)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;

		apifd=communicationsReturnFD(cs);
		if (apifd>=0)
		{
			FD_SET(apifd, &readfds);
			GETMAXFD(maxfd,apifd);
		}

		gettimeofday(&curtime, NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			return;
		}
		timersub(&nextreport, &curtime, &timeout);
		rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);
		if(rc>0)
		{
			if ((apifd>0) && (FD_ISSET(apifd,&readfds)))
			{
#if 0
				fprintf(stderr,"API fd readable\n");
#endif
				communicationsReadReady(cs);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	CommunicationsStatePtr cs;
	ManetAddr nod=0;
	CommunicationsNeighbor *n;
	int flag=0;

	if (argc>1)
		nod=communicationsHostnameLookup(argv[1]);
		
	cs=communicationsInit(nod);

	if ((cs==NULL) || ((cs!=NULL) && (communicationsReturnFD(cs)<0)))
	{
		fprintf(stderr,"communicationsInit() failed.\n");
		exit(1);
	}

	communicationsNameHandlerSet(cs,&flag,nameHandler);
	communicationsNameSet(cs,"glancer","");
	communicationsNameGet(cs);

	printf("%15s %20s Distance\n","Neighbor","Type");
	for( n=communicationsNeighborList(cs);n;n=n->next)
	{
		printf("%d.%d.%d.%d    %20s %2d\n",PRINTADDR(n->addr),typetostr(n->type),n->distance);
	}

	selectLoop(cs,&flag);

	communicationsClose(cs);
	return 0;
}
