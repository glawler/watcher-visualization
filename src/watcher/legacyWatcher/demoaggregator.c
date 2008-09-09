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
 * This is an example aggregator, to show how to use the Infrastructure API.
 * It listens for messages from the demodetector, and sends aggregated messages
 * every 2 seconds.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demoaggregator.c,v 1.24 2007/08/20 02:54:29 dkindred Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

typedef struct detector
{
    CommunicationsStatePtr cs;
    xmlDocPtr accumulated;
    int rootflag;
} detector;

static int messageid=0;

/* This function and the next pretty-print an xml report
 * (Call the second one, not this one.)
 */

static void childParse(FILE *fd, xmlNodePtr cur,int distance)
{
    xmlNodePtr child;
    unsigned char *p,*p2,*p3;

    fprintf(fd,"xmlnode %s origin %s\n",cur->name, p=xmlGetProp(cur,(unsigned char*)"origin"));
    xmlFree(p);
    p=NULL;

    if (strcmp((char const *)cur->name,"report") == 0 && 
        strcmp((char*)(p = xmlGetProp(cur,(unsigned char const*)"type")),"leaf")==0)
    {
        fprintf(fd,"node %s report %s distance %d\n",p2=xmlGetProp(cur,(unsigned char*)"origin"),p3=xmlGetProp(cur,(unsigned char*)"num"),distance);
        xmlFree(p2);
        xmlFree(p3);
    }
    if (p)
        xmlFree(p);
    p=NULL;
    if (strcmp((char const*)cur->name, "report")==0 && 
        strcmp((char*)(p = xmlGetProp(cur, (unsigned char const*)"type")), "accumulated") == 0)
    {
        child=cur->children;
        while(child)
        {
            if (strcmp((char const*)child->name, "report")==0)
                childParse(fd,child,distance+1);
            child=child->next;
        }
    }
    if (p)
        xmlFree(p);
}

