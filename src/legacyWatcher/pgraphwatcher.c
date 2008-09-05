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

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example a detector, to show how to use the Infrastructure API.
 * It detects demos.
 *
 * Every 2 seconds, this detector will send a message to its coordinator.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: pgraphwatcher.c,v 1.5 2007/06/27 22:08:47 mheyman Exp $";

#define MESSAGETYPE 42

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
	int numnodes;
	ManetAddr *addr;
	CommunicationsStatePtr *cs;
	int rootflag;
	int updateflag;
} detector;

/* This is called by the API when a neighbor node arrives or departs
 * It is defined using communicationsNeighborRegister().
 *
 * the CommunicationsNeighbor * arg is READ ONLY!
 */
static void detectorNeighborUpdate(void *data, CommunicationsNeighbor *cn)
{
	detector *dt=(detector*)data;

	dt->updateflag=1;
}

static void detectorHierarchyDump(detector *dt)
{
	CommunicationsNeighbor *n;
	int i;

	printf("clear links\n");

	for(i=0;i<dt->numnodes;i++)
	{
		n=communicationsNeighborList(dt->cs[i]);
		while(n)
		{
			if (n->type==COMMUNICATIONSNEIGHBOR_PARENT)
			{
				printf("add link %d.%d.%d.%d %d.%d.%d.%d\n",PRINTADDR(dt->addr[i]),PRINTADDR(n->addr));
			}
			n=n->next;
		}
	}
}

/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void detectorSend(detector *st)
{
}

static detector *detectorInit(int numnodes, ManetAddr *node)
{
	detector *st;
	int i;

	st=(detector*)malloc(sizeof(*st));
	st->addr=(ManetAddr*)malloc(sizeof(st->addr[0])*numnodes);
	st->cs=(CommunicationsStatePtr*)malloc(sizeof(st->cs[0])*numnodes);
	st->numnodes=numnodes;
	for(i=0;i<numnodes;i++)
	{
		st->addr[i]=node[i];
		st->cs[i]=communicationsInit(node[i]);
		if (st->cs[i]==NULL)
			return NULL;

		communicationsNameSet(st->cs[i], "pgraphwatcher", "");
		communicationsNeighborRegister(st->cs[i],detectorNeighborUpdate,st);
	}

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
	struct timeval nextreport,curtime;
	struct timeval timeout;
	int i;

	gettimeofday(&nextreport,NULL);
	nextreport.tv_sec+=2;
	while(1)
	{
		dt->updateflag=0;

		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;

		for(i=0;i<dt->numnodes;i++)
		{
			FD_SET(communicationsReturnFD(dt->cs[i]),&readfds);
			GETMAXFD(maxfd,communicationsReturnFD(dt->cs[i]));
		}

		FD_SET(0,&readfds);

		gettimeofday(&curtime,NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			detectorSend(dt);
			timeout.tv_sec=2;
			timeout.tv_usec=0;
			timeradd(&curtime,&timeout,&nextreport);
		}
		timersub(&nextreport,&curtime,&timeout);
#if 0
		fprintf(stderr,"entering select.  timeout= %d\n",timeout.tv_sec);
#endif
		rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

		if (rc>0)
		{
			if (FD_ISSET(0,&readfds))
			{
				char buff[1024];
				ssize_t rlen = read(0, buff, sizeof(buff) - 1);
				buff[rlen++] = 0;
			}
			for(i=0;i<dt->numnodes;i++)
			if (FD_ISSET(communicationsReturnFD(dt->cs[i]),&readfds))
			{
#if 0
				fprintf(stderr,"API %d readable\n",i);
#endif
				if (communicationsReadReady(dt->cs[i])<0)
				{
					fprintf(stderr,"selectLoop: API returned error\n");
					exit(1);
				}
			}
		}

		if (dt->updateflag)
		{
			detectorHierarchyDump(dt);
			dt->updateflag=0;
		}
	}
}

int main(int argc, char *argv[])
{
	detector *dt;
	ManetAddr node[1024];
	int numnodes;
	int i;

	numnodes=argc-1;
	for(i=1;i<=numnodes;i++)
		 node[i-1]=ntohl(inet_addr(argv[i]));

	dt=detectorInit(numnodes,node);

	if (dt==NULL)
	{
		fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
		exit(1);
	}
	for(i=0;i<numnodes;i++)
		printf("add node %d.%d.%d.%d\n",PRINTADDR(node[i]));
	detectorHierarchyDump(dt);
	selectLoop(dt);

    return 0;
}
