#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "idsCommunications.h"
#include "demolib.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example program which consumes the Infrastrucure API.
 * It may be better to look at demodetector.c
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: testapi.c,v 1.39 2007/08/17 19:43:45 dkindred Exp $";

CommunicationsDestination dst;
int zeroflag;		/* if set, then only xmit 0 length messages.  */
int hexpayload;		/* if set, dump payload in hex instead of ASCII  */
int dupcount;		/* number of duplicate messages to send, for load-testing */
int iteratekids;	/* if true, walk through all the children, and send them each a msg  */

static void status(const struct MessageInfo *mi, void *messageStatusHandlerData, MessageStatus stat)
{
	int msgnum=(int)messageStatusHandlerData;
	switch(stat)
	{
		case MESSAGE_SUCCESSFUL:
			fprintf(stderr,"message %d transmitted successfully\n",msgnum);
		break;
		default:
			fprintf(stderr,"message %d was not acked\n",msgnum);
		break;
	}
}


static void gotmsg(void *data,const struct MessageInfo *mi)
{
	char hexdigit[]="0123456789abcdef";
	char *termpayload;

	char *payload= (char*)messageInfoRawPayloadGet(mi);
	int len=messageInfoRawPayloadLenGet(mi);
	int i;

	if (hexpayload)
	{
		char *hexpayloadbuf = (char*)malloc(len*3+1);
		hexpayloadbuf[0]=0;
		for(i=0;i<len;i++)
		{
			hexpayloadbuf[i*3]=hexdigit[(payload[i]>>4) & 0x0F ];
			hexpayloadbuf[i*3+1]=hexdigit[payload[i] & 0x0F];
			hexpayloadbuf[i*3+2]=(((i % 16)==15))?'\n':' ';
		}
		hexpayloadbuf[i*3]=0;
		payload=hexpayloadbuf;
	}
	else
	{	/* make sure we're nul terminated */
		termpayload = (char*)malloc(len+1);
		memcpy(termpayload,payload,len);
		termpayload[len]=0;
		payload=termpayload;
	}
	CommunicationsDestination midst=messageInfoDestinationGet(mi);
	fprintf(stderr,"got a message from %d.%d.%d.%d to %u.%u.%u.%u mode %s len= %d payload:\n%s\n",
		PRINTADDR(messageInfoOriginatorGet(mi)),
		PRINTADDR(midst.addr),
		communicationsDestinationType2Str(midst.type),
		len,payload);
	free(payload);

}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

int typ;

