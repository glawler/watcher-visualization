#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <ctype.h>
#include <limits.h>

#include <zlib.h>

#include "packetapi.h"

#include "config.h"
#include "des.h"
#include "routing.h"
#include "data.h"
#include "node.h"
#include "flood.h"

#include "apisupport.h"
#include "marshal.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: packetapi.cpp,v 1.35 2007/09/04 20:51:02 dkindred Exp $";

/* There is one of these per connected API
 */
typedef struct ApiSession
{
	struct ApiSession *next;
	int fd;				/* FD of TCP session to this client */

	ApiCommand *outgoing;		/* queue of ApiCommands which need to be written to the client */
	MessageInfo *inflight;		/* list of MI's for messages sent by this client, which are transmitted and awaiting ACKs  */

	int statusEnable;		/* If true, send this client status messages (debugging) */
	int statusPeriod;		/* How frequently does this client want status msgs?  */

	IDSPosition *positionList;	/* list of positions this client is elegible for */
	MessageTypeNode *typeList;	/* list of message types for which this client has registered */

	manetNode *node;
	int fileWriteable;              /* If true, then we've already scheduled the FD event for this ApiSession...  */

	ApiName *name;			/* name and key of this client, if set (NULL otherwise)  */
} ApiSession;

/* Per-Node PacketAPI state information:
 * 
 */
typedef struct PacketApiNodeState
{
	int tcpport;		/* TCP port demon is listening on for client traffic   */

	unsigned int nextDemonId;

	int apiacceptfd;	/* FD for incoming API connection requests  */
	ApiSession *client;	/* linked list of connected clients */

	CommunicationsNeighbor *neighborList;
	CommunicationsPositionWeight *weightList;
	IDSPositionStatus currentPositionState[COORDINATOR_MAXVAL];

	MessageTypeNode *chainIncoming;	/* chain of messageTypes for incoming messages  */
	MessageTypeNode *chainOutgoing; /* chain of messageTypes for outgoing messages */

	int doCompression;     /* if true, we will compress payloads */
	char const *failoverList;	/* list of message types to do failover routing on */
	int maxQueueLen;	/* max number of apiCommands to enqueue, before killing off the client */

	ApiStatus status;
	int statusRunning;
} PacketApiNodeState;

// #define DEBUG_SELECT

/* FD event callbacks
 */
static void apiAccept(manetNode *us, void *data);
static void apiReadable(manetNode *us, void *data);
static void apiWriteable(manetNode *us, void *data);

MessageTypeNode *messageNextChainEnqueue(MessageTypeNode *mt, MessageInfo *mi);
static void apiDelete(PacketApiNodeState *st, ApiSession *as);
static void messageNextChain(manetNode *us, ApiCommandType ac, MessageInfo *newmi);

static void apiWrite(ApiSession *as, ApiCommand *ac);
static void apiWriteAll(ApiSession *as, ApiCommand *ac);

static void idsPositionCheck(manetNode *us, void *data);
static void neighborCheck(manetNode *us, void *data);

/* packet event callbacks
 */
static void packetApiReceiveAck(manetNode *us, packet *p);
static void packetApiRoute(manetNode *us, packet *p);
static void packetApiReceive(manetNode *us, packet *p);

/* Debugging message generation
 */

static void statusStartCheck(manetNode *us);
static void statusTimeout(manetNode *us, void *data);
static void statusCheck(manetNode *us, int override);

/* Insert a messageInfo into a client's inflight list
 * (its being xmitted, and we are expecting an ACK.)
 */
static void messageInfoInsert(ApiSession *as, MessageInfo *mi);

/* Search all client's inflight messages for a message
 * The search key is the tag assigned by the data module.
 */
static void messageInfoSearch(PacketApiNodeState *st, MessageID id, MessageInfo **m, ApiSession **a);

/* Remove a messageInfo from a client's inflight list
 */
static void messageInfoRemove(ApiSession *as,MessageInfo *mi);

static void packetApiIntSend(manetNode *us, MessageInfo *mi,ApiSession *as);

/* apiApplicationRoute actually transmitts a packet.  Sortof.
 * 
 * When a packet is going to be transmitted, it is passed to apiApplicationRoute.
 * It then looks at the destination, and implements the various API application routing
 * modes.  IE: if its for PARENTSOF, that is where the address is rewritten.
 *
 * When a packet type PACKET_API_ROUTE is received, it is passed to apiApplicationRoute.
 * That will then forward it, or rewrite the packet type, and rereceive it.
 *
 * apiApplicationRoute is in simulator land.
 */
static void apiApplicationRoute(manetNode *us, packet *p,MessageInfo *mi, ApiSession *as);


/* Function for pondering the neighbor list as generated in simulator land, and converting
 * it to API land.
 */
static ApiCommand *neighborList(PacketApiNodeState *st);

/* Function for pondering the posweight list as generated in simulator land, and converting
 * it to API land.
 */
static ApiCommand *positionWeightList(PacketApiNodeState *st);

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

/* Sends an error log message to stderr.
 * Like "printf()" but to "stderr" and no return value.
 */
static void elog(char const *format, ...) 
{
    va_list arglist;
    va_start(arglist, format);    
    vfprintf(stderr, format, arglist);
    va_end(arglist);
    return;
} /* elog */


/* This is a messagehandler callback, used to write a message to an ApiSession.
 *
 * A client may actually be a simulatorland module, and thus actualy get its callback
 * called directly.  However the messageNextChainEnqueue function dosn't need to know.
 */
static void messageHandlerApiSession(void *messageHandlerData, const struct MessageInfo * mi)
{
	ApiCommand *ac;
	MessageTypeNode *mt=(MessageTypeNode*)messageHandlerData;

	ac=messageInfoMarshal(mi);       /* this makes a copy of mi's payload */
	ac->type=APICOMMAND_MESSAGE_REC;
	apiWrite(mt->api,ac);                       /* may delete mt->api!  XXX  */
}

/* Given a MessageTypeNode list, and a mhr, delete
 * the MessageTypeNode which that mhr refers to.  
 * if that MessageTypeNode does not exist, do nothing
 */
void messageTypeChainDelete(MessageTypeNode **list, MessageHandlerRequest *mhr, ApiSession *api)
{
	MessageTypeNode *top,*topprev;
	MessageTypeNode *chain,*chainprev;
	MessageTypeNode *lst,*lstprev;
	MessageInfo *mi,*minext;

	top=*list;
	topprev=NULL;

	/* walk accross top to find message type chain  */
	while(top)
	{
		if (top->type==mhr->type)
			break;
		topprev=top;
		top=top->nextType;
	}

	if (top==NULL)   /* did not find the message type.  Bail.   */
		return;

	chain=top;
	chainprev=NULL;

	/* walk down chain, looking for this client.  (search key is type,client)  */
	while(chain)
	{
		if (chain->api==api)
			break;
		chainprev=chain;
		chain=chain->nextClient;
	}

	if (chain==NULL)    /* found the message type, but did not find this api.  Bail  */
		return;

	/* chain now points to the MTN which that mhr refers to  */

		/* now that we have found the node, take all its pending MIs, and send them on to the next node */
		/* (this must be done before this node is deleted from the list */

	mi=chain->pending;
	while(mi)
	{
		minext=mi->next;
		mi->next=NULL;
		assert(mi->chainApi==api);   /* this mi should be pending on the api whose MTN we're deleteing */
		messageNextChainEnqueue(chain->nextClient,mi);
		mi=minext;
	}
	chain->pending=NULL;

		/* now back to deleting the mtn from the big data structure
		 */
	if (chainprev)
	{
		/* delete a node down in the chain.  */
		chainprev->nextClient=chain->nextClient;
	}
	else
	{
		/* delete node at top of chain */
		assert(chain==top);

		if (top->nextClient)     /* next client in chain moves up to top of chain  */
		{
			assert(top->nextClient->nextType==NULL);
			top->nextClient->nextType=top->nextType;

			if (topprev)
				topprev->nextType=top->nextClient;
			else
				*list=top->nextClient;
		}
		else
		{		/* no next client.  next type moves left.   */
			if (topprev)
				topprev->nextType=top->nextType;
			else
				*list=top->nextType;
		}
	}

	/* now do a more normal linked list delete from the client's MessageTypeNode list  (as->typeList) */
	lst=api->typeList;
	lstprev=NULL;
	while(lst)
	{
		if (lst==chain)
			break;
		lstprev=lst;
		lst=lst->nextClientType;
	}
	if(lstprev)
		lstprev->nextClientType=lst->nextClientType;
	else
		api->typeList=lst->nextClientType;

	free(chain);
}

/* Given a list of MessageTypeNodes, and a new MessageHandlerRequest, 
 * insert a new MessageTypeNode for that message type and client
 */
MessageTypeNode *messageTypeChainInsert(MessageTypeNode **list, MessageHandlerRequest *mhr, ApiSession *api)
{
	MessageTypeNode *top,*topprev;
	MessageTypeNode *chain,*chainprev;
	MessageTypeNode *newnode;

	top=*list;
	topprev=NULL;

	while(top)
	{
		if (top->type==mhr->type)
			break;
		topprev=top;
		top=top->nextType;
	}

	/* so now top points to the top of the chain for the message type in mhr  
	 */

	newnode=(MessageTypeNode*)malloc(sizeof(*newnode));
	newnode->nextClient=NULL;
	newnode->nextType=NULL;
	newnode->type=mhr->type;
	newnode->access=mhr->access;
	newnode->pending=NULL;
	newnode->handler=messageHandlerApiSession;
	newnode->handlerData=newnode;

	newnode->api=api;
	if (api)
	{
		newnode->nextClientType=api->typeList;
		api->typeList=newnode;
	}
	else
		newnode->nextClientType=NULL;

	if (top==NULL)
	{
		if (topprev)
			topprev->nextType=newnode;
		else
			*list=newnode;
		return newnode;
	}

	chain=top;
	chainprev=NULL;

	switch(mhr->priority)
	{
		case COMMUNICATIONS_MESSAGE_BEFOREALL:
			newnode->priority=top->priority;
		break;
		case COMMUNICATIONS_MESSAGE_AFTERALL:
			while(chain)
			{
				chainprev=chain;
				chain=chain->nextClient;
			}
			newnode->priority=chainprev->priority;
		break;
		default:
			while((chain) && (chain->priority < mhr->priority))
			{
				chainprev=chain;
				chain=chain->nextClient;
			}
			newnode->priority=mhr->priority;
		break;
	}

	/* now chain points to the node after where the new handler goes
	 * and chainprev is the node before chain.  (which the new handler goes after)
	 */

	 if (chainprev==NULL)  
	 {				/* insert before top in chain, which means newnode replaces top in top row  */
		newnode->nextType=top->nextType;
		newnode->nextClient=top;
		top->nextType=NULL;

		if (topprev)
			topprev->nextType=newnode;
		else
			*list=newnode;
	 }
	 else
	 {
		newnode->nextType=NULL;
		newnode->nextClient=chain;
		chainprev->nextClient=newnode;
	 }
	 return newnode;
}


