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
 * This is an example rewriter, to show how to use the Infrastructure API
 * to rewrite outgoing messages.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demorewriter.c,v 1.18 2007/08/16 19:32:52 dkindred Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
	CommunicationsStatePtr cs;
	int rootflag;
	int deleteFlag;
} detector;


/* This function and the next pretty-print an xml report
 * (Call the second one, not this one.)
 */

static void childParse(FILE *fd, xmlNodePtr cur,int distance)
{
        xmlNodePtr child;
	unsigned char *p,*p2,*p3;
        fprintf(fd,"xmlnode %s origin %s\n",cur->name, p=xmlGetProp(cur,(unsigned char*)"origin"));
	xmlFree(p);

	if ((strcmp((char const*)cur->name,"report")==0) && (strcmp((char*)(p=xmlGetProp(cur,(unsigned char*)"type")),"leaf")==0))
	{
		fprintf(fd,"node %s report %s distance %d\n",p2=xmlGetProp(cur,(unsigned char*)"origin"),p3=xmlGetProp(cur,(unsigned char*)"num"),distance);
		xmlFree(p2);
		xmlFree(p3);
	}
	xmlFree(p);

	if ((strcmp((char const*)cur->name,"report")==0) && (strcmp((char*)(p=xmlGetProp(cur,(unsigned char*) "type")),"accumulated")==0))
	{
		child=cur->children;
		while(child)
		{
			if (strcmp((char const*)child->name,"child")==0)
				childParse(fd,child->children,distance+1);
			child=child->next;
		}
	}
	xmlFree(p);
}

static void detectorParse(FILE *fd, xmlDocPtr doc)
{
	xmlNodePtr p,cur;

	cur = xmlDocGetRootElement(doc);

	p=cur;
        while(p)
        {
		printf("xmlnode %s \n",p->name);
		childParse(fd,p,0);
		p=p->next;
        }

}


/* This is called by the API when a message arrives
 * It is defined using the API function messageHandlerSet(), which also takes
 * the type of message as an argument.
 *
 * It is expected that a detector will have a separate function for each message
 * type that it can handle.  However if a detector wishes to use the same one, the
 * message type can be specified to the callback by using a field in the detector
 * defined structure pointed to by the data pointer.  The value of that pointer
 * is also an argument to messageHandlerSet().
 *
 * In this case, the void pointer is used to point to the detector state structure,
 * so it can access the accumulated messages.
 *
 */
static void detectorMessageArrive(void *data,struct MessageInfo *mi)
{
	detector *st=(detector*)data;
	CommunicationsNeighbor *neigh;
	xmlDocPtr incoming=NULL;
	xmlNodePtr nod;
	unsigned char buff[1024];
	CommunicationsDestination dst;

	neigh=communicationsNeighborSearch(st->cs,messageInfoOriginatorGet(mi));
	incoming=messageInfoPayloadGet(mi);

	printf("got a message from %d len= %d payload:\n",
		messageInfoOriginatorGet(mi) & 0xFF,
		messageInfoRawPayloadLenGet(mi)
	);
	fwrite(messageInfoRawPayloadGet(mi),1,messageInfoRawPayloadLenGet(mi),stdout);

	detectorParse(stdout,incoming);

		/* we then add a child node to the report.   */
	nod=xmlNewChild(incoming->children,NULL,(unsigned char*)"rewriter",NULL);
	sprintf((char*)buff,"%d.%d.%d.%d",PRINTADDR(messageInfoOriginatorGet(mi)));
	xmlSetProp(nod,(unsigned char*)"visited",buff);
	sprintf((char*)buff,"%d",getpid());
	xmlSetProp(nod,(unsigned char*)"PID",buff);

        dst.addr=NODE_LOCAL;
        dst.type=COMMUNICATIONSDESTINATION_PARENTSOF;
        dst.ttl=255;

	/* To rewrite a message, you replace the payload on the mi, and then
	 * just send it...
	 * To delete a message, set its payload to NULL, and send it.
	 */
	if (st->deleteFlag)
		messageInfoPayloadSet(mi,NULL);   /* a NULL payload deletes msg */
	else
		messageInfoPayloadSet(mi,incoming);
	st->deleteFlag=!(st->deleteFlag);
	messageInfoSend(mi);
		/* and send takes ownership of *mi  */
		
		/* free incoming report and message */
	xmlFreeDoc(incoming);
}

/* This is called by the API when this node's position in the hierarchy changes
 * It is defined using the API function idsPositionRegister().
 */
static void myDetectorPositionUpdate(void *data, IDSPositionType position, IDSPositionStatus status)
{
	detector *st=(detector*)data;

	if (position==COORDINATOR_ROOT)
		st->rootflag=status==IDSPOSITION_ACTIVE;

	detectorPositionUpdate(data, position, status);
}

static detector *detectorInit(ManetAddr us)
{
	MessageHandlerReadWrite handler = detectorMessageArrive;
	detector *st;

	st=(detector*)malloc(sizeof(*st));
	st->cs=communicationsInit(us);
	communicationsNameSet(st->cs,"demorewriter","");
	st->deleteFlag=0;

	if (st->cs==NULL)
		return NULL;

	idsPositionRegister(st->cs, COORDINATOR_ROOT,IDSPOSITION_ACTIVE,myDetectorPositionUpdate,st);
	idsPositionRegister(st->cs, COORDINATOR_REGIONAL,IDSPOSITION_ACTIVE,detectorPositionUpdate,st);
	idsPositionRegister(st->cs, COORDINATOR_NEIGHBORHOOD,IDSPOSITION_ACTIVE,detectorPositionUpdate,st);
	communicationsNeighborRegister(st->cs,detectorNeighborUpdate,st);
	messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_OUTBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READWRITE, IDSCOMMUNICATIONS_MESSAGE_DEMO_REPORT,(MessageHandler)handler,st);
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
	int apifd;

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

		FD_SET(0,&readfds);

		gettimeofday(&curtime,NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			if (apifd<0)
				communicationsReadReady(dt->cs);
				
//			detectorSend(dt);
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
				ssize_t rlen = read(0, buff, sizeof(buff));
				buff[rlen++] = 0;
			}
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

	if (argc>1)
		us=communicationsHostnameLookup(argv[1]);

	dt=detectorInit(us);	/* In a real detector, us=0.  */

	if (dt==NULL)
	{
		fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
		exit(1);
	}
	printf("%s: starting\n",argv[0]);
	selectLoop(dt);

    return 0;
}