static void childAdd(xmlNodePtr nod, xmlNodePtr incoming)    /* copy all the children of incoming to child node  */
{
    xmlNodePtr child,oldchild;
    unsigned char *p;

    child=incoming;
    while(child)
    {
        nod=xmlNewChild(nod,NULL,(unsigned char*)"report",NULL);
        xmlSetProp(nod,(unsigned char*)"type",p=xmlGetProp(child,(unsigned char*)"type"));
        xmlFree(p);
        xmlSetProp(nod,(unsigned char*)"num",p=xmlGetProp(child,(unsigned char*)"num"));
        xmlFree(p);
        xmlSetProp(nod,(unsigned char*)"origin",p=xmlGetProp(child,(unsigned char*)"origin"));
        xmlFree(p);

        oldchild=child;
        child=child->next;

        childAdd(nod,oldchild->children);
    }
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
 * message type can be determined using messageInfoTypeGet().
 *
 * In this case, the void 'data' pointer is used to point to the detector state 
 * structure, so it can access the accumulated messages.
 *
 */
static void detectorMessageArrive(void *data,const struct MessageInfo *mi)
{
    detector *st=(detector *)data;
    CommunicationsNeighbor *neigh;
    int accepted=0;
    xmlDocPtr incoming=NULL;
    unsigned char buff[1024];
    unsigned char *p;

    neigh=communicationsNeighborSearch(st->cs,messageInfoOriginatorGet(mi));
    incoming=messageInfoPayloadGet(mi);

    accepted=((neigh) && (neigh->type&COMMUNICATIONSNEIGHBOR_CHILD));	/* if the msg is from a child */
    accepted|=strcmp((char*)(p=xmlGetProp(xmlDocGetRootElement(incoming), (unsigned char*)"type")),"leaf")==0;			/* or it is a leaf report */
    xmlFree(p);

    if (accepted)
    {
        printf("got a message from %d  type= 0x%x len= %d  accepted: %d payload:\n",
	       messageInfoOriginatorGet(mi) & 0xFF,
	       messageInfoTypeGet(mi),
	       messageInfoRawPayloadLenGet(mi),
	       accepted
              );
        fwrite(messageInfoRawPayloadGet(mi),1,messageInfoRawPayloadLenGet(mi),stdout);
        detectorParse(stdout,incoming);

        if (st->accumulated==NULL)	/* do we need to make a new report?   */
        {
            st->accumulated = xmlNewDoc((unsigned char*)"1.0");
            st->accumulated->children=xmlNewDocNode(st->accumulated,NULL,(unsigned char*)"report",NULL);
            sprintf((char*)buff,"%d.%d.%d.%d",PRINTADDR(communicationsNodeAddress(st->cs)));
            xmlSetProp(st->accumulated->children,(unsigned char*)"origin",buff);
            xmlSetProp(st->accumulated->children,(unsigned char*)"type",(unsigned char*)"accumulated");
        }

        /* we then add the received report as a child of our accumulated report.   */

        childAdd(st->accumulated->children,incoming->children);    /* copy all the children of incoming to child node  */
    }
    else
    {
        printf("refused a message from from %d  len= %d  payload:\n",
                messageInfoOriginatorGet(mi) & 0xFF,
                messageInfoRawPayloadLenGet(mi)
              );
    }


    /* free incoming report and message */
    xmlFreeDoc(incoming);
}

/* Called when we get a directive.  
 * we then forward the directive to our children
 */
static void detectorDirectiveArrive(void *data,const struct MessageInfo *mi)
{
    detector *st=(detector*)data;
    MessageInfoPtr newmi;
    CommunicationsDestination dst;
    char *payload;

    printf("got a directive.  %s\n",(char*)messageInfoRawPayloadGet(mi));

    dst.addr=NODE_LOCAL;
    dst.type=COMMUNICATIONSDESTINATION_CHILDRENOF;
    dst.ttl=255;

    printf("forwarding directive as msg %d\n",messageid);
    newmi=messageInfoCreate(st->cs,IDSCOMMUNICATIONS_MESSAGE_DEMO_DIRECTIVE,dst,detectorMessageStatus,(void*)messageid);
    payload=(char*)malloc(messageInfoRawPayloadLenGet(mi));
    memcpy(payload,messageInfoRawPayloadGet(mi),messageInfoRawPayloadLenGet(mi));
    messageInfoRawPayloadSet(newmi,payload,messageInfoRawPayloadLenGet(mi));
    messageInfoSend(newmi);
    messageid++;
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


/* This is called regularly by the select loop (below)
 * It will create a message, and send it to this node's coordinator
 */
static void detectorSend(detector *st)
{
    MessageInfoPtr mi;
    CommunicationsDestination dst;

    dst.addr=NODE_LOCAL;
    dst.type=COMMUNICATIONSDESTINATION_PARENTSOF;
    dst.ttl=255;

    /* If we have any accumulated messages, forward them to our coordinator */
    if (st->accumulated)
    {
        xmlChar *mem;
        int size;

        xmlDocDumpMemory(st->accumulated,&mem,&size);                   /* convert xml struct into a xml stream  */

        printf("sending aggregated report %d  payload= %s.\n",messageid,mem);
        xmlFree(mem);
        mi=messageInfoCreate(st->cs,IDSCOMMUNICATIONS_MESSAGE_DEMO_REPORT,dst,detectorMessageStatus,(void*)messageid);
        messageInfoPayloadSet(mi,st->accumulated);
        messageInfoSend(mi);

        xmlFreeDoc(st->accumulated);
        st->accumulated=NULL;
        messageid++;
    }
}

static detector *detectorInit(ManetAddr us, const char *readlog, const char *writelog, CommunicationsMessageDirection direction, unsigned int position, CommunicationsMessageAccess mode, MessageType type)
{
    detector *st;

    st=(detector*)malloc(sizeof(*st));
    st->cs=detectorCommsInit(us,readlog,writelog,"demoaggregator");
    st->accumulated=NULL;
    st->rootflag=0;

    if (st->cs==NULL)
        return NULL;

    idsPositionRegister(st->cs, COORDINATOR_ROOT,IDSPOSITION_ACTIVE,myDetectorPositionUpdate,st);
    idsPositionRegister(st->cs, COORDINATOR_REGIONAL,IDSPOSITION_ACTIVE,detectorPositionUpdate,st);
    idsPositionRegister(st->cs, COORDINATOR_NEIGHBORHOOD,IDSPOSITION_ACTIVE,detectorPositionUpdate,st);
    communicationsNeighborRegister(st->cs,detectorNeighborUpdate,st);
    messageHandlerSet(st->cs, direction, position, mode, type, detectorMessageArrive,st);
    messageHandlerSet(st->cs,COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_DEMO_DIRECTIVE,detectorDirectiveArrive,st);
    return st;
}

static void detectorDestroy(detector *st)
{
	CommunicationsLogStatePtr cl = communicationsLogStateGet(st->cs);
	if (st->cs) communicationsClose(st->cs);
	if (cl) communicationsLogClose(cl);
	free(st);
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
    CommunicationsLogStatePtr cl = communicationsLogStateGet(dt->cs);

    communicationsTimevalGet(dt->cs, &nextreport);
    nextreport.tv_sec+=2;
    while(1)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        maxfd=-1;

        apifd=communicationsReturnFD(dt->cs);

        communicationsTimevalGet(dt->cs,&curtime);
        if (timercmp(&curtime,&nextreport,>=))
        {
            detectorSend(dt);
            timeout.tv_sec=2;
            timeout.tv_usec=0;
            timeradd(&curtime,&timeout,&nextreport);
            if (apifd<0 && cl==NULL)
            {
                communicationsReadReady(dt->cs);
                apifd=communicationsReturnFD(dt->cs);
            }
        }
        timersub(&nextreport,&curtime,&timeout);
        if (apifd>0)
        {
            FD_SET(apifd,&readfds);
            GETMAXFD(maxfd,apifd);
        }

        if (cl)
        {
            /* read events from the log */
            int step = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
#if 0
            fprintf(stderr,"time= %ld.%03ld processing next %dms of events from log\n",
                    curtime.tv_sec, curtime.tv_usec/1000, step);
#endif
            if (communicationsLogStep(cl, step, NULL) < 0)
            {
                /* EOF */
                break;
            }
        }
        else
        {
            /* get events from the daemon */
#if 0
            fprintf(stderr,"entering select.  timeout= %d\n",timeout.tv_sec);
#endif
            rc=select(maxfd+1,&readfds,&writefds,NULL,&timeout);

            if (rc>0)
            {
                if ((apifd>=0) && (FD_ISSET(apifd,&readfds)))
                {
#if 0
                    fprintf(stderr,"API fd readable\n");
#endif
                    communicationsReadReady(dt->cs);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    detector *dt;
    int ch;
    unsigned int us=0;
    const char *readlog = NULL;
    const char *writelog = NULL;
    CommunicationsMessageDirection direction=COMMUNICATIONS_MESSAGE_INBOUND;
    unsigned int position=COMMUNICATIONS_MESSAGE_AFTERALL;
    CommunicationsMessageAccess mode=COMMUNICATIONS_MESSAGE_READONLY;
    MessageType type=IDSCOMMUNICATIONS_MESSAGE_DEMO_REPORT;

    while((ch=getopt(argc, argv,"t:u:w:r:?h"))!=-1)
    {
        switch(ch)
        {
          case 'u':
              us=communicationsHostnameLookup(optarg);
              break;
          case 't':
              sscanf(optarg,"%d",&type);
              break;
          case 'w':
              writelog = optarg;
              break;
          case 'r':
              readlog = optarg;
              break;
          case '?':
          case 'h':
              fprintf(stderr,
"%s [options]\n"
"options:\n"
"-u addr - connect to a daemon on a remote machine\n"
"-t num - listen for message type <num>\n"
"-w file - write API events to the given file for later replay (see -r)\n"
"-r file - read API events (e.g., incoming messages) from the given file\n"
"          (\"-\" is stdin) instead of connecting to a daemon\n"
                      ,
					argv[0]);
				exit(1);
		}
	}

    dt=detectorInit(us,readlog,writelog,direction,position,mode,type);	/* In a real detector, us=0.  */

    if (dt==NULL)
    {
        fprintf(stderr,"detector init failed, probably could not connect to infrastructure demon.\n");
        exit(1);
    }
    printf("%s: starting\n",argv[0]);
    selectLoop(dt);
    detectorDestroy(dt);
    return 0;
}