/* packetApiInit - create an instance of an infrastructure demon.
 *
 */

void packetApiInit(manetNode *us)
{
	PacketApiNodeState *st;
	struct sockaddr_in incoming;
	int rc,i;
	const char *posWeightFile;
	char fullpath[PATH_MAX];

	st=(PacketApiNodeState*)malloc(sizeof(*st));
	us->packetApi=st;

	st->doCompression=configSetInt(us->manet->conf,"api_compression",1);
	st->tcpport=configSetInt(us->manet->conf,"tcpport",API_DEFAULTPORT) + us->index;
	st->failoverList=configSearchStr(us->manet->conf,"api_nofailoverlist");
	st->maxQueueLen=configSetInt(us->manet->conf,"api_maxqueuelen",200);

	st->nextDemonId=us->addr % getpid() % (getppid() << 16) % getMilliTime();
	st->neighborList=NULL;
	st->weightList=NULL;

	st->client=NULL;
	st->chainIncoming=NULL;
	st->chainOutgoing=NULL;

	st->status.packetList=(ApiPacketCount*)calloc(PACKET_MAX,sizeof(st->status.packetList[0]));
	st->status.numtypes=PACKET_MAX;
	st->status.period=5;
	st->status.level=0;
	st->status.rootflag=0;
	st->statusRunning=0;
	for(i=0;i<PACKET_MAX;i++)
		st->status.packetList[i].type=i;
	memset(st->currentPositionState,0,sizeof(st->currentPositionState));

	i=configSetInt(us->manet->conf,"simulation",0);

	if (i)			/* If we're doing a simulation, don't setup the IO stuff...   */
		return;

/* This is packetAPI
 */
	st->apiacceptfd=socket(AF_INET, SOCK_STREAM, 0);
	if (st->apiacceptfd>=0)
	{
		int optval = 1;
		if(setsockopt(st->apiacceptfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
		{
			fprintf(stderr,
			"stateInit: Failed setsockopt(SO_REUSEADDR) "
			"for api socket. \"%s\"(%d)\n",
			strerror(errno), errno);
			close(st->apiacceptfd);
			free(st);
			exit(1);
		}
	}
	else
	{
		fprintf(stderr,
			"stateInit: apiacceptfd socket failed. \"%s\"(%d)\n",
			strerror(errno), errno);
		free(st);
		exit(1);
	}

	incoming.sin_family = AF_INET;
	incoming.sin_addr.s_addr = htonl(INADDR_ANY);
	incoming.sin_port = htons(st->tcpport);

	rc = bind (st->apiacceptfd, (struct sockaddr *) &incoming,sizeof(incoming));
	if (rc<0)
	{
		fprintf(stderr,"stateInit: apiacceptfd bind failed\n");
		close(st->apiacceptfd);
		free(st);
		exit(1);
	}

	rc = listen (st->apiacceptfd, 5);
	if (rc<0)
	{
		fprintf(stderr,"stateInit: apiacceptfd listen failed\n");
		close(st->apiacceptfd);
		free(st);
		exit(1);
	}
#if 1
	fprintf(stderr,"Listening for api connections on fd=%d, port %d\n",
            st->apiacceptfd, ntohs(incoming.sin_port));
#endif
	eventFileRead(us,st->apiacceptfd,apiAccept,NULL);

#if 0
	fprintf(stderr,"local address: %d.%d.%d.%d  %d.%d.%d.%d \n",PRINTADDR(st->localaddr),PRINTADDR(st->localmask));
	for(i=0;i<st->numbcast;i++)
		fprintf(stderr,"broadcast address %d: %d.%d.%d.%d  %d.%d.%d.%d \n",i,PRINTADDR(st->localbcast[i]),PRINTADDR(st->localbcastmask[i]));
#endif

        posWeightFile = configSearchStr(us->manet->conf,"positionweightfile");
	if (posWeightFile &&
	    0 == configGetPathname(us->manet->conf,
				   posWeightFile,
				   fullpath,
				   sizeof(fullpath)))
	{
		posWeightFile = fullpath;
	}
	if (posWeightFile) 
	{
		fprintf(stderr, "%s: loading position weights from %s\n",
			__func__, posWeightFile);
	}
	communicationsPositionWeightLoad(posWeightFile,&st->weightList);


	tickSet(us, idsPositionCheck,NULL);
	tickSet(us, neighborCheck,NULL);

	manetPacketHandlerSet(us, PACKET_API_RECEIVE, packetApiReceive);
	manetPacketHandlerSet(us, PACKET_API_ROUTE, packetApiRoute);
	manetPacketHandlerSet(us, PACKET_API_RECEIVEACK, packetApiReceiveAck);
}


/* If the acceptFD is readable, this function accepts the connection, creates a new client, etc.
 */
static void apiAccept(manetNode *us, void *data)
{
	PacketApiNodeState *st=us->packetApi;
	ApiSession *as;
	ApiCommand *initlist;
	ApiCommand *posweights;
	ApiCommand *initac;
	ApiInit	init;
	int fd;
	struct sockaddr incoming;
	socklen_t incominglen;

	/* Reschedule reading the accept FD, for the next incoming client
	 */
	eventFileRead(us,st->apiacceptfd,apiAccept,NULL);

	incominglen=sizeof(incoming);
	fd=accept(st->apiacceptfd,&incoming,&incominglen);
	if (fd<0)
		return;

#ifdef DEBUG_SELECT
	if(incoming.sa_family == AF_INET)
	{
		char tmp[50];
		struct sockaddr_in *sin = (struct sockaddr_in*)&incoming;

		fprintf(stderr, "apiAccept: accepted %s:%u, fd=%d\n",
			inet_ntop(AF_INET, &sin->sin_addr, tmp, sizeof(tmp)),
			sin->sin_port, fd);
	}
	else if(incoming.sa_family == AF_INET6)
	{
		char tmp[50];
		struct sockaddr_in6 *sin = (struct sockaddr_in6*)&incoming;

		fprintf(stderr, "apiAccept: accepted %s:%u, fd=%d\n",
			inet_ntop(AF_INET6, &sin->sin6_addr, tmp, sizeof(tmp)),
			sin->sin6_port, fd);
	}
#endif

	as=(ApiSession*)malloc(sizeof(*as));

	as->fd=fd;
	as->outgoing=NULL;
	as->inflight=NULL;
	as->typeList=NULL;
	as->positionList=NULL;
	as->statusEnable=0;
	as->statusPeriod=0;
	as->node=us;
	as->fileWriteable=0;

	as->name=(ApiName*)malloc(sizeof(*as->name)+9);
	as->name->name=(char *)as->name + sizeof(*as->name);
	strcpy(as->name->name,"unknown");
	as->name->key=as->name->name+8;
	as->name->messagesSent=0;
	as->name->messagesRec=0;
	as->name->messagesUnacked=0;
	as->name->messagesAcked=0;
	as->name->key[0]=0;
	as->name->next=NULL;

	as->next=st->client;
	st->client=as;

	/* transfer current neighbor list to the new client
	 */
	initlist=neighborList(st);

	/* transfer current positions Here?  This is currently done after the client requests to be told about a position. */

	/* transfer current position weights to the new client
	 */
	posweights=positionWeightList(st);
	initlist=apiCommandConcatenate(initlist,posweights);

	/* The init ApiCommand must be last, it contain's the node's local addr,
	 * and indicates that the session is ready to go (communicationsInit()
	 * blocks until it reads the init 
	 */
	init.localid=us->addr;
	init.netmask=us->netmask;
	init.apiVersion=API_VERSION;
	initac=apiInitMarshal(&init);

	initlist=apiCommandConcatenate(initlist,initac);

	fprintf(stderr,"node %d: fd %d: accepting new client at %lld\n",
                us->addr & 0xFF, as->fd, us->manet->curtime);
#ifdef DEBUG_PACKETAPI
	{
		ApiCommand *ac;
		for(ac=as->outgoing ; ac; ac=ac->next)
		{
			fprintf(stderr, "outgoing command type %d \n",ac->type);
		}
		fprintf(stderr,"outgoing command end\n");
	}
#endif
	eventFileRead(us,as->fd,apiReadable,as);    /* schedule the new client to be reaable */
	apiWrite(as,initlist);                     /*  may delete as!  */
}

/* Called to close and delete a client connection
 * 
 * typically called on error conditions...
 */
static void apiDelete(PacketApiNodeState *st, ApiSession *as)
{
	ApiCommand *ac,*dac;
	MessageHandlerRequest mhr;
	MessageInfo *mi, *mid;

		/* linked list delete of as from the list of ApiSessions in st  */
	ApiSession *pac;

	fprintf(stderr,"node %d: fd %d: closing connection to client \"%s\" at %lld (errno %d)\n",as->node->addr & 0xFF,as->fd,as->name->name,as->node->manet->curtime,errno);

	if (st->client==as)                                 /* delete as from linked list of ApiSessions  */
		st->client=st->client->next;
	else
	{
		pac=st->client;
		while((pac) && (pac->next!=as))
			pac=pac->next;
		if (pac)
			pac->next=pac->next->next;
	}


#ifdef DEBUG_SELECT
	fprintf(stderr, "apiDelete: closed fd %d\n", as->fd);
#endif
	close(as->fd);
	eventFileClose(as->node,as->fd);     /* cancel any pending file operations */

		/* Walk list of outgoing apiCommands, and free them   */

	ac=as->outgoing;

	while(ac)
	{
		dac=ac;
		ac=ac->next;
		free(dac);
	}

	/* Walk as->inflight, and delete the MIs (these are already transmitted, data will ACK them, and data's ACK to the demon will be an ack for an unknown packet */
	mi=as->inflight;
	while(mi)
	{
		mid=mi;
		mi=mi->next;
		messageInfoDestroy(mid);
	}

	/* Find any messages owned by as (mi->origApi==as) on the outgoing chain (not yet on the air), and set origApi to NULL, as isn't going to be around to take the ACK...  */
	/* assert that there will not be any messages owned by as on the incoming chain  */

	{
		MessageTypeNode *typ;
		for(typ=st->chainOutgoing;typ;typ=typ->nextType)
        {
		    MessageTypeNode *cli;
			for(cli=typ;cli;cli=cli->nextClient)
            {
		        MessageInfo *curmi;
				for(curmi=cli->pending;curmi;curmi=curmi->next)
                {
					if (curmi->originApi==as)
                    {
						curmi->originApi=NULL;
                    }
                }
            }
        }
	}

	/* Walk as->typeList, deleting the MessageTypeNodes which as owned  (also takes pending MIs, and moves them to the next chain link) */

	while(as->typeList)
	{
		mhr.type=as->typeList->type;
		messageTypeChainDelete(&(st->chainIncoming),&mhr,as);
		messageTypeChainDelete(&(st->chainOutgoing),&mhr,as);
	}

	while(as->positionList)
	{
		IDSPositionType pos=as->positionList->position;
		idsPositionDelete(&(as->positionList),pos);
	}

	/* on any pending MIs owned by this API, set originApi to NULL  */

	if (as->name!=NULL)
		free(as->name);

	free(as);
}

/* enqueue message mi to be sent to clients on the chain mt is on, starting
 * at mt, until we hit either the end of the chain, or a READWRITE (and
 * thus we must block).  (If end of chain, it returns NULL, and caller gets
 * to do whatever.)
 *
 * Note that this doesn't care if its incoming or outgoing chain.
 *
 * if apiWrite deletes the client, the mt pointer will become invalid, thus
 * the next pointer, and access var.  
 */
MessageTypeNode *messageNextChainEnqueue(MessageTypeNode *mt, MessageInfo *mi)
{
	MessageTypeNode *next;
	CommunicationsMessageAccess access;

	while((mt!=NULL))
	{
		next=mt->nextClient;
		access=mt->access;

//		fprintf(stderr,"enqueueing message to client %d  access= %d\n", mt->api->fd,mt->access);
		if (mt->api)
			mt->api->name->messagesRec++;

		(mt->handler)(mt->handlerData,mi);

		if (access==COMMUNICATIONS_MESSAGE_READWRITE)
			break;
		else
			mt=next;
	}
	/* mt may be a dead pointer */

	if (mt)     /* if not NULL, mt->access must not equal COMMUNICATIONS_MESSAGE_READONLY */
	{
#ifdef DEBUG_PACKETAPI
		fprintf(stderr,"blocking on client %d \n",mt->api->fd);
#endif
		assert(mt->access==COMMUNICATIONS_MESSAGE_READWRITE);

		mi->next=mt->pending;	/* add this mi to MTN's pending list  */
		mt->pending=mi;

		mi->chainApi=mt->api;
	}
	return mt;
}

/* This is called in four cases:
 *  A client sends an outgoing msg.  That msg is run down chainOutgoing and xmitted
 *  An incoming msg arrived.  That msg is run down chainIncoming, and then freed
 *
 *  A client has sent back a APICOMMAND_MESSAGE_NEXT  (containing a new version of the
 *  message)
 *
 *  A client is lost.  this is called on each member of the pending list of each MessageTypeNode that client owns
 *  (XXX: we will not have an ApiCommand in this case!)
 */
static void messageNextChain(manetNode *us, ApiCommandType ac, MessageInfo *newmi)
{
	MessageTypeNode *mt=NULL,*ct=NULL;
	MessageInfo *mi=NULL,*pmi=NULL;
	CommunicationsMessageDirection direction;

	/* this message has just returned from a client.  Move it onto the next link
	 * in its chain, or xmit/drop it.  
	 */

//	fprintf(stderr,"messageNextChain: demonid= %d\n",newmi->demonId);

	/* look for this MI on all the pending lists.  */
	/* The key is ApiSession and MI->demonId  (id is not yet defined on outgoing chain)  */

	direction=COMMUNICATIONS_MESSAGE_INBOUND;
	ct=NULL;
	for(mt=us->packetApi->chainIncoming;mt!=NULL;mt=mt->nextType)   /* mt walks accross top, foreach MessageType */
	{
		for(ct=mt;ct!=NULL;ct=ct->nextClient)        /* ct walks down, foreach client in each MessageType chain  */
		{
			mi=ct->pending;
			pmi=NULL;
			while(mi)
			{
				if (mi->demonId==newmi->demonId)
					goto found;
				pmi=mi;
				mi=mi->next;
			}
		}
	}
	direction=COMMUNICATIONS_MESSAGE_OUTBOUND;
	ct=NULL;
	for(mt=us->packetApi->chainOutgoing;mt!=NULL;mt=mt->nextType)   /* mt walks accross top, foreach MessageType */
	{
		for(ct=mt;ct!=NULL;ct=ct->nextClient)        /* ct walks down, foreach client in each MessageType chain  */
		{
			mi=ct->pending;
			pmi=NULL;
			while(mi)
			{
				if (mi->demonId==newmi->demonId)
					goto found;
				pmi=mi;
				mi=mi->next;
			}
		}
	}
	mi=NULL;
	found:

	/* If mi==NULL, this is a new message (start at top of chain (IE: mt)  */

	/* mi is MessageInfo of the message, as stored when it was sent to the client for review
	 * pmi is the MessageInfo preceeding mi, in the pending list.  (to delete mi from list with)
	 * ct is the MessageTypeNode which mi is in the pending list of.
	 * mt is the top of the chain which ct is in.
	 */

	/* remove mi from the pending list it is on.  */
	if (mi)
	{
		if (pmi)
			pmi->next=mi->next;
		else
			ct->pending=mi->next;
	}

	/* Did we get a payload with 0 length?  Thats the consume command, make the msg go away   */

	if (newmi->payloadLen==0)
	{

		if (mi==NULL)   /* if mi==NULL, we're at the top of the chain, and couldn't possibly have gotten a consume command.  SO: someone is lieing.  Drop the message.  */
		{
		    ApiCommand *tmp;
			tmp=apiCommandMalloc(APICOMMAND_MESSAGE_NAK,0);
			tmp->tag=newmi->tag;
			apiWrite(newmi->originApi,tmp);		/* which takes ownership of tmp, and will free when it writes.  may delete newmi->originApi */
			messageInfoDestroy(newmi);

			return;
		}

//		fprintf(stderr,"messageNextChain: got a consume.  plonk!  direction= %d api= %p\n",direction, mi->originApi);

		if (direction==COMMUNICATIONS_MESSAGE_OUTBOUND)
		{
			/* if its an outgoing msg, tell the client which sent the msg it was not delivered.   */
			if (mi->originApi)
			{
		        ApiCommand *tmp;
				tmp=apiCommandMalloc(APICOMMAND_MESSAGE_NAK,0);
				tmp->tag=mi->tag;
				apiWrite(mi->originApi,tmp);		/* which takes ownership of tmp, and will free when it writes.  may delete mi->originApi */
			}
		}

		messageInfoDestroy(newmi);   /* also gets tmp  */
		if (newmi!=mi)
			messageInfoDestroy(mi);
		return;
	}
	
	if (mi)
	{
		newmi->originApi=mi->originApi;     /* we're going to be inserting newmi into the list and deleteing mi, preseve original api of message, so we know where to send the ack/nak later  */
		newmi->tag=mi->tag;
		messageInfoDestroy(mi);
		mi=NULL;
	}


	if (ct)	/* we're on a chain, go onto the next link  */
	{
		assert(ac==APICOMMAND_MESSAGE_NEXT);
		ct=messageNextChainEnqueue(ct->nextClient,newmi);
	}
	else
	{	/* start at top of chain.  */
		/* incoming msgs call messageNextChainEnqueue in apiPacket.  */
		assert(ac==APICOMMAND_MESSAGE_SEND);
		ct=messageNextChainEnqueue(messageTypeHandlerSearch(&(us->packetApi->chainOutgoing),newmi->type),newmi);
	}

	/* ct now points to next link... 
	 * Is ct NULL?  Then we've hit the bottom of the chain.
	 * if outgoing, really xmit, if incoming free it  (determined by which pending list we found it on)
	 */

	if (ct==NULL)   /* if we're at the bottom of the chain...  */
	{
		if (direction==COMMUNICATIONS_MESSAGE_OUTBOUND)   /* if we're on the bottom of the outgoing chain... */
		{
			packet *p;
			PacketApi pa;
			CommunicationsDestination dst;

			dst=messageInfoDestinationGet(newmi);

			/* begin simulator land */

			pa.type=messageInfoTypeGet(newmi);
			pa.origdest.addr=dst.addr;  
			pa.origdest.type=dst.type;
			pa.origdest.ttl=dst.ttl;
			pa.desttype=dst.type;       /* This may be rewritten in the process of routing */
			pa.payload=newmi->payload;
			pa.payloadLen=newmi->payloadLen;

			p=packetApiMarshal(us, &pa,us->packetApi->doCompression);

#ifdef DEBUG_APPROUTING
			fprintf(stderr,"API message send:  dst= %d dsttype= %s type= 0x%x len= %d clen= %d wrappertype= %x\n",pa.origdest.addr & 0xFF,communicationsDestinationType2Str(pa.origdest.type),pa.type,pa.payloadLen,p->len,PACKET_API_RECEIVE);
#endif

			apiApplicationRoute(us,p,newmi,newmi->originApi);

			packetFree(p);

			/* end simulator land */
		}
		else
		{
			/* was newmi inserted into a pending list?  ct is which pending list it would have been in...  so no it wasn't  */
			messageInfoDestroy(newmi);   /* also gets ac  */
		}
	}
	/* Otherwise, messageNextChainEnqueue has enqueued the msg to go to another client, and we're blocking...
	 * ac is pointed to by MI which is now on the pending list, so don't free ac.
	 */
}

/* Given a messageHandlerRequest, either insert or delete it...
 */
static MessageTypeNode *messageHandlerRequestHandle(manetNode *us, ApiSession *as, MessageHandlerRequest *mhr)
{
	if (mhr->request==APICOMMAND_MESSAGETYPE_REQUEST)
	{
#ifdef DEBUG_PACKETAPI
		fprintf(stderr,"got a message type request type=%s priority= %d direction= %d fd= %d\n", messageType2Str(mhr->type), mhr->priority, mhr->direction, as?as->fd:-1);
#endif
		switch (mhr->direction)
		{	
			case COMMUNICATIONS_MESSAGE_INBOUND:
				return messageTypeChainInsert(&(us->packetApi->chainIncoming),mhr,as);
			break;
			case COMMUNICATIONS_MESSAGE_OUTBOUND:
				return messageTypeChainInsert(&(us->packetApi->chainOutgoing),mhr,as);
			break;
			default:
				fprintf(stderr,"%s: illegal message direction\n",__func__);
				abort();
			break;
		}
	}
	else
	{
#ifdef DEBUG_PACKETAPI
		fprintf(stderr,"got a message type remove type=%s  fd= %d\n", messageType2Str(mhr->type), as?as->fd:-1);
#endif
		switch (mhr->direction)
		{	
			case COMMUNICATIONS_MESSAGE_INBOUND:
				messageTypeChainDelete(&(us->packetApi->chainIncoming),mhr,as);
			break;
			case COMMUNICATIONS_MESSAGE_OUTBOUND:
				messageTypeChainDelete(&(us->packetApi->chainOutgoing),mhr,as);
			break;
			default:
				fprintf(stderr,"%s: illegal message direction\n",__func__);
				abort();
			break;
		}
		return NULL;
	}
}


/* Called when a client session is readable.
 * Reads the command, and may schedule packets to be transmitted or update data structures
 *
 */
static void apiReadable(manetNode *us, void *data)
{
	ApiSession *as=(ApiSession*)data;
	ApiCommand *ac;

#ifdef DEBUG_PACKETAPI
	fprintf(stderr,"node %d: reading from fd %d\n", us->addr & 0xFF, as->fd);
#endif

	ac=apiCommandRead(as->fd, elog);

	if (ac==NULL)
	{
		apiDelete(us->packetApi,as);
		return;
	}

	eventFileRead(us, as->fd, apiReadable, as);    /* apiDelete() should deal with this correctly  */

	switch (ac->type)
	{
		case APICOMMAND_MESSAGE_SEND:
		{
			MessageInfo *mi;

			mi=messageInfoUnmarshal(ac);
			packetApiIntSend(us, mi, as);
		}
		break;
		case APICOMMAND_MESSAGE_NEXT:
		{
			MessageInfo *mi;

//			fprintf(stderr,"got a chained message from client %d\n",as->fd);

			/* Walk down state->chainOutgoing, to find messagetype of outgoing msg  */
			/* if NULL, xmit now  */
			/* Otherwise, send MI to client, and if blocking put on pending list.  Otherwise go to nextClient */
			/* if NULL, xmit now  */

			mi=messageInfoUnmarshal(ac);
			mi->originApi=NULL;
			mi->chainApi=NULL;
			messageNextChain(us, ac->type, mi);
#warning does this message have a valid demon ID?  It should  What if its origin client was deleted?

		}
		break;
		case APICOMMAND_STATUS:	
		{
			/* client sent us a request to send it STATUS messages.    (They are debugging...  used by watcher and not officially exported)
			 */
			ApiStatus *status;
			ApiSession *asi;
		
			status=apiStatusUnmarshal(ac);

#ifdef DEBUG_PACKETAPI
			fprintf(stderr,"node %d: status: got APICOMMAND_STATUS period= %d\n",us->addr & 0xFF,status->period);
#endif

			if (status->period<0)     /* disable all status messages...  */
			{
				as->statusEnable=0;
			}
			else
			{ 				/* or, enable either 0 (status change only) or >0 (periodic) */
				as->statusEnable=1;
				as->statusPeriod=status->period;
			}

			/* find shortest period, of enabled clients.  */
			us->packetApi->status.period=0x7FFFFFFF;
			for(asi=us->packetApi->client;asi;asi=asi->next)
				if (asi->statusEnable)
					if ((asi->statusPeriod>0) && (asi->statusPeriod<us->packetApi->status.period))
						us->packetApi->status.period=asi->statusPeriod;

			if (us->packetApi->status.period==0x7FFFFFFF)
				us->packetApi->status.period=0;

			statusStartCheck(us);   /* start callback if needed...  */

			/* If we are not running, schedule the callback here
		 	 */

			free(status);
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_MESSAGETYPE_REQUEST:
		case APICOMMAND_MESSAGETYPE_REMOVE:
		{
			/* update list of accepted types */

			/* This needs the whole chain building thing  */

			MessageHandlerRequest *mhr;

			mhr=messageHandlerRequestUnmarshal(ac);
			messageHandlerRequestHandle(us,as,mhr);
			apiCommandFree(ac);
			free(mhr);
		}
		break;
		case APICOMMAND_POSITION:
		{
			IDSPositionType position;
			IDSPositionStatus stat;
			idsPositionUnmarshal(ac,&position,&stat);

#ifdef DEBUG_PACKETAPI
			fprintf(stderr,"got a position request position= %s, status= %s\n", idsPosition2Str(position), idsPositionStatus2Str(stat));
#endif

			switch(stat)
			{
				case IDSPOSITION_INFORM:
				case IDSPOSITION_ACTIVE:
					idsPositionInsert(&(as->positionList),position,stat,NULL,NULL);

				break;
				case IDSPOSITION_INACTIVE:
					idsPositionDelete(&(as->positionList),position);
				break;
				default:
					fprintf(stderr,"sanity fail\n");
					abort();
			}
			/* If this is a new client which has requested a position, and the node is already in that
			 * position, it will be sent an update when we calll idsPositionCheck, and note that the node
			 * is active, but the client is not
			 */
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_POSITIONWEIGHT:
			CommunicationsPositionWeight *cpw,*newentry;

			cpw=communicationsPositionWeightUnmarshal(ac);

			newentry=communicationsPositionWeightInsert(&(us->packetApi->weightList),cpw);

			free(cpw);
			apiCommandFree(ac);

			ac=communicationsPositionWeightMarshal(newentry);     /* tell all the clients the new value  */
			apiWriteAll(us->packetApi->client,ac);
				/* do not free ac.  apiWriteAll takes ownership  */
		break;
		case APICOMMAND_RAWSEND:
		{
			packet *p;
			MessageInfo *mi;
			mi=messageInfoUnmarshal(ac);

			p=packetMalloc(us,mi->payloadLen);
			memcpy(p->data,mi->payload,mi->payloadLen);
			p->dst=mi->dest.addr;
			if (p->dst==NODE_LOCAL)
				p->dst=us->addr;
			p->src=mi->origin;
			p->type=mi->type & 0xFF;
#ifdef DEBUG_PACKETAPI
			fprintf(stderr,"got an APICOMMAND_RAWSEND, sending type 0x%x len %d to %d.%d.%d.%d\n",p->type,p->len,PRINTADDR(p->dst));
#endif
			if (p->dst==us->addr)
				packetReReceive(us,p);
			else
				packetSend(us,p, PACKET_ORIGIN);
			packetFree(p);

			messageInfoDestroy(mi);
		}
		break;
		case APICOMMAND_NAMESET:
		{
			ApiName *an, *oldname;

			an=apiNameUnmarshal(ac);
			if (an!=NULL)
			{
				fprintf(stderr,"node %d: fd %d: client name set to \"%s\" at %lld\n",us->addr & 0xFF, as->fd, an->name, us->manet->curtime);
				oldname=as->name;
				as->name=an;
				if (oldname)
				{
					as->name->messagesSent+=oldname->messagesSent;
					as->name->messagesAcked+=oldname->messagesAcked;
					as->name->messagesRec+=oldname->messagesRec;
					as->name->messagesUnacked+=oldname->messagesUnacked;

					apiNameFree(oldname);
				}
			}
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_NAMEGET:
		{
			ApiSession *i;
			ApiName *list=NULL;
			
#ifdef DEBUG_PACKETAPI
			fprintf(stderr,"client on fd %d: requested names\n",as->fd);
#endif

			for(i=us->packetApi->client;i!=NULL;i=i->next)
			{
#ifdef DEBUG_PACKETAPI
				fprintf(stderr,"client %s  fd= %d...\n",i->name->name,i->fd);
#endif
				i->name->next=list;
				list=i->name;
			}

			apiCommandFree(ac);

			ac=apiNameMarshal(list);
			ac->type=APICOMMAND_NAMEGET;
			apiWrite(as,ac);                   /* may delete as!   */
		}
		break;
		default:
			fprintf(stderr,"apiReadable: unknown API command %d on fd %d\n",ac->type,as->fd);
			errno=0;
			apiDelete(us->packetApi,as);   /* Assume its evil, and close.  */
			apiCommandFree(ac);
		break;
	}
}

/* Called when an API is writeable...
 */
static void apiWriteable(manetNode *us, void *data)
{
	ApiSession *as=(ApiSession*)data;
	ApiCommand *ac;
	int rv;
#ifdef DEBUG_PACKETAPI
	fprintf(stderr,"select: write on api %d\n",as->fd);
#endif

	ac=as->outgoing;

	if (ac==NULL)
	{
		as->fileWriteable=0;
		return;
	}

	as->outgoing=as->outgoing->next;
	ac->next=NULL;
	rv=apiCommandWriteOrSend(as->fd,ac,1);
	apiCommandFree(ac);

	if (rv<0)               /* write failed...  nuke the client.  */
	{
		apiDelete(us->packetApi,as);
		return;      /* as is now a dead pointer, do not do the outgoing thing below */
	}

	if (as->outgoing)
	{
		eventFileWrite(us,as->fd,apiWriteable,as);
		as->fileWriteable=1;
	}
	else
		as->fileWriteable=0;

}

/* enqueue a list of apiCommands to be sent a client connection refered to by as
 * may delete as.  So, do not refer to as after calling this.  Wait for the next
 * callback.
 */
static void apiWrite(ApiSession *as, ApiCommand *ac)
{
	ApiCommand *last;
	int len=0;

	if (as==NULL)
	{
		apiCommandFree(ac);
		return;
	}

	last=as->outgoing;

	if (ac==NULL)
		return;

	if (last==NULL)
	{
		as->outgoing=ac;
		len=0;
	}
	else
	{
		while(last->next!=NULL)
		{
			last=last->next;
			len++;
		}

		last->next=ac;
	}

//	fprintf(stderr,"apiWrite: queue for %s len= %d\n",as->name->name,len);

	if (len>as->node->packetApi->maxQueueLen)
	{
		fprintf(stderr,"apiWrite: queue for %s len= %d, closing\n",as->name->name,len);
		apiDelete(as->node->packetApi,as);
		return;
		/* schedule the client to be deleted.   this may be called within another loop,
		 * like apiWriteAll, so we really do want to schedule it, not do it here */
	}

	/* schedule callback for writing  */
	if (as->fileWriteable==0)
	{
		eventFileWrite(as->node,as->fd,apiWriteable,as);
		as->fileWriteable=1;
	}
}

/* enqueue an apicommand on every client.  for n clients, does n-1 copies, instead of n.
 * takes ownership of ac.
 */
static void apiWriteAll(ApiSession *as, ApiCommand *ac)
{
	ApiCommand *newac;
	ApiSession *next;

	if (as == NULL)
	{
		/* no clients to write to */
		apiCommandFree(ac);
		return;
	}
	while(as)
	{
		next=as->next;

		if (next)		/* If we're not on the last one, make a copy of ac.  otherwise on the last one, use ac itself */
			newac=apiCommandCopy(ac);
		else
			newac=ac;


		next=as->next;
		apiWrite(as,newac);
		as=next;                   /* apiWrite may delete the client, so don't follow as after apiWrite is called.  */
	}
}


/*********************************************************************************************
 *
 * Functions to handle packets coming in from the MANET
 *
 */


void packetApiRoute(manetNode *us, packet *p)
{
#ifdef DEBUG_APPROUTING
	fprintf(stderr,"node %d: packetApiPacket: got a PACKET_API_ROUTE packet\n",us->addr & 0xFF);
#endif
	apiApplicationRoute(us,p,NULL,NULL);
}


/* Called when we get an incoming data packet from the network.    (IE:
 * not an ack)
 */
static void packetApiReceive(manetNode *us, packet *p)
{
	PacketApi *pa;
	MessageInfo *mi;
	MessageTypeNode *ct;

	if (!((p->dst==us->addr) || (p->dst==NODE_BROADCAST)
	      || (p->dst==NODE_ROOTGROUP && us->rootgroupflag)))
		return;

#ifdef DEBUG_APPROUTING
	fprintf(stderr,"node %d: packetApiReceive: got a PACKET_API_RECEIVE  packet len= %d\n",us->addr & 0xFF,p->len);
#endif

	pa=packetApiUnmarshal(p,us->packetApi->doCompression);

	if (pa==NULL)
	{
		fprintf(stderr,"Got a corrupted PACKET_API src= %d dst= %d len= %d ttl= %d hopcount= %d\n",p->src & 0xFF,p->dst,p->len,p->ttl, p->hopcount);
		return;
	}

#ifdef DEBUG_APPROUTING
	fprintf(stderr,"got incoming PACKET_API type 0x%x headerlen= %d payloadLen= %d\n",pa->type,PACKETAPI_HEADERLEN, pa->payloadLen);
#endif

	mi=(MessageInfo*)malloc(sizeof(*mi));
	mi->tag=0;			/* incoming packets have no tag  */
					/* incoming packets don't have an ID (well, they do at the data
					** module, but it deals with the ACK) */
	mi->dataId=0;
	mi->demonId=us->packetApi->nextDemonId++;
	mi->statusCallback=NULL;
	mi->statusData=NULL;
	mi->origin=p->src;
	mi->dest.addr=pa->origdest.addr;
	mi->dest.type=pa->origdest.type;
	mi->dest.ttl=pa->origdest.ttl;
	mi->type=pa->type;
	mi->next=NULL;
	mi->routeFlags=0;

	mi->payload=pa->payload;
	mi->payloadPtr=pa;
	mi->payloadLen=pa->payloadLen;

	ct=messageNextChainEnqueue(messageTypeHandlerSearch(&us->packetApi->chainIncoming,mi->type),mi);
	if (ct==NULL)
		messageInfoDestroy(mi);   /* if ct!=NULL, mi was put onto a pending list  */
}


/* This is called when we get an ACK packet for a data packet we sent.
 * and we WILL get an ACK packet, because our local data module will send us one even if it dosn't get the ACK packet it expected
 */
static void packetApiReceiveAck(manetNode *us, packet *p)
{
	DataPacketAck *pd;
	MessageInfo *mi;
	ApiCommand *ac;
	ApiSession *as;
	int allack=1;
	int i;

#ifdef DEBUG_APPROUTING
	fprintf(stderr,"node %d: packetApiReceiveAck: got a PACKET_API_RECEIVEACK packet\n",us->addr & 0xFF);
#endif

	pd=dataPacketAckUnmarshal(p);

	if (pd==NULL)     /* unmarshal failed...   */
	{
		fprintf(stderr,"packetApiReceiveAck: unmarshal dataPacketAckUnmarshal failed\n");
		return;
	}

	for(i=0;i<pd->destinationCount;i++)              /* have all the destinations acked?   */
		if (pd->destinationAck[i]!=DATA_ACK)
			allack=0;

	messageInfoSearch(us->packetApi,pd->id,&mi,&as);    /* lookup MI and client (ApiSession) by the dataID  */

	if (mi==NULL)
	{
#ifdef DEBUG_APPROUTING
		fprintf(stderr,"ackPacket: got an ACK for a missing client.  dataid= %u\n",pd->id);
#endif
		free(pd);
		return;
	}

	if (as==NULL)
	{
		fprintf(stderr,"ackPacket: got an ACK for a client which has disconnected.\n");
		messageInfoDestroy(mi);          /* this will free mi, AND the orginal AC which was read when the message sent was read from the client */
		free(pd);
		return;
	}

	as->name->messagesUnacked--;
	as->name->messagesAcked++;

	if (allack)
		ac=apiCommandMalloc(APICOMMAND_MESSAGE_ACK,0);
	else
		ac=apiCommandMalloc(APICOMMAND_MESSAGE_NAK,0);

#ifdef DEBUG_APPROUTING
	fprintf(stderr,"node %d: got an ACK, id= %d tag= %d\n",us->addr & 0xFF,pd->id,mi->tag);
#endif

	ac->len=0;
	ac->tag=mi->tag;

	messageInfoRemove(as,mi);

	apiWrite(as,ac);    /* takes ownership of ac, will be freed in the select loop when it is actually written.  may delete as */

	messageInfoDestroy(mi);          /* this will free mi, AND the orginal AC which was read when the message sent was read from the client */
	free(pd);
}


/* Given an outgoing packet, or incoming packet-to-be-routed, either route it, or transmit it
 * This implements the CommunicationsDestinationType addressing modes.
 *
 * Incoming packets will not have MessageInfos, or ApiSessions  (and thus can not be acked)
 */
static void apiApplicationRoute(manetNode *us, packet *p, MessageInfo *mi, ApiSession *as)
{
	unsigned int id;
	PacketApi pa;

	if (packetApiHeaderUnmarshal(p,&pa) != 0)
	{
		fprintf(stderr, "node %s: %s: ERROR: packetApiHeaderUnmarshal failed (p->len= %d)\n",
			manetAddr2Str(us->addr), __func__, p->len);
		return;
	}

	switch(pa.desttype)
	{
		case COMMUNICATIONSDESTINATION_DIRECT:
			assert(mi!=NULL);
			if (p->dst==us->addr)   /* if dst addr is us, just rereceive it (and no ACK from the data module) */ 
			{
				ApiCommand *ac;
				packetReReceive(us,p);

				/* send API ACK to sending client (packet was delivered)  */

				ac=apiCommandMalloc(APICOMMAND_MESSAGE_ACK,0);
				ac->len=0;
				ac->tag=mi->tag;
				if (as)
				{
					as->name->messagesAcked++;
					as->name->messagesUnacked--;
				}
				apiWrite(as,ac);		/* which takes ownership of ac, and will free when it writes.  may delete as */
				messageInfoDestroy(mi);
			}
			else
			{
				dataSend(us, p, DATA_ROUTE_AMBIENT | mi->routeFlags, DATA_ACK_ENDTOEND,&id);
				mi->dataId=id;	/* the data module assigns another tag.  We record it in the MI so we can recogize the ACK message from the data module  */
#ifdef DEBUG_APPROUTING
				fprintf(stderr,
					"node %s: apiApplicationRoute: "
					"sending an API packet type= 0x%x id= %d tag= %d len= %d routeflags= 0x%x",
					manetAddr2Str(us->addr),
					pa.type,
					mi->dataId,
					mi->tag,
					p->len,
					mi->routeFlags);
				fprintf(stderr, "to %s DIRECT as= %p\n", manetAddr2Str(p->dst),as);
#endif
				messageInfoInsert(as,mi);	/* and we will wait for an ACK for it...  */
			}
		break;
		case COMMUNICATIONSDESTINATION_DIRECTBACKUP:			/* unicast route, using the backup routing algorithm */
			assert(mi!=NULL);
			dataSend(us, p, DATA_ROUTE_FLOOD | mi->routeFlags, DATA_ACK_ENDTOEND,&id);
			mi->dataId=id;
			messageInfoInsert(as,mi);	/* and we will wait for an ACK for it...  */
		break;
		case COMMUNICATIONSDESTINATION_MULTICAST:
#ifdef DEBUG_APPROUTING
			fprintf(stderr,"node %u: apiApplicationRoute: sending MULTICAST to %s (NYI -- does a broadcast for now)\n",
                                us->addr & 0xFF, manetAddr2Str(p->dst));
#endif
			/* fall through... */
		case COMMUNICATIONSDESTINATION_BROADCAST:
		{
			ApiCommand *ac;

			assert(mi!=NULL);
#ifdef MODULE_FLOOD
			floodSend(us, p);
#else
#warning The flood module is disabled
			assert(0);
#endif

			/* send API ACK to sending client (packet might be delivered...  but broadcast isn't acked so we don't actually know if its delivered, so the API returns NAK  */

			ac=apiCommandMalloc(APICOMMAND_MESSAGE_NAK,0);
			ac->len=0;
			ac->tag=mi->tag;
			apiWrite(as,ac);		/* which takes ownership of ac, and will free when it writes.  may delete as */

			messageInfoDestroy(mi);   /* not waiting for ACK, so chuck the MI  */
		}
		break;

		case COMMUNICATIONSDESTINATION_PARENTSOF:
#ifdef DEBUG_APPROUTING
			fprintf(stderr,"Sending to PARENTSOF %d\n",p->dst & 0xFF);
#endif
			if (p->dst==us->addr)   /* this packet wants our parents...   */
			{
				if (us->rootflag)
				{
					int rc;
#ifdef DEBUG_APPROUTING
					fprintf(stderr,"sending packet to parentsof, but we are root so that is us\n");
#endif
					p->dst=us->addr;
					p->hopcount=0;
					p->type=PACKET_API_RECEIVE;
					pa.desttype=COMMUNICATIONSDESTINATION_DIRECT;
					rc=packetApiHeaderMarshal(p,&pa);
					assert(rc==0); // we unmarshaled successfully, so there must be room
					apiApplicationRoute(us,p,mi,as);     /* IE: fall right into the DIRECT case above.  */
				}
				else if (us->clusterhead)   /* XXX: fixme: multiple parent support, using neighbor list, not CH pointer */
				{
					int rc;
#ifdef DEBUG_APPROUTING
					fprintf(stderr,"sending packet to parentsof\n");
#endif
					p->dst=us->clusterhead->addr;
					p->hopcount=0;
					p->type=PACKET_API_RECEIVE;
					pa.desttype=COMMUNICATIONSDESTINATION_DIRECT;
					rc=packetApiHeaderMarshal(p,&pa);
					assert(rc==0); // we unmarshaled successfully, so there must be room
					
					apiApplicationRoute(us,p,mi,as);     /* IE: fall right into the DIRECT case above.  */
				}
				else
				{	/* we have no coordinator...   */
					if (as)		/* if we are local, we can send a NAK  */
					{
						ApiCommand *ac;

						/* message is undeliverable, send API NAK message */
						ac=apiCommandMalloc(APICOMMAND_MESSAGE_NAK,0);
						ac->tag=mi->tag;
						if (as->name)
							as->name->messagesUnacked--;
						apiWrite(as,ac);		/* which takes ownership of ac, and will free when it writes.  may delete as */
						messageInfoDestroy(mi);
					}
				}
			}
			else
			{		/* unicast to the node whose parents we want...  XXX: how to do reliability?  */
#ifdef DEBUG_APPROUTING
				fprintf(stderr,"sending packet over to %d, to be routed   src= %d us= %d\n",p->dst & 0xFF,p->src & 0xFF, us->addr & 0xFF);
#endif
				p->type=PACKET_API_ROUTE;
				dataSend(us, p, DATA_ROUTE_AMBIENT | mi->routeFlags, DATA_ACK_NONE,&id);
				messageInfoDestroy(mi);
			}
		break;
		case COMMUNICATIONSDESTINATION_CHILDRENOF:
#ifdef DEBUG_APPROUTING
			fprintf(stderr,"Sending to CHILDRENOF %d\n",p->dst & 0xFF);
#endif
			if (p->dst!=us->addr)
			{
				/* unicast to p->dst.  And then do the else condition below when we get there */

#ifdef DEBUG_APPROUTING
				fprintf(stderr,"sending packet over to %d, to be routed\n",p->dst & 0xFF);
#endif
				p->type=PACKET_API_ROUTE;
				dataSend(us, p, DATA_ROUTE_AMBIENT | mi->routeFlags, DATA_ACK_NONE,&id);
				messageInfoDestroy(mi);
			}
			else
			{
				ManetAddr dstlist[512];    /* XXX: this 512 is a hard upper bound on number of neighbors  */
				unsigned int dstcount=0;
				CommunicationsNeighbor *cn;
				int rc;

				for(cn=us->packetApi->neighborList;cn;cn=cn->next)
				{
					if (cn->type&COMMUNICATIONSNEIGHBOR_CHILD)
					{
						if (dstcount <(sizeof(dstlist)/sizeof(dstlist[0])))
							dstlist[dstcount++]=cn->addr;
						else
							fprintf(stderr,"Sending to CHILDRENOF exceeded size of dstlist!\n");
					}
				}

				pa.desttype=COMMUNICATIONSDESTINATION_DIRECT;
				rc=packetApiHeaderMarshal(p,&pa);
				assert(rc==0); // we unmarshaled successfully, so there must be room
				dataSendMulti(us, p, dstlist,dstcount, DATA_ROUTE_AMBIENT | mi->routeFlags, DATA_ACK_ENDTOEND,&id);
				if (mi!=NULL)
				{
					mi->dataId=id;	/* the data module assigns another tag.  We record it in the MI  */
#ifdef DEBUG_APPROUTING
					fprintf(stderr,
						"node %s: apiApplicationRoute: "
						"sending an API packet id= %d tag= %d len= %d ",
						manetAddr2Str(us->addr),
						mi->dataId,
						mi->tag,
						p->len);
					fprintf(stderr, "to %s CHILDRENOF\n", manetAddr2Str(p->dst));
#endif
					messageInfoInsert(as,mi);	/* and we will wait for an ACK for it...  */
				}
				else
					fprintf(stderr,"XXX: mi was NULL, we've lost the ack!\n");
			}
		break;
		case COMMUNICATIONSDESTINATION_NEARESTCOORD:
			/* if we are a coordinator, send to local node.  Otherwise send to parent */
			if (
				(us->packetApi->currentPositionState[COORDINATOR_ROOT]==IDSPOSITION_ACTIVE) ||
				(us->packetApi->currentPositionState[COORDINATOR_REGIONAL]==IDSPOSITION_ACTIVE) ||
				(us->packetApi->currentPositionState[COORDINATOR_NEIGHBORHOOD]==IDSPOSITION_ACTIVE)
				)
			{
				int rc;
				p->dst=us->addr;
				p->hopcount=0;
				p->type=PACKET_API_RECEIVE;
				pa.desttype=COMMUNICATIONSDESTINATION_DIRECT;
				rc=packetApiHeaderMarshal(p,&pa);
				assert(rc==0); // we unmarshaled successfully, so there must be room
				
				apiApplicationRoute(us,p,mi,as);     /* IE: fall right into the DIRECT case above.  */
			}
			else
			{
				int rc;
				p->dst=us->addr;
				p->hopcount=0;
				p->type=PACKET_API_RECEIVE;
				pa.desttype=COMMUNICATIONSDESTINATION_PARENTSOF;
				rc=packetApiHeaderMarshal(p,&pa);
				assert(rc==0); // we unmarshaled successfully, so there must be room
				
				apiApplicationRoute(us,p,mi,as);     /* IE: fall right into the PARENTSOF case above.  Which may then fall into DIRECT, depending on if this node is root or not...  */
			}

		break;
		case COMMUNICATIONSDESTINATION_NEIGHBORSOF:
		case COMMUNICATIONSDESTINATION_RECURSIVECHILDRENOF:
		case COMMUNICATIONSDESTINATION_RECURSIVEPARENTSOF:
			fprintf(stderr,"Unimplemented destination type!  Cross your fingers, memory is leaking, and your message has been dropped (sorry).\n");
		break;
	}
}


/*********************************************************************************************
 *
 * Calls to handle the packets in flight lists
 *
 * Each client has its own packets in flight list.  When a packet is transmitted it is placed
 * on the list of the client which sent it.
 * When a packet arrives, /all/ the inflight lists are searched, using the MessageId field as 
 * a key, to find which client to send the ACK packet to.  
 * Timeouts are handled in the data module (simulator land) which gives ACK/NAK packets to this
 * code.  Thus when a packet is dropped, the data module gives this code a NAK, which forwards
 * it to the client involved.
 *
 *
 */

/* insert a message into an api's packets inflight list
 */
static void messageInfoInsert(ApiSession *as, MessageInfo *mi)
{
	if (as)
	{
		mi->next=as->inflight;
		as->inflight=mi;
	}
	else
	{
		messageInfoDestroy(mi);   /* XXX: we don't have any inflight lists not associated with an existing API session.  So we're going to chuck it, and it will look like we got an ack for a packet we didn't send when the ACK arrives.  */
	}
}

/* Search all the inflight list for message ID id.
 * The messageinfo struct is returned in *m
 * the client struct is returnedin *a
 */
static void messageInfoSearch(PacketApiNodeState *st, MessageID dataId, MessageInfo **m, ApiSession **a)
{
	ApiSession *as;
	MessageInfo *mi;

	*m=NULL;
	*a=NULL;

	for(as=st->client;as!=NULL;as=as->next)
	{
		for(mi=as->inflight;mi!=NULL;mi=mi->next)
			if (mi->dataId==dataId)
			{
				*m=mi;
				*a=as;
				return;
			}
	}
	return;
}

/* removes mi from the inflight list of as
*/
static void messageInfoRemove(ApiSession *as,MessageInfo *mi)
{
	MessageInfo *p,*q;

	q=NULL;
	p=as->inflight;

	while((p!=mi) && (p!=NULL))
	{
		q=p;
		p=p->next;
	}

	if (p)
	{
		if (q)
			q->next=q->next->next;
		else
			as->inflight=as->inflight->next;
	}
}

/*************************************************************************************************************
 *
 * Calls to handle IDSPosition changes
 */


/* When we get a new position, we report that to every client which has
 * registered elegibility for (or interest in) that position.  This 
 * function takes a postion and status, walks the list of clients, and
 * schedules ACs to be transmitted appropriately.
 */

static void idsPositionSend(manetNode *us, IDSPositionType position,IDSPositionStatus stat)
{
	ApiSession *as,*nxt;
	ApiCommand *ac;
	IDSPosition *rn;

	as=us->packetApi->client;
	while(as)
	{
		nxt=as->next;
		if ((rn=idsPositionSearch(&(as->positionList),position)))
		{
			if (rn->status!=stat)
			{
				fprintf(stderr, "Changing %s from %s to %s on client %d\n",
					idsPosition2Str(position),
					idsPositionStatus2Str(rn->status),
					idsPositionStatus2Str(stat),
					as->fd);

				rn->status=stat;
				ac=idsPositionMarshal(rn->position,rn->status);
				apiWrite(as,ac);    /* may delete as   */
			}
		}
		as=nxt;
	}
}

/* This is called once per select loop to see if we have been moved into, or removed
 * from a postion.  If we have, idsPositionSend() is called to report that to the clients.
 */
static void idsPositionCheck(manetNode *us,void *data)
{
	IDSPositionStatus stat;
	CommunicationsNeighbor *cn;

	timerSet(us,idsPositionCheck,1000,NULL);

	/*
	 * Check Root
	*/
	if ((us->rootflag))
	{
		/* we're the root!  */
		stat=IDSPOSITION_ACTIVE;
	}
	else
		stat=IDSPOSITION_INACTIVE;

#ifdef DEBUG_PACKETAPI
	if (us->packetApi->currentPositionState[COORDINATOR_ROOT]!=stat)
	{
		elog("idsPositionCheck: COORDINATOR_ROOT %s\n",idsPositionStatus2Str(stat));
	}
#endif
	us->packetApi->currentPositionState[COORDINATOR_ROOT]=stat;
	idsPositionSend(us,COORDINATOR_ROOT,stat);


	/*
	 * Check Root Group
         */
	if ((us->rootgroupflag))
	{
		/* we're an active member of the root group */
		stat=IDSPOSITION_ACTIVE;
	}
	else
		stat=IDSPOSITION_INACTIVE;

#ifdef DEBUG_PACKETAPI
	if (us->packetApi->currentPositionState[COORDINATOR_ROOTGROUP]!=stat)
	{
		elog("idsPositionCheck: COORDINATOR_ROOTGROUP %s\n",idsPositionStatus2Str(stat));
	}
#endif
	us->packetApi->currentPositionState[COORDINATOR_ROOTGROUP]=stat;
	idsPositionSend(us,COORDINATOR_ROOTGROUP,stat);


	/* If we have child nodes, then we are a NEIGHBORHOOD coordinator
	 */
	stat=IDSPOSITION_INACTIVE;
	for(cn=us->packetApi->neighborList;cn;cn=cn->next)
	{
		if (cn->type&COMMUNICATIONSNEIGHBOR_CHILD)
			stat=IDSPOSITION_ACTIVE;
	}
#ifdef DEBUG_PACKETAPI
	if (us->packetApi->currentPositionState[COORDINATOR_NEIGHBORHOOD]!=stat)
	{
		elog("idsPositionCheck: COORDINATOR_NEIGHBORHOOD %s\n",idsPositionStatus2Str(stat));
	}
#endif
	us->packetApi->currentPositionState[COORDINATOR_NEIGHBORHOOD]=stat;
	idsPositionSend(us,COORDINATOR_NEIGHBORHOOD,stat);


	/* If we have child nodes which also have child nodes then we are
	 * a REGIONAL coordinator.  We can't observe the grandchild nodes 
	 * directly, BUT its the only way our level would be greater than 1.
	 */

	if ((stat==IDSPOSITION_ACTIVE) && (us->level>1))    /* if we are a COORDINATOR_NEIGHBORHOOD (IE: we have kids) and our level >1, then we are a REGIONAL coordinator too */
		stat=IDSPOSITION_ACTIVE;
	else
		stat=IDSPOSITION_INACTIVE;

#ifdef DEBUG_PACKETAPI
	if (us->packetApi->currentPositionState[COORDINATOR_REGIONAL]!=stat)
	{
		elog("idsPositionCheck: COORDINATOR_REGIONAL %s\n",idsPositionStatus2Str(stat));
	}
#endif
	us->packetApi->currentPositionState[COORDINATOR_REGIONAL]=stat;
	idsPositionSend(us,COORDINATOR_REGIONAL,stat);
}


/* Looks at the neighbor list mantained by the clustering algorithm, and makes a 
 * list with a single entry per node.  (There are multiple entries, one per level, in
 * the clustering alg's neighbor list.)
 */
static CommunicationsNeighbor *communicationsNeighborMakeCurrent(manetNode *us)
{
	CommunicationsNeighbor *cn;
	CommunicationsNeighbor *tmplist=NULL;
	neighbor *n;

	/* Make list of symmetric neighbors which are either distance==1, or a child  */

        for(n=us->neighborlist;n;n=n->next)
	{
		int type;
		cn=communicationsIntNeighborSearch(&tmplist,n->addr);
		type=cn?cn->type:0;
		
		if ((n->flags & NEIGHBOR_ROOT) && (n->flags & NEIGHBOR_HEARS))
		{
			if (cn==NULL)
			{
				cn=communicationsIntNeighborMalloc(n->addr,COMMUNICATIONSNEIGHBOR_UNKNOWN,n->hopcount);
				communicationsIntNeighborUpdate(&tmplist,cn);    /* takes ownership of cn  */
			}

			if (n->flags & NEIGHBOR_ROOT)
				type|=COMMUNICATIONSNEIGHBOR_ROOT;
		}

		if ((n->flags & NEIGHBOR_HEARS))	/* if not symmetric, ignore it */
		{

			if ((n->flags & NEIGHBOR_CHILD) || (n->hopcount==1))
			{
				if (cn==NULL)
				{
					cn=communicationsIntNeighborMalloc(n->addr,COMMUNICATIONSNEIGHBOR_UNKNOWN,n->hopcount);
					communicationsIntNeighborUpdate(&tmplist,cn);    /* takes ownership of cn  */
				}

				if (n->flags & NEIGHBOR_CHILD)
					type|=COMMUNICATIONSNEIGHBOR_CHILD;

			}
		}
		if (cn)     /* if its on the list...   */
		{
			cn->type=type;
			if (n->hopcount < cn->distance)
				cn->distance=n->hopcount;
		}
	}

	/* add clusterhead to the list  */
	if (us->clusterhead)
	{
		cn=communicationsIntNeighborSearch(&tmplist,us->clusterhead->addr);
		if (cn==NULL)
		{
			cn=communicationsIntNeighborMalloc(us->clusterhead->addr,COMMUNICATIONSNEIGHBOR_UNKNOWN,us->clusterhead->hopcount);
			communicationsIntNeighborUpdate(&tmplist,cn);    /* takes ownership of cn  */
		}
		if (us->clusterhead->hopcount < cn->distance)
			cn->distance=us->clusterhead->hopcount;

		cn->type|=COMMUNICATIONSNEIGHBOR_PARENT;
	}

#if 0
	fprintf(stderr,"current neighbor list: ");
	for(cn=tmplist;cn;cn=cn->next)
		fprintf(stderr,"%d ",cn->addr & 0xFF);
	fprintf(stderr,"\n");
#endif		

	return tmplist;
}

/* Free a linked list of CommunicationsNeighbor structs
 */
static void communicationsIntNeighborListFree(CommunicationsNeighbor *tmplist)
{
	CommunicationsNeighbor *cn;
	while(tmplist)
	{
		cn=tmplist;
		tmplist=tmplist->next;
		free(cn);
	}
}

/* Walk the node list, and compare it to our list of existing neighbors.
 * There is some gore due to the coordinator maybe not being a single hop neighbor
 */
static void neighborCheck(manetNode *us, void *data)
{
	CommunicationsNeighbor *cn,*ncn;
	CommunicationsNeighbor *tmplist=NULL;
	CommunicationsNeighborState state;
	ApiCommand *aclist=NULL,*ac=NULL;

	timerSet(us, neighborCheck,1000, NULL);   /* check once/second, instead of once/event.  */

	tmplist=communicationsNeighborMakeCurrent(us);   /* make list of current neighbors */

        /* XXX consider keeping neighborList sorted so this isn't n^2?
         * -dkindred 2007-06-01 */

	/* walk old list, and search new list, looking for departures  */
	for(cn=us->packetApi->neighborList;cn;cn=cn->next)
	{
		ncn=communicationsIntNeighborSearch(&tmplist,cn->addr);

		if (ncn==NULL)   /* if cn is on the old list, but not on the new list...  */
		{
							/* send departure.  */
			state=cn->state;
			cn->state=COMMUNICATIONSNEIGHBOR_DEPARTING;

			if (cn->distance==1)
				cn->type|=COMMUNICATIONSNEIGHBOR_WASONEHOP;

			ac=communicationsNeighborMarshal(cn);
			ac->next=aclist;
			aclist=ac;

			cn->type&=~COMMUNICATIONSNEIGHBOR_WASONEHOP;

			cn->state=state;   /* save state, because we need it to be correct in next search...  */
		}
	}

	/* walk new list, and search old list, looking for arrivals/deltas */
	for(cn=tmplist;cn;cn=cn->next)
	{
		ncn=communicationsIntNeighborSearch(&(us->packetApi->neighborList),cn->addr);

		if ((ncn==NULL) || 								    	/* if its not on the old list  */
			((ncn!=NULL) && ((ncn->type!=cn->type) || (ncn->distance!=cn->distance))))	/* or if it is, but its different...  */
		{
									/* then send an arrival.   */
			state=cn->state;
			cn->state=(ncn==NULL)?COMMUNICATIONSNEIGHBOR_ARRIVING:COMMUNICATIONSNEIGHBOR_UPDATING;

			if ((ncn) &&(ncn->distance==1))    /* if on old list, and old distance was 1...  */
				cn->type|=COMMUNICATIONSNEIGHBOR_WASONEHOP;

			ac=communicationsNeighborMarshal(cn);
			ac->next=aclist;
			aclist=ac;

			cn->type&=~COMMUNICATIONSNEIGHBOR_WASONEHOP;

			cn->state=state;   /* save state, because we need it to be correct in next search...  */
		}
			
	}

	communicationsIntNeighborListFree(us->packetApi->neighborList);   /* free old list, and replace with new one.  */
	us->packetApi->neighborList=tmplist;

#if 0
	for(ac=aclist;ac;ac=ac->next)
	{
		cn=communicationsNeighborUnmarshal(ac);
		fprintf(stderr,"node %d: neighbor %d distance %d type %d %s\n",us->addr & 0xFF, cn->addr & 0xFF, cn->distance,cn->type,(cn->state==COMMUNICATIONSNEIGHBOR_DEPARTING)?"departing":"arriving");
		free(cn);
	}
#endif

	if (aclist)
		apiWriteAll(us->packetApi->client,aclist);
}

/* Returns a list of ApiCommands containing the current neighbor list
 *
 * Intended for initing a recently connected detector, so it has the current neighbor list.
 */
static ApiCommand *neighborList(PacketApiNodeState *st)
{
	ApiCommand *ac,*aclist;
	CommunicationsNeighbor *cn;

	aclist=NULL;

	for(cn=st->neighborList;cn;cn=cn->next)
	{
		ac=communicationsNeighborMarshal(cn);

		ac->next=aclist;
		aclist=ac;
	}

	return aclist;
}

/* Returns a list of ApiCommands containing the current posweight list
 *
 * Intended for initing a recently connected detector, so it has the current posweight list.
 */
static ApiCommand *positionWeightList(PacketApiNodeState *st)
{
	ApiCommand *ac,*aclist;
	CommunicationsPositionWeight *pw;

	aclist=NULL;

	for(pw=st->weightList;pw;pw=pw->next)
	{
		ac=communicationsPositionWeightMarshal(pw);

		ac->next=aclist;
		aclist=ac;
	}

	return aclist;
}

static void statusStartCheck(manetNode *us)
{
	if (us->packetApi->status.period>0)
		if (!us->packetApi->statusRunning)
		{
			fprintf(stderr,"node %d: status: starting callback period= %d\n",us->addr & 0xFF,us->packetApi->status.period);
			timerSet(us,statusTimeout, us->packetApi->status.period,NULL);
			us->packetApi->statusRunning=1;
		}

}
/* Called every delay milliseconds
 */
static void statusTimeout(manetNode *us, void *data)
{
	if (us->packetApi->status.period>0)
	{
//		fprintf(stderr,"node %d: status: rescheduling callback period= %d\n",us->addr & 0xFF,us->packetApi->status.period);
		timerSet(us,statusTimeout, us->packetApi->status.period,NULL);
	}
	else
	{
//		fprintf(stderr,"node %d: status: stop callback\n",us->addr & 0xFF);
		us->packetApi->statusRunning=0;
	}

	statusCheck(us,1);
}

/* called by statusTimeout, AND at the end of the select loop.
 *
 * we want to send a status message every time statusTimeout is called.  So it sets the
 * override argument.
 *
 * We will not set the override argument at the end of the select loop, so then we only
 * want to send a message if the state has changed.
 */
static void statusCheck(manetNode *us, int override)
{
	ApiStatus status;
	ApiSession *as,*nxt;
	ApiCommand *ac;
	int changeflag=0;

	status.period=us->packetApi->status.period;
	status.level=us->level;
	status.rootflag=us->rootflag;
	status.packetList=us->packetApi->status.packetList;
	status.numtypes=us->packetApi->status.numtypes;
	status.timestamp=us->manet->curtime;

	if ((us->packetApi->status.period!=status.period) || (us->packetApi->status.level!=status.level) || (us->packetApi->status.rootflag!=status.rootflag))
		changeflag=1;

	if ((!changeflag) && (!override))
		return;

	changeflag=changeflag || override;

	ac=apiStatusMarshal(&status);

	as=us->packetApi->client;
	while(as!=NULL)
	{
		nxt=as->next;
		if ((as->statusEnable==1) ||
			((as->statusEnable==2) && (changeflag)))
			{
// 				fprintf(stderr,"node %d: status: sending to fd %d\n",st->themanet->nlist[0].addr & 0xFF, as->fd);
				as->statusEnable=2;
				apiWrite(as,apiCommandCopy(ac));     /* may delete as   */
			}
		as=nxt;
	}
	memcpy(&us->packetApi->status,&status,sizeof(status));
	apiCommandFree(ac);
}


static void packetApiIntSend(manetNode *us, MessageInfo *mi, ApiSession *as)
{
	char typestring[32];

	mi->demonId=us->packetApi->nextDemonId++;
	mi->originApi=as;     // as may be NULL, if simulatorland is sending
	mi->chainApi=NULL;

	if (us->packetApi->failoverList)
	{
		sprintf(typestring,",%x,",mi->type);
		if (strstr(us->packetApi->failoverList,typestring))
			mi->routeFlags=DATA_ROUTE_NOFAILOVER;
	}

	//                      fprintf(stderr,"got a message from client %d  len= %d demonid= %d tag= %d dst= %d dstype= %d routeFlags= 0x%x\n",as->fd,mi->payloadLen,mi->demonId,mi->tag,mi->dest.addr & 0xFF,mi->dest.type,mi->routeFlags);

	if (as!=NULL)
	{
		as->name->messagesSent++;
		as->name->messagesUnacked++;
	}
	messageNextChain(us, APICOMMAND_MESSAGE_SEND, mi);
}


/* simulatorland call to send a hierarchy message
 * (how to subscribe/receive?  ignoring that problem for now...)
 */
void packetApiSend(manetNode *us, ManetAddr dest, int ttl,  PacketApi *pa)
{
	MessageInfo *mi;

// Want to copy the payload onto the end of the mi?  Unmarshaled mi's don't do that...
	mi=(MessageInfo*)malloc(sizeof(*mi));

	mi->next=NULL;
	mi->cs=NULL;
	mi->originApi=NULL;
	mi->chainApi=NULL;
	mi->dataId=0;
	mi->statusCallback=NULL;
	mi->statusData=NULL;
	mi->routeFlags=0;

	mi->origin=us->addr;
	mi->dest.addr=dest;
	mi->dest.type=pa->desttype;
	mi->dest.ttl=ttl;
	mi->type=pa->type;
	mi->tag=0;
	mi->demonId=0;
	mi->routeFlags=0;
	mi->payload=pa->payload;
	mi->payloadLen=pa->payloadLen;
	mi->payloadPtr=pa->payload;

	packetApiIntSend(us, mi, NULL);    /* we have no apistate for simulator land, so it will act as if the client instantly disconnected.  */
}




/* Create a packet (simulator land) and marshal a PacketAPI structure (API land) into it
 */
/* NOTE: bounds checking against packet len done */
packet *packetApiMarshal(manetNode *us, const PacketApi *pa,int doCompression)
{
	packet *p=NULL;
	int len;
	int rc;
	z_stream compstream;
	CommunicationsMessageType *cmt;

	len=(int)(pa->payloadLen*1.3)+32+ PACKETAPI_HEADERLEN;  /* This is an upperbound on the size of the compressed data */
	p=packetMalloc(us,len);

	rc=packetApiHeaderMarshal(p,pa);
	assert(rc==0);	// can't fail - we just allocated PACKETAPI_HEADERLEN + ...

/* This pointless loop is here in an attempt to determine where an uninited value is referenced by zlib, when the packet is compressed
 */
#if 0
	int i;
	for(i=0;i<pa->payloadLen;i++)
		if (pa->payload[i]==0)
			fprintf(stderr,"0");
#endif

	p->dst=((pa->origdest.addr==NODE_LOCAL) || (pa->origdest.addr==0))?(us->addr):pa->origdest.addr;  

	p->type=PACKET_API_RECEIVE;
	p->ttl=pa->origdest.ttl;

#warning how to avoid compression when sending to local?
	/* Compress the payload for transmission on the wire...  */
	if (doCompression)
	{
		compstream.zalloc=(alloc_func)NULL;                     /* compress the payload...  */
		compstream.zfree=(ZLIB_FREE_FUNC)NULL;
		compstream.opaque=(voidpf)NULL;
		deflateInit(&compstream,Z_DEFAULT_COMPRESSION);
		compstream.next_in=pa->payload;
		compstream.next_out=(Bytef*)((char*)p->data+PACKETAPI_HEADERLEN);
		compstream.avail_in=pa->payloadLen;
		compstream.avail_out=len-PACKETAPI_HEADERLEN;
		cmt=communicationsMessageTypeSearch(pa->type);
		if (cmt)
			deflateSetDictionary(&compstream,(const Bytef*)cmt->dictionary,cmt->dictionarylen);
		rc=deflate(&compstream,Z_FINISH);
		assert(rc==Z_STREAM_END);
		rc=deflateEnd(&compstream);
		assert(rc==Z_OK);

		p->len=PACKETAPI_HEADERLEN+compstream.total_out;     /* rewrite packet length...  so as to ignore unused buffer on end  */
		assert(p->len < len);
	}
	else
	{
		assert(PACKETAPI_HEADERLEN + pa->payloadLen <= p->len);
		memcpy((unsigned char *)p->data+PACKETAPI_HEADERLEN,pa->payload,pa->payloadLen);
		p->len=pa->payloadLen+PACKETAPI_HEADERLEN;
	}
	return p;
}


int packetApiHeaderMarshal(packet *p, const PacketApi *pa)
{
	unsigned char *hp;

	hp=(unsigned char *)p->data;
	if (p->len < PACKETAPI_HEADERLEN)
	{
		return -1;	/* not enough space */
	}
	MARSHALLONG(hp,pa->type);
	MARSHALBYTE(hp,pa->desttype);
	MARSHALLONG(hp,pa->origdest.addr);
	MARSHALBYTE(hp,pa->origdest.type);
	MARSHALBYTE(hp,pa->origdest.ttl);
	return 0;
}

/* This unmarshals only the PacketApi header from a packet, and ignores
 * the payload fields.
 * It exists because the routing code modifies the PacketApi header as it
 * goes through.
 */
int packetApiHeaderUnmarshal(const packet *p, PacketApi *pa)
{
	unsigned char *hp;
	int i;

	hp=(unsigned char *)p->data;
	if (p->len < PACKETAPI_HEADERLEN)
	{
		return -1;	/* too short */
	}
	UNMARSHALLONG(hp,pa->type);
	UNMARSHALBYTE(hp,i);
	pa->desttype=(CommunicationsDestinationType)i;
	UNMARSHALLONG(hp,pa->origdest.addr);
	UNMARSHALBYTE(hp,i);
	pa->origdest.type=(CommunicationsDestinationType)i;
	UNMARSHALBYTE(hp,pa->origdest.ttl);
	return 0;
}

/* NOTE: bounds checking against packet len done */
PacketApi *packetApiUnmarshal(const packet *p, int doCompression)
{
	z_stream compstream;
	int decompsize;
	int rc;
	int dictflag=0;
	CommunicationsMessageType *cmt;
	PacketApi *pa = NULL;

	if (p->type!=PACKET_API_RECEIVE)
		goto fail;

	decompsize=p->len+sizeof(*pa);
	pa=(PacketApi*)malloc(decompsize);

	if (packetApiHeaderUnmarshal(p,pa) != 0)
	{
		goto fail;
	}

	pa->payload=((unsigned char *)pa) +sizeof(*pa);

	if (doCompression)
	{
		compstream.zalloc=(alloc_func)NULL;                     /* decompress the payload */
		compstream.zfree=(ZLIB_FREE_FUNC)NULL;
		compstream.opaque=(voidpf)NULL;
		rc=inflateInit(&compstream);

		compstream.next_in=((unsigned char *)p->data) + PACKETAPI_HEADERLEN;
		compstream.avail_in=p->len - PACKETAPI_HEADERLEN;

		compstream.next_out=pa->payload;
		compstream.avail_out=decompsize-sizeof(*pa);

		while((rc=inflate(&compstream,0))!=Z_STREAM_END)
		{
			if ((rc==Z_NEED_DICT) && (!dictflag))
			{
				cmt=communicationsMessageTypeSearch(pa->type);
				if (cmt)
					inflateSetDictionary(&compstream,(const Bytef*)cmt->dictionary,cmt->dictionarylen);
				dictflag=1;
				continue;
			}
			if (rc!=Z_OK)
				break;

			int oldlen=decompsize;
			int oldused=compstream.next_out - pa->payload;

			decompsize*=2;
			pa=(PacketApi*)realloc(pa,decompsize);
			pa->payload=((unsigned char *)pa)+sizeof(*pa);
			compstream.next_out=pa->payload + oldused;
			compstream.avail_out+=decompsize-oldlen;
		}

		if (rc!=Z_STREAM_END)
		{
			fprintf(stderr,"packetApiUnmarshal: zlib failed...  dropping message!  rc= %d\n",rc);
			inflateEnd(&compstream);
			goto fail;
		}

		rc=inflateEnd(&compstream);
		if (rc!=Z_OK)
		{
			goto fail;
		}
		pa->payloadLen=compstream.total_out;
	}
	else
	{
		memcpy(pa->payload,(unsigned char*)p->data + PACKETAPI_HEADERLEN,p->len - PACKETAPI_HEADERLEN);
		pa->payloadLen=p->len - PACKETAPI_HEADERLEN;
	}
	return pa;
 fail:
	if (pa) free(pa);
	return NULL;
}


void watcherNodeColor(manetNode *us, ManetAddr node, unsigned char *color)
{
        unsigned char *msg = (unsigned char *)malloc(1024);   /*  "sufficient"  */
        unsigned char *pos=msg;
	PacketApi pa;

        if ((node==NODE_LOCAL) || (node==0))
                node=us->addr;

        pos=watcherColorMarshal(msg, node, color);

        pa.type=IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR;
        pa.desttype=COMMUNICATIONSDESTINATION_DIRECT;
        pa.payload=msg;
        pa.payloadLen=pos-msg;

        packetApiSend(us, NODE_LOCAL, 1, &pa);   /* which takes owner ship of the payload...  */
}
 
void packetApiMessageHandlerSet(
        manetNode *us,
        CommunicationsMessageDirection direction,
        unsigned int priority,
        CommunicationsMessageAccess access,
        MessageType messageType,
        MessageHandler messageHandlerFunc,
        void *messageHandlerData)
{
	MessageHandlerRequest mhr;
	MessageTypeNode *mtn;

	mhr.request=messageHandlerFunc?APICOMMAND_MESSAGETYPE_REQUEST:APICOMMAND_MESSAGETYPE_REMOVE;
	mhr.direction=direction;
	mhr.priority=priority;
	mhr.access=access;
	mhr.type=messageType;

	mtn=messageHandlerRequestHandle(us,NULL,&mhr);
	mtn->handler=messageHandlerFunc;
	mtn->handlerData=messageHandlerData;
}

CommunicationsPositionWeight *packetApiPositionWeightSearchList(manetNode *us, CommunicationsPositionWeight *key)
{
	return communicationsPositionWeightSearchList(us->packetApi->weightList,key);
}

void packetApiPositionWeightLoad(manetNode *us, const char *filename)
{
	communicationsPositionWeightLoad(filename,&(us->packetApi->weightList));
}