static void selectLoop(CommunicationsStatePtr cs)
{
	fd_set readfds,writefds;
        int maxfd;
	int rc;
	int msgnum=1;
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

		/* do we have API connections?  */

		apifd=communicationsReturnFD(cs);
		if (apifd>=0)
		{
			FD_SET(apifd,&readfds);
			GETMAXFD(maxfd,apifd);
		}

		FD_SET(0,&readfds);


		gettimeofday(&curtime,NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			if (apifd<0)
				communicationsReadReady(cs);
			timeout.tv_sec=5000/1000;
			timeout.tv_usec=(5000 % 1000) * 1000;
			timeradd(&curtime,&timeout,&nextreport);
		}
		timersub(&nextreport,&curtime,&timeout);

		rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

		if (rc>0)
		{
			int i;

			if (FD_ISSET(0,&readfds))
			{
				MessageInfoPtr mi;
				char buff[65535];
				ssize_t rlen = read(0, buff, sizeof(buff));

				if (rlen<=0)
					exit(0);

//				buff[rlen++]=0;

				for(i=0;i<dupcount;i++)
				{
					if (iteratekids)
					{
						CommunicationsNeighbor *cn;

						for(cn=communicationsNeighborList(cs);cn!=NULL;cn=cn->next)
						{
                            char *tmp;
							dst.addr=cn->addr;
							mi=messageInfoCreate(cs,typ,dst,status,(void*)(msgnum));
							if (mi==NULL)
								fprintf(stderr,"failed to create messageInfo.\n");
							tmp=(char*)malloc(rlen);
							memcpy(tmp,buff,rlen);
							messageInfoRawPayloadSet(mi,(void*)tmp,zeroflag?0:rlen);
							messageInfoSend(mi);
							fprintf(stderr,"message %d sent %d bytes to %d.%d.%d.%d\n",msgnum,zeroflag?0:rlen,PRINTADDR(dst.addr));
							msgnum++;
						}
					}
					else
					{
                        char *tmp;
						mi=messageInfoCreate(cs,typ,dst,status,(void*)(msgnum));
						if (mi==NULL)
						{
							fprintf(stderr,"failed to create messageInfo.\n");
							continue;
						}
						tmp=(char*)malloc(rlen);
						memcpy(tmp,buff,rlen);
						messageInfoRawPayloadSet(mi,(void*)tmp,zeroflag?0:rlen);
						messageInfoSend(mi);
						fprintf(stderr,"message %d sent %d bytes\n",msgnum, zeroflag?0:rlen);
						msgnum++;
					}
				}
			}
			if (FD_ISSET(communicationsReturnFD(cs),&readfds))
			{
				fprintf(stderr,"API fd readable\n");
				if (communicationsReadReady(cs)<0)
				{
					fprintf(stderr,"selectLoop: API returned error (in reconnnect mode)\n");
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	CommunicationsStatePtr cs;
	ManetAddr attach;
	int ch;
	int chain=COMMUNICATIONS_MESSAGE_INBOUND;
	int priority=COMMUNICATIONS_MESSAGE_AFTERALL;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;
	zeroflag=0;
	hexpayload=0;
	dupcount=1;
	iteratekids=0;

	typ=IDSCOMMUNICATIONS_MESSAGE_DEMO_MESSAGE;
	attach=0;	/* IE: localhost */

	while ((ch = getopt(argc, argv, "u:hd:e:c:t:p:m:iz?")) != -1)
		switch (ch)
		{
			case 'i':
				iteratekids=1;
			break;
			case 'z':
				zeroflag=1;
			break;
			case 'h':
				hexpayload=1;
			break;
			case 'u':
				sscanf(optarg,"%d",&dupcount);
			break;
			case 'd':
				attach=communicationsHostnameLookup(optarg);
			break;
			case 'e':
				if (strcasecmp(optarg,"rootgroup")==0)
				{
					dst.addr=NODE_ROOTGROUP;
				}
				else if (strcasecmp(optarg,"all")==0
					 || strcasecmp(optarg,"broadcast")==0)
				{
					dst.addr=NODE_ALL;
				}
				else if (strcasecmp(optarg,"local")==0)
				{
					dst.addr=NODE_LOCAL;
				}
				else
				{
					dst.addr=communicationsHostnameLookup(optarg);
				}
			break;
			case 'm':
				switch (optarg[0])
				{
					case 'd':
					case 'D':
						dst.type=COMMUNICATIONSDESTINATION_DIRECT;
					break;
					case 'b':
					case 'B':
						dst.type=COMMUNICATIONSDESTINATION_BROADCAST;
					break;
					case 'n':
					case 'N':
						dst.type=COMMUNICATIONSDESTINATION_NEIGHBORSOF;
					break;
					case 'c':
					case 'C':
						dst.type=COMMUNICATIONSDESTINATION_CHILDRENOF;
					break;
					case 'p':
					case 'P':
						dst.type=COMMUNICATIONSDESTINATION_PARENTSOF;
					break;
					case 'e':
					case 'E':
						dst.type=COMMUNICATIONSDESTINATION_NEARESTCOORD;
					break;
					case 'm':
					case 'M':
						dst.type=COMMUNICATIONSDESTINATION_MULTICAST;
					break;
					default:
						fprintf(stderr,"Unknown destination addressing mode\nd - direct\nb - broadcast\nn - neighbors\nc - children of\np - parents of\ne - nearest coordinator\nm - multicast\n");
						exit(1);
					break;
				}
			break;
			case 'c':
				switch (optarg[0])
				{
					case 'i':
					case 'I':
						chain=COMMUNICATIONS_MESSAGE_INBOUND;
					break;
					case 'o':
					case 'O':
						chain=COMMUNICATIONS_MESSAGE_OUTBOUND;
					break;
					default:
						fprintf(stderr,"unknown chain!\n");
						exit(1);
					break;
				}
			break;
			case 't':
				if (strncmp(optarg,"0x",2)==0)
					sscanf(optarg,"0x%x",&typ);
				else
					sscanf(optarg,"%d",&typ);
			break;
			case 'p':
				switch (optarg[0])
				{
					case 'b':
					case 'B':
						priority=COMMUNICATIONS_MESSAGE_BEFOREALL;
					break;
					case 'a':
					case 'A':
						priority=COMMUNICATIONS_MESSAGE_AFTERALL;
					break;
					default:
						sscanf(optarg,"%d",&priority);
					break;
				}
			break;
			default:
				fprintf(stderr,"testapi - program to listen to, and send, arbitrary hierarchy messages\n"
					"-d IPaddr - node to attach to (duck)\n"
					"-e IPaddr - Node to send messages to (n.n.n.n, \"all\", \"rootgroup\", or \"local\")\n"
					"-m mode - addressing mode to use (? to get a list)\n"
					"-c [io] - receive on the input or output chain\n"
					"-t int - message type to send and receive (0xnn to specify in hex)\n"
					"-p [a|b|int] - priority on chain b= beforeall, a= afterall, or integer\n"
					"-u int - number of dUplicate messages to send\n"
					"-i - iterate through children, and send each a msg (not CHILDRENOF)\n"
					"-z - send zero length messages (no matter what payload is specified)\n"
					"-h - display payload in hex, instead of ASCII\n"
					);
				exit(1);
			break;
		}

	cs=communicationsInit(attach);
	communicationsNameSet(cs,"testapi","");

		if (cs==NULL)
	{
		fprintf(stderr,"communicationsInit failed.  errno= %d\n",errno);
		exit(1);
	}
	printf("ready!\n");

	idsPositionRegister(cs, COORDINATOR_ROOT,IDSPOSITION_ACTIVE,detectorPositionUpdate,NULL);
	idsPositionRegister(cs, COORDINATOR_REGIONAL,IDSPOSITION_ACTIVE,detectorPositionUpdate,NULL);
	idsPositionRegister(cs, COORDINATOR_NEIGHBORHOOD,IDSPOSITION_ACTIVE,detectorPositionUpdate,NULL);
	idsPositionRegister(cs, COORDINATOR_ROOTGROUP,IDSPOSITION_INFORM,detectorPositionUpdate,NULL);
	communicationsNeighborRegister(cs,detectorNeighborUpdate,NULL);

	messageHandlerSet(cs,chain,priority,COMMUNICATIONS_MESSAGE_READONLY,typ,gotmsg,NULL);

#if 0
	{
	MessageInfoPtr mi;
	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_DEMO_MESSAGE,dst,status,0);

	messageInfoRawPayloadSet(mi,(void*)strdup("blammo"),7);

	messageInfoSend(mi);
	}
#endif

	selectLoop(cs);

    return 0;
}
