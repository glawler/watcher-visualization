#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>

#include <libxml/xmlmemory.h>			/*libXML2  */
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <zlib.h>				/*libz   */

#include "des.h"
#include "apisupport.h"
#include "idsCommunications.h"
#include "marshal.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *
 * This will implement the API for talking to the hierarchy process.
 *
 * It is intended to be linked into other processes, and to exist in a
 * select loop
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: idsCommunications.c,v 1.101 2007/08/19 04:01:19 dkindred Exp $";

static void messageInfoInsert(MessageInfo *mi);
static MessageInfo* messageInfoRemove(CommunicationsStatePtr cs, 
				      const MessageID *tag);
static void messageInfoInvokeCallbackAndDestroy(MessageInfo *mi,
						MessageStatus status);
static int communicationsAttemptOpen(CommunicationsStatePtr cs);
static void communicationsLostConnection(CommunicationsStatePtr cs);
static void communicationsApiCommandSend(CommunicationsStatePtr cs,
					 ApiCommand *ac);
static int apiCommandHandle(ApiCommand *ac, CommunicationsState *cs);

static void defaultLogFunction(char const *fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	vfprintf(stderr, fmt, arglist);
	fflush(stderr);
	va_end(arglist);
	return;
} /* defaultLogFunction */

/* FIXME: globals are Very Bad.  The API is intended to be able to have multiple
 * instances.  See any of the watcher programs.
 */
// #warning The API has globals
static CommunicationsErrorFunction defaultErrorLog = defaultLogFunction;
static CommunicationsErrorFunction defaultWarnLog = defaultLogFunction;
static CommunicationsErrorFunction defaultDebugLog = defaultLogFunction;

/* This is called exactly once, to allocate and init our data structure.
 * the communicationsInit call will then attempt to connect to the demon,
 * which may be done repeatedly.
 */
static CommunicationsState *intCommunicationsInit(ManetAddr host)
{
	CommunicationsState *cs;

	cs=(CommunicationsState*)malloc(sizeof(*cs));
	if (cs == NULL)
		return NULL;

	cs->fd=-1;
	cs->readLog=NULL;
	cs->linkup=0;
	cs->host=host;
	cs->dlog=defaultDebugLog;
	cs->wlog=defaultWarnLog;
	cs->elog=defaultErrorLog;
	cs->positionList = NULL;
	cs->positionWeightList = NULL;
	cs->inflight = NULL;
	cs->typehandler = NULL;
	cs->neighborList = NULL;
	cs->neighborHandler = NULL;
	cs->neighborHandlerData = NULL;
	cs->statusHandler = NULL;
	cs->statusHandlerData = NULL;
	cs->nameHandler = NULL;
	cs->nameHandlerData = NULL;
	cs->name = NULL;

	cs->nexttag = 42;   /* this will be reset to a more unique value when we get the INIT command from demon */
	cs->initflag = 0;
	cs->logFD = -1;
	cs->lastEventTime = 0;

	return cs;
}

CommunicationsState *communicationsInit(ManetAddr host)
{
	CommunicationsState *cs;
	int rc;

	cs=intCommunicationsInit(host);

	/* The first open attempt MUST succeed.  otherwise we will not know our manet addr,
	 * and will not be able to return it with communicationsNodeAddress
	 */
	rc=communicationsAttemptOpen(cs);
#if 0
	if (rc<0)
	{
		free(cs);
		return NULL;
	}
	else
#endif
		return cs;
}

/* Called by a client to tell the demon what it is.
 * currently, this is for debugging, but the key variable will allow it
 * to be used as a way for a client to authenticate itself to the demon.
 */
void communicationsNameSet(CommunicationsStatePtr cs, const char *name, const char *key)
{
	ApiCommand *ac;
	int keylen=0;

	if (name==NULL)
		return;

	if (cs->name)
		free(cs->name);

	if (key==NULL)
		keylen=0;
	else
		keylen=strlen(key);

	cs->name=(ApiName*)malloc(sizeof(*cs->name)+strlen(name)+1+keylen+1);
	cs->name->name=(char*)cs->name+sizeof(*cs->name);
	cs->name->key=cs->name->name+strlen(name)+1;
	strcpy(cs->name->name,name);
	if (key==NULL)
		cs->name->key[0]=0;
	else
		strcpy(cs->name->key,key);
	cs->name->messagesSent=0;
	cs->name->messagesAcked=0;
	cs->name->messagesRec=0;
	cs->name->messagesUnacked=0;
	cs->name->next=NULL;

	ac=apiNameMarshal(cs->name);
	communicationsApiCommandSend(cs,ac);
	if (cs->logFD>0)
		communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
	apiCommandFree(ac);
}

void communicationsNameGet(CommunicationsStatePtr cs)
{
	ApiCommand *ac;
	ac=apiCommandMalloc(APICOMMAND_NAMEGET,0);
	communicationsApiCommandSend(cs,ac);
	apiCommandFree(ac);
}

void communicationsNameHandlerSet(CommunicationsStatePtr cs, void *nameHandlerData, NameHandler cb)
{
	cs->nameHandler=cb;
	cs->nameHandlerData=nameHandlerData;
}

void communicationsLogEnable(CommunicationsState *cs, int fd)
{
	ApiCommand *initlist=NULL,*ac;
	CommunicationsNeighbor *n;
	ApiInit init;

	cs->logFD=fd;
	
	/* Write the neighbor list...  */
	n=communicationsNeighborList(cs);
	while(n)
	{
		ac=communicationsNeighborMarshal(n);
		ac->next=initlist;
		initlist=ac;

		n=n->next;
	}

	init.localid=cs->localid;             /* and the init command...  */
	init.netmask=cs->localmask;
	init.apiVersion=API_VERSION;
	ac=apiInitMarshal(&init);
	initlist=apiCommandConcatenate(initlist,ac);

	communicationsLogApiCommandWrite(cs->logFD,initlist,cs->lastEventTime, cs->localid);
	apiCommandFree(initlist);
}

/* Function to walk list of communicationsStatePtrs, and see if we already have a node
 * in the list.
 */

static CommunicationsStatePtr communicationsLogSearch(CommunicationsStatePtr *list, ManetAddr addr)
{
	int i;

	for(i=0;list[i];i++)
		if (list[i]->localid==addr)
			return list[i];
	return NULL;
}

/* This will stuff a message type into a goodwin file, to make it appear as if a message was
 * received by the API.
 * It is intended for goodwin to watcher communications.
 */
void communicationsLogMessage(CommunicationsStatePtr cs, int type, unsigned char *payload, int payloadlen)
{
	MessageInfo mi;
	ApiCommand *ac;

	mi.cs=cs;
	mi.originApi=NULL;
	mi.chainApi=NULL;
	mi.tag=0;
	mi.demonId=0;
	mi.dataId=0;
	mi.statusCallback=NULL;
	mi.statusData=NULL;
	mi.origin=communicationsNodeAddress(cs);
	mi.dest.addr=mi.origin;
	mi.dest.type=COMMUNICATIONSDESTINATION_DIRECT;
	mi.dest.ttl=1;
	mi.type=type;
	mi.payload=payload;
	mi.payloadPtr=NULL;
	mi.payloadLen=payloadlen;

	ac=messageInfoMarshal(&mi);
	ac->type=APICOMMAND_MESSAGE_REC;
	if (cs->logFD>0)
		communicationsLogApiCommandWrite(cs->logFD,ac,
						 cs->lastEventTime, /* close enough? */
						 cs->localid);
	apiCommandFree(ac);
}

/* This loads a goodwin file, sets up all the CommunicationsState data structures, etc.
 */
CommunicationsLogStatePtr communicationsLogLoad(int fd)
{
	CommunicationsLogState *cl;
	CommunicationsState *nodeList[1024],*cs=NULL;
	size_t i, numnodes=0;
	ApiCommand *ac=NULL;
	destime tim=0, prevtim=0;
	ManetAddr addr;


	/* loop through init ApiCommands.  They will be either neighbors or inits.  
	 * An init will be zero or more neighbor commands for a node which we don't have an entry for,
	 * followed by its init command.  So live data begins with the first neighbor or non-init
	 * command for a node we have already listed.
	 */

	nodeList[numnodes]=NULL;

	while (1)
	{
		prevtim=tim;
		ac=communicationsLogApiCommandRead(fd, &tim, &addr);
                if (ac==NULL) break;
		if ((cs=communicationsLogSearch(nodeList,addr))!=NULL)
		{
			/* we've gotten an event from a node which we've already setup, so its the first data...  */
			break;
		}
		else
		{
			/* too many nodes? */
			assert (numnodes < sizeof(nodeList)/sizeof(nodeList[0]) - 1);

			cs=nodeList[numnodes++]= intCommunicationsInit(addr);
			nodeList[numnodes]=NULL;

			cs->linkup=1;
			cs->localid=addr;
			cs->logFD=-1;    /* This MUST be true for Step to call apiCommandHandle successfully */

			while(ac->type!=APICOMMAND_INIT)
			{
				switch(ac->type)
				{
					case APICOMMAND_NEIGHBOR:
					{
						CommunicationsNeighbor *cn;
						cn=communicationsNeighborUnmarshal(ac);
						communicationsIntNeighborUpdate(&(cs->neighborList),cn);	/* takes ownership of cn  */
					}
					break;
					default:
						fprintf(stderr,"Unexpected ApiCommand type!\n");
						abort();
					break;
				}
				apiCommandFree(ac);
				ac=communicationsLogApiCommandRead(fd, &tim, &addr);
				/* shouldn't see commands for any other nodes
				 * until we get the init */
				assert(addr == cs->host);
				cs->lastEventTime = tim;
			}
			/* ac now points to the init command for this node..  */
			apiCommandFree(ac);
		}
	}
	
	cl=(CommunicationsLogState*)malloc(sizeof(*cl)+sizeof(nodeList[0])*(numnodes+1));
	cl->nodeList=(CommunicationsState**)((unsigned char*)cl+sizeof(*cl));
	memcpy(cl->nodeList,nodeList,sizeof(nodeList[0])*(numnodes+1));
	cl->nextCmd=ac;
        cl->nextCmdTime=tim;
	cl->nextCmdCs=cs;
	cl->curtime=prevtim;
	cl->fd=fd;
	cl->inflight=0;

	for (i=0; cl->nodeList[i] != NULL; i++)
	{
		cl->nodeList[i]->readLog = cl;
	}

	return cl;
}

void communicationsLogClose(CommunicationsLogStatePtr cl)
{
	int i;
	for (i = 0; cl->nodeList[i]; i++)
	{
		/* Clearing readLog here causes the communicationsClose() to actually 
		 * free rather than just resetting. */
		cl->nodeList[i]->readLog = NULL;
		communicationsClose(cl->nodeList[i]);
	}
	/* do not free cl->nodeList -- it's allocated in the same malloc block as cl */
	if (cl->nextCmd) apiCommandFree(cl->nextCmd);
	cl->fd = -1; /* make sure we don't read anything else from the file */
	free(cl);
}

/* process a single event. return 1 on success, -1 on error */
static long communicationsLogStepSingle(CommunicationsLogState *cl, destime *callercurtime)
{
	destime acTime;
	ManetAddr addr;
	ApiCommand *ac;
	CommunicationsState *cs;

	if (cl->nextCmd==NULL)
	{
		return -1;	/* EOF */
	}

        /* save the pending event that was already read */
	ac=cl->nextCmd;
	cs=cl->nextCmdCs;
	acTime=cl->nextCmdTime;
	assert(cs!=NULL);

	/* buffer the next command from the file, if any */
	cl->nextCmd=communicationsLogApiCommandRead(cl->fd, &cl->nextCmdTime,
						    &addr);
	if (cl->nextCmd)
	{
		cl->nextCmdCs=communicationsLogSearch(cl->nodeList,addr);
		/* we should have already seen (in communicationsLogLoad())
		 * all the nodes that will appear in the file */
		assert(cl->nextCmdCs != NULL);
	}

#ifdef DEBUG_API
        cs->dlog("%s: Replaying command (type=%d).  time= %lld\n",
                 __func__, ac->type, acTime);
#endif
	/* process the pending event */
	cl->curtime=acTime;
	cs->lastEventTime=acTime;
	if (callercurtime)
	{
		*callercurtime=acTime;
	}

	/* apiCommandHandle() takes ownership of ac */
	apiCommandHandle(ac,cs);

	return 1;
}

/* This is like the checkIO function: it will call callbacks for events from
 * a goodwin file until the specified number of milliseconds have passed
 * (on the goodwin file's clock).  This may mean no events are processed.
 * If step==0, process exactly one event.
 */
long communicationsLogStep(CommunicationsLogState *cl, int step, destime *callercurtime)
{
	int cmdsProcessed = 0;
	destime starttime=cl->curtime;

	if (callercurtime)
	{
		/* in case we are at EOF, we'll still set callercurtime */
		*callercurtime=cl->curtime;
	}

	if (cl->inflight)
	{
		/* Some node may have an inflight message.  
		 * We can't send it since there's no daemon, so
		 * we will call its callback with MESSAGE_FAILED.
		 */
		int i;
		for (i=0; cl->nodeList[i]; i++)
		{
			MessageInfo *mi;
			if ((mi=messageInfoRemove(cl->nodeList[i],NULL)) != NULL)
			{
				messageInfoInvokeCallbackAndDestroy(mi,MESSAGE_FAILED);
				if (step == 0)
				{
					/* We're only supposed to do one
					 * "event", so return.  One could
					 * argue these don't count since they
					 * don't come from the log... 
					 */
					return 1;
				}
			}
		}
		/* we must have cleared all the inflight queues already */
		cl->inflight=0;
	}

	if (step == 0)
	{
		/* process exactly one command */
		return communicationsLogStepSingle(cl, callercurtime);
	}

	while(cl->nextCmd && cl->nextCmdTime < (starttime+step))
	{
		int ret = communicationsLogStepSingle(cl, callercurtime);
		assert(ret == 1); /* we already checked nextCmd!=NULL */
		cmdsProcessed++;
	}
	if (cl->nextCmd==NULL && cmdsProcessed==0)
	{
		return -1;	/* EOF */
	}

	/* advance curtime even if we didn't process any events, so we
	 * make progress */
	assert(cl->curtime <= starttime+step);
	cl->curtime=starttime+step;
	if (callercurtime)
	{
		*callercurtime=cl->curtime;
	}

	return cmdsProcessed;
}

CommunicationsLogStatePtr communicationsLogStateGet(CommunicationsStatePtr cs)
{
	return cs->readLog;
}

CommunicationsStatePtr const *communicationsLogNodesGet(CommunicationsLogStatePtr cl)
{
	return cl->nodeList;
}

destime communicationsLogTimeGet(CommunicationsLogStatePtr cl)
{
	return cl->curtime;
}

destime communicationsLogNextEventTimeGet(CommunicationsLogStatePtr cl)
{
	return cl->nextCmd ? cl->nextCmdTime : (destime) -1;
}


static int communicationsAttemptOpen(CommunicationsStatePtr cs)
{
	struct sockaddr_in addr;
	int i;

	cs->dlog("communicationsAttemptOpen: attempting open\n");

	cs->fd=socket(AF_INET, SOCK_STREAM, 0);
	if (cs->fd < 0)
	{
		cs->elog("IDS Communications: failed to open socket. "
		"\"%s\" (%d)\n", strerror(errno), errno);
		return -1;
	}

	i=FD_CLOEXEC;
	if (fcntl(cs->fd,F_SETFD, &i)<0)
	{
		cs->elog("IDSCommunications: fcntl failed. "
		"\"%s\" (%d)\n",strerror(errno),errno);
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(API_DEFAULTPORT);
	if (cs->host==0)
	{
		addr.sin_addr.s_addr = htonl(((127<<24) | 1));      /* localhost  127.0.0.1 */
	}
	else
	{
		addr.sin_addr.s_addr = htonl(cs->host);
	}
	

	if (connect(cs->fd,(struct sockaddr*)&addr,sizeof(addr))<0)
	{
#ifdef NEW_NETWORK
        char tmp[100];
#endif
        cs->elog("IDS Communications: failed to connect "
                "socket to %s:%u, \"%s\" (%d)\n",
#ifdef NEW_NETWORK
                inet_ntop(AF_INET, &addr.sin_addr, tmp, sizeof(tmp)),
#else
		inet_ntoa(addr.sin_addr),
#endif
                ntohs(addr.sin_port), strerror(errno), errno);
		// release FD
		if (close(cs->fd) != 0)
			cs->elog("IDS Communications: socket close failed.\n");
		cs->fd=-1;
		return -1;
	}

	/* The demon then sends us our neighbor list, as if they were just arriving.  
	 * The end of the init is indicated by the arrival of an APICOMMAND_INIT, which 
	 * sets the cs->initflag.
	 */
	while(cs->initflag==0)
		if (communicationsReadReady(cs))
		{
			// release FD
			if (close(cs->fd) != 0)
				cs->elog("IDS Communications: socket close failed.\n");
			cs->fd=-1;
			return -1;
   		};

	{
#ifdef NEW_NETWORK
        char tmp[100];
#endif
        cs->dlog( "IDS Communications: ready on %s:%d tag= %d\n",
#ifdef NEW_NETWORK
                inet_ntop(AF_INET, &addr.sin_addr, tmp, sizeof(tmp)),
#else
		inet_ntoa(addr.sin_addr),
#endif
                ntohs(addr.sin_port),
		cs->nexttag);
	}

	/* If we're reconnecting after loosing connectivity to the demon, we now need to walk our
	 * list of registered message types, and re-request them
	 */
		/* walk list of hierarchy position callbacks we have registered for:  */

	IDSPosition *poslist,*pos,*dpos;

	poslist=cs->positionList;
	cs->positionList=NULL;
	for(pos=poslist ; pos ; pos=pos->next)
	{
#ifdef DEBUG_API
		cs->dlog("Reregistering for position %s\n",idsPosition2Str(pos->position));
#endif
		idsPositionRegister(cs ,pos->position, pos->eligibility,pos->update,pos->updateData);
	}

		/* walk list of message type callbacks we have registered for:  */

	MessageTypeNode *messagetypelist, *messagetype, *dmessagetype;

	messagetypelist=cs->typehandler;
	cs->typehandler=NULL;
	for(messagetype=messagetypelist ; messagetype ; messagetype=messagetype->nextType)
	{
		messageHandlerSet(cs, messagetype->direction, messagetype->priority, messagetype->access, messagetype->type, messagetype->handler, messagetype->handlerData);
	}

	if (cs->fd<0)    /* we got an error...  keep the old lists  */
	{
		cs->positionList=poslist;
		cs->typehandler=messagetypelist;
		return -1;
	}
	else
	{
		pos=poslist;
		while(pos)
		{
			dpos=pos;
			pos=pos->next;
			free(dpos);
		}
		messagetype=messagetypelist;
		while(messagetype)
		{
			dmessagetype=messagetype;
			messagetype=messagetype->nextType;
			free(dmessagetype);
		}
	}

	/* neighbor events are always sent...  so no re-registering  */

	/* need to re-register for status handler? */

	if (cs->statusHandler)
		communicationsStatusRegister(cs,cs->statusPeriod, cs->statusHandler, cs->statusHandlerData);

	cs->dlog("ready!\n");
	cs->linkup=1;
	cs->lastEventTime = communicationsDestimeGet(cs);
	if (cs->logFD>=0)
	{
		ApiCommand *ac = apiCommandMalloc(APICOMMAND_LINKUP,0);
		communicationsLogApiCommandWrite(cs->logFD, ac, cs->lastEventTime, cs->localid);
		apiCommandFree(ac);
	}

	/* need to resend client name?  */
	if (cs->name)
	{
		ApiCommand *ac = apiNameMarshal(cs->name);
		communicationsApiCommandSend(cs, ac);
		if (cs->logFD > 0)
			communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
	}

	return 0;
}

/* Internal function called when a read or write fails.
 * 
 * It will call status callbacks for all the inflight messages, and arrange for
 * the connection to be retried, when the client calls communicationsReadReady
 */
static void communicationsLostConnection(CommunicationsStatePtr cs)
{
    if (cs->fd >= 0) close(cs->fd);
	cs->fd=-1;
	cs->initflag=0;
	cs->linkup=0;

	if (cs->logFD>=0)
	{
		ApiCommand *ac;
		ac=apiCommandMalloc(APICOMMAND_LINKDOWN,0);
		communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
		apiCommandFree(ac);
	}
	/* Do NOT call callbacks here, we're probably inside one.  Clear the FD, and the client
	 * must then call communicationsReadReady(), where we can safely call callbacks.
	 */
}

/* Send the given command to the daemon if we are connected to one.
 * On error, call communicationsLostConnection() so we will reconnect later.
 */
void communicationsApiCommandSend(CommunicationsStatePtr cs, ApiCommand *ac)
{
	if (cs->fd >= 0)
	{
		int rc=apiCommandWriteOrSend(cs->fd,ac,1);
		if (rc<0)
		{
			/* we have lost connection to the demon. */
			cs->lastEventTime = communicationsDestimeGet(cs);
			cs->elog( "idsCommunications: failed to write API command on fd %d: %s\n", cs->fd, strerror(errno));
			communicationsLostConnection(cs);
		}
	}
}

/*
 * Set the debug log function
 */
void communicationsLogDebugDefaultSet(void (*dlog)(char const *fmt, ...))
{
    if(dlog)
    {
        defaultDebugLog = dlog;
    }
    else
    {
        defaultDebugLog = defaultLogFunction;
    }
    return;
} /* communicationsLogDebugDefaultSet */

/*
 * Set the warn log function
 */
void communicationsLogWarnDefaultSet(void (*wlog)(char const *fmt, ...))
{
    if(wlog)
    {
        defaultWarnLog = wlog;
    }
    else
    {
        defaultWarnLog = defaultLogFunction;
    }
    return;
} /* communicationsLogWarnDefaultSet */

/*
 * Set the error log function
 */
void communicationsLogErrorDefaultSet(void (*elog)(char const *fmt, ...))
{
    if(elog)
    {
        defaultErrorLog = elog;
    }
    else
    {
        defaultErrorLog = defaultLogFunction;
    }
    return;
} /* communicationsLogErrorDefaultSet */

/*
 * Set the debug log function
 */
void communicationsLogDebugSet(
        CommunicationsStatePtr cs, void (*dlog)(char const *fmt, ...))
{
    if(dlog)
    {
        cs->dlog = dlog;
    }
    return;
} /* communicationsLogDebugSet */

/*
 * Set the warn log function
 */
void communicationsLogWarnSet(
        CommunicationsStatePtr cs, void (*wlog)(char const *fmt, ...))
{
    if(wlog)
    {
        cs->wlog = wlog;
    }
    return;
} /* communicationsLogWarnSet */

/*
 * Set the error log function
 */
void communicationsLogErrorSet(
        CommunicationsStatePtr cs, void (*elog)(char const *fmt, ...))
{
    if(elog)
    {
        cs->elog = elog;
    }
    return;
} /* communicationsLogErrorSet */

/* Subscribe for a message type
 */
int messageHandlerSet(
        CommunicationsStatePtr cs,
        CommunicationsMessageDirection communicationsMessageDirection,
        unsigned int priority,
        CommunicationsMessageAccess communicationsMessageAccess,
        MessageType messageType,
        MessageHandler messageHandlerFunc,
        void *messageHandlerData)
{
	ApiCommand *ac;
	MessageTypeNode *mth;
	int directionValid = 0;
	int accessValid = 0;

	/* parameter checking */

	/* this is a little contorted but gives compiler warning if we omit
	 * an enum value */
	switch (communicationsMessageDirection) {
	case COMMUNICATIONS_MESSAGE_INBOUND:  
	case COMMUNICATIONS_MESSAGE_OUTBOUND:
		directionValid=1;
	}
	if (!directionValid)
	{
		cs->elog("%s: Invalid direction (%d)\n", __func__, 
			 communicationsMessageDirection);
		return 0;	/* fail */
	}
	
	switch (communicationsMessageAccess) {
	case COMMUNICATIONS_MESSAGE_READONLY:  
	case COMMUNICATIONS_MESSAGE_READWRITE:
		accessValid=1;
	}
	if (!accessValid)
	{
		cs->elog("%s: Invalid access (%d)\n", __func__, 
			 communicationsMessageAccess);
		return 0;	/* fail */
	}

	if (messageHandlerFunc==NULL)                           /* do delete...  */
	{
		messageTypeHandlerDelete(&(cs->typehandler),messageType);

		ac=messageHandlerRequestMarshal(communicationsMessageDirection,priority,communicationsMessageAccess,messageType);
		ac->type=APICOMMAND_MESSAGETYPE_REMOVE;
		communicationsApiCommandSend(cs,ac);
		apiCommandFree(ac);

		return 1;
	}

	mth = messageTypeHandlerInsert(&(cs->typehandler),communicationsMessageDirection, priority, communicationsMessageAccess, messageType, messageHandlerFunc, messageHandlerData, 0);

        if (mth == NULL)
        {
		/* Right now various things assume only one handler registered 
		 * for each message type.  See RT issue #97. */
		cs->elog("IDS Communications: Already registered for message type 0x%x. To change handler, remove and register again.\n", messageType);
		return 0;
        }

#ifdef DEBUG_API
	cs->dlog("%s: Registering for type 0x%x direction %s priority 0x%x access %s\n",
			 __func__,
			 messageType,
			 communicationsMessageDirection == COMMUNICATIONS_MESSAGE_INBOUND ? "INBOUND"
			 : communicationsMessageDirection == COMMUNICATIONS_MESSAGE_OUTBOUND ? "OUTBOUND"
			 : "???",
			 priority,
			 communicationsMessageAccess == COMMUNICATIONS_MESSAGE_READONLY ? "READONLY"
			 : communicationsMessageAccess == COMMUNICATIONS_MESSAGE_READWRITE ? "READWRITE"
			 : "???");
#endif

	/* send message to demon listing the types we can handle */

	ac=messageHandlerRequestMarshal(communicationsMessageDirection,priority,communicationsMessageAccess,messageType);
	communicationsApiCommandSend(cs,ac);
	apiCommandFree(ac);

	return 1;
}

destime communicationsDestimeGet(CommunicationsStatePtr cs)
{
	if (cs->readLog)
	{
		return communicationsLogTimeGet(cs->readLog);
	}
	else
	{
		return getMilliTime();
	}
}

void communicationsTimevalGet(CommunicationsStatePtr cs, struct timeval *tv)
{
	if (cs->readLog)
	{
		destime t = communicationsLogTimeGet(cs->readLog);
		tv->tv_sec = t / 1000;
		tv->tv_usec = (t % 1000) * 1000;
	}
	else
	{
		gettimeofday(tv, NULL);
	}
}

destime communicationsLastEventTimeGet(CommunicationsStatePtr cs)
{
	return cs->lastEventTime;
}

MessageInfoPtr messageInfoCreate(
	CommunicationsState *cs,
	MessageType messageType,	/* note that this type is independant from the types used in simulator and demon land */
	CommunicationsDestination destination,
	MessageStatusHandler statusHandler,
	void *messageStatusHandlerData)
{
	MessageInfo *mi;

	/* check for unimplented addressing modes.
	 */
	if ((destination.type==COMMUNICATIONSDESTINATION_RECURSIVECHILDRENOF)
	    || (destination.type==COMMUNICATIONSDESTINATION_RECURSIVEPARENTSOF)
	    || (destination.type==COMMUNICATIONSDESTINATION_NEIGHBORSOF))
		return NULL;
	if ((destination.addr!=NODE_LOCAL) && 
	    ((destination.type==COMMUNICATIONSDESTINATION_CHILDRENOF)
             || (destination.type==COMMUNICATIONSDESTINATION_PARENTSOF)
             || (destination.type==COMMUNICATIONSDESTINATION_NEARESTCOORD)))
	{
		cs->elog("IDS Communications: unimplemented addressing mode!\n");
		return NULL;
	}

	mi=(MessageInfo*)malloc(sizeof(*mi));

	mi->statusCallback=statusHandler;
	mi->statusData=messageStatusHandlerData;
	mi->origin=cs->localid;
	mi->dest=destination;
	mi->type=messageType;
	mi->cs=cs;
	mi->apiOriginated=1;
	mi->tag=0;
	mi->demonId=0;
	mi->dataId=0;
	mi->routeFlags=0;

	mi->payload=NULL;
	mi->payloadPtr=NULL;
	mi->payloadLen=0;

	return mi;
}

void messageInfoDestroy(MessageInfo *mi)
{
	if (mi->payloadPtr)
		free(mi->payloadPtr);

	free(mi);
}

CommunicationsStatePtr messageInfoCommunicationsStateGet(const MessageInfo *messageInfo)
{
	return messageInfo->cs;
}

void *messageInfoRawPayloadGet(const struct MessageInfo *messageInfo)
{
	return messageInfo->payload;
}

size_t messageInfoRawPayloadLenGet(const struct MessageInfo *messageInfo)
{
	return messageInfo->payloadLen;
}

/*
 * At return, the messageinfo struct is in a block, the payload is in a second block,
 * and pointed to by payloadptr, so the second block will be properly freed.
 *
 * IE: messageInfo takes ownership of the block which the payload is in.
 */
void messageInfoRawPayloadSet(MessageInfoPtr messageInfo,void *payload, int len)
{
	if (messageInfo->payloadPtr)
		free(messageInfo->payloadPtr);

	messageInfo->payload=(unsigned char*)payload;
	messageInfo->payloadPtr=payload;
	messageInfo->payloadLen=len;
}


xmlDocPtr messageInfoPayloadGet(const struct MessageInfo *messageInfo)
{
	xmlDocPtr payload;

	payload = xmlParseMemory(
            (char const*)messageInfoRawPayloadGet(messageInfo),
            messageInfoRawPayloadLenGet(messageInfo));
	
	return payload;
}

void messageInfoPayloadSet(MessageInfoPtr messageInfo, const xmlDoc *payload)
{
	xmlChar *mem;
	int size;

	if (payload)
	{
		/* cast away const in "payload" since libxml doesn't use const 
		 * when it could. */
#ifdef __cplusplus
		xmlDocPtr payloadNonConst = const_cast<xmlDocPtr>(payload);
#else
		/* mwahaha, -Wcast-qual is silent about this: */
		xmlDocPtr payloadNonConst = (xmlDocPtr)(unsigned long)payload;
#endif	  
		xmlDocDumpMemory(payloadNonConst,&mem,&size);			/* convert xml struct into a xml stream  */
		messageInfoRawPayloadSet(messageInfo,mem,size);
	}
	else
		messageInfoRawPayloadSet(messageInfo,NULL,0);

}

ManetAddr messageInfoOriginatorGet(const struct MessageInfo *messageInfo)
{
	return messageInfo->origin;
}

MessageType messageInfoTypeGet(const struct MessageInfo *messageInfo)
{
	return messageInfo->type;
}

CommunicationsDestination messageInfoDestinationGet(const struct MessageInfo *messageInfo)
{
	return messageInfo->dest;
}

void messageInfoSend(MessageInfoPtr messageInfo)
{
/*
	Packets are transmitted to demon.  Packets have a tag determined by the API.
	That tag is then used so the API can recognize which messageInfo messages from
	the demon are refering to.
	The Demon (actually the data module) will also assign a unique ID for over the
	air.  When the packet
	ACK returns, the Demon looks up the over-the-air ID, and finds the API instance
	which owns it.  The API assigned tag is then returned with the status.

	(The Demon handles the ACK packets...)

*/
	ApiCommand *ac;

	messageInfo->tag=messageInfo->cs->nexttag++;

	ac=messageInfoMarshal(messageInfo);
	if (messageInfo->apiOriginated)
		ac->type=APICOMMAND_MESSAGE_SEND;
	else
		ac->type=APICOMMAND_MESSAGE_NEXT;
	ac->tag=0;

#ifdef DEBUG_API
	messageInfo->cs->dlog("sending message dst= %d dstype= %s type= %d tag= %d len= %d AC len= %d\n",messageInfo->dest.addr & 0xFF,communicationsDestinationType2Str(messageInfo->dest.type),messageInfo->type,messageInfo->tag,messageInfo->payloadLen,ac->len);
#endif
	if (messageInfo->cs->fd >= 0)
	{
		communicationsApiCommandSend(messageInfo->cs,ac);
#ifdef DEBUG_API
		messageInfo->cs->dlog( "sendMessage: buffering outgoing packet tag= %d  len= %d type= %s\n",
				       messageInfo->tag,messageInfoRawPayloadLenGet(messageInfo),
				       (ac->type==APICOMMAND_MESSAGE_SEND)?"SEND":"NEXT"
			);
#endif
	}
	apiCommandFree(ac);

	if (messageInfo->apiOriginated)
		messageInfoInsert(messageInfo);    /* we do this even if we lost the connection, because we can't call status callbacks until communicationsReadReady() is called  */
	else
		messageInfoDestroy(messageInfo);
}


int idsPositionRegister(
	CommunicationsStatePtr cs,
	IDSPositionType position,
	IDSPositionStatus eligibility,
	IDSPositionUpdateProc idsPositionUpdateProc,
	void *idsPositionUpdateProcData)
{
	ApiCommand *ac;

	if ((eligibility==IDSPOSITION_INACTIVE) || (idsPositionUpdateProc==NULL))    /* a NULL callback function means this detector is no longer eligible */
	{
		idsPositionDelete(&(cs->positionList),position);
		eligibility=IDSPOSITION_INACTIVE;
	}
	else
	{
		idsPositionInsert(&(cs->positionList),position,eligibility,idsPositionUpdateProc,idsPositionUpdateProcData);
	}

	/* send msg to demon  */
	ac=idsPositionMarshal(position,eligibility);
	communicationsApiCommandSend(cs,ac);
	apiCommandFree(ac);
	
	return 1;
}

IDSPosition *idsPositionCurrent(CommunicationsStatePtr cs)
{
	return cs->positionList;
}

IDSState *IDSStateMalloc(CommunicationsStatePtr cs, int num)
{
	IDSState *vect;
	size_t size = sizeof(*vect)+sizeof(vect->state[0])*num;
	
	vect=(IDSState*)malloc(size);
	memset(vect,0,size);
	vect->cs=cs;
	vect->state=(IDSStateElement*)(vect+1);
	vect->numNodes=num;
	return vect;
}

void IDSStateFree(IDSState *vect)
{
	unsigned int i;
	for (i = 0; i < vect->numNodes; i++)
	{
		free(vect->state[i].clusteringData);
	}
	/* the vect->state vector is part of the same malloc'ed block as "vect",
	 * so must not be freed separately */
	free(vect);
}

void IDSStateSet(IDSState *vect)
{
	ApiCommand *ac;

	ac=IDSStateMarshal(vect);
	communicationsApiCommandSend(vect->cs,ac);
	apiCommandFree(ac);
}


/**********************************************************************************
 *
 * Calls to manipulate the list of positions and weights.
 *
 */

/* Return current list
 */
const CommunicationsPositionWeight *communicationsPositionWeightGet(CommunicationsStatePtr cs)
{
	return cs->positionWeightList;
}

/* Search current list for a specific entry
 */
const CommunicationsPositionWeight *communicationsPositionWeightSearch(CommunicationsStatePtr cs, CommunicationsPositionWeight *position)
{
	return communicationsPositionWeightSearchList(cs->positionWeightList,position);
}

/* Search current list for a specific entry
 */
CommunicationsPositionWeight *communicationsPositionWeightSearchList(CommunicationsPositionWeight *list, const CommunicationsPositionWeight *position)
{ 
	while(list)
	{
		if ((list->addr==position->addr) && (list->position==position->position))
			break;
		list=list->next;
	}
	return list;
} 


/* Insert a series of entries
 * This does a copy.  caller keeps ownership of *list
 */
void communicationsPositionWeightAdd(CommunicationsStatePtr cs,const CommunicationsPositionWeight *list)
{
	const CommunicationsPositionWeight *p;
	ApiCommand *ac;

	for(p=list;p;p=p->next)
	{
		ac=communicationsPositionWeightMarshal(p);
		communicationsApiCommandSend(cs,ac);
		apiCommandFree(ac);
	}
	/* our list will be updated when demon replies with value.  Thus we see the demons' value, not ours  */
}

/* Delete a series of entries
 * Caller keeps ownership of *list
 */
void communicationsPositionWeightSub(CommunicationsStatePtr cs,CommunicationsPositionWeight *list)
{
	CommunicationsPositionWeight *p;
	ApiCommand *ac;

	for(p=list;p;p=p->next)
	{
		p->weight=COMMUNICATIONSPOSITIONWEIGHT_DEFAULT;
		ac=communicationsPositionWeightMarshal(p);
		communicationsApiCommandSend(cs,ac);
		apiCommandFree(ac);
	}
}


static void messageInfoInsert(MessageInfo *mi)
{
	mi->next=mi->cs->inflight;      /* list insert into inflight messages */
	mi->cs->inflight=mi;
	if (mi->cs->readLog) mi->cs->readLog->inflight=1;
}

/* Remove the first messageInfo with the given tag from the inflight list,
 * or if tag==NULL, remove the first one on the list.
 * Return the removed messageInfo, or NULL if none was found.
 */
static MessageInfo *messageInfoRemove(CommunicationsState *cs,
				      const MessageID *tag)
{
	MessageInfo *p,*q;

	q=NULL;
	p=cs->inflight;

	if (tag)
	{
		while (p!=NULL && p->tag!=*tag)
		{
			q=p;
			p=p->next;
		}
	}

	if (p)
	{
		if (q)
			q->next=q->next->next;
		else
			cs->inflight=cs->inflight->next;

		p->next = NULL;
	}
	return p;
}

static void messageInfoInvokeCallbackAndDestroy(MessageInfo *mi,
						MessageStatus status)
{
	if (mi->statusCallback)
	{
		(mi->statusCallback)(mi,mi->statusData,status);
	}
	messageInfoDestroy(mi);
}

int communicationsReturnFD(CommunicationsState *cs)
{
	return cs->fd;
}

int communicationsLinkup(CommunicationsStatePtr cs)
{
	return cs->linkup;
}

/* takes ownership of ac */
static int apiCommandHandle(ApiCommand *ac, CommunicationsState *cs)
{
	MessageInfo *mi;

	switch(ac->type)
	{
		case APICOMMAND_MESSAGE_ACK:
		case APICOMMAND_MESSAGE_NAK:
		{

			/* we got an ack for a packet.  look up the MI for that packet in the pending list, and call its error handler if appropriate.  */
			/* remove MI from pending list  */

			mi=messageInfoRemove(cs,&ac->tag);
			if (mi)
			{
				MessageStatus status = 
					(ac->type == APICOMMAND_MESSAGE_ACK
					 ? MESSAGE_SUCCESSFUL
					 : MESSAGE_FAILED);
#ifdef DEBUG_API
				cs->dlog(
					"idsCommunications: "
					"got a%s for an inflight packet\n",
					(ac->type == APICOMMAND_MESSAGE_ACK ?  "n ACK" : " NAK"));
#endif
				messageInfoInvokeCallbackAndDestroy(mi,
								    status);
			}
			else
			{
				cs->wlog( "idsCommunications: "
					"CommunicationsReadReady: "
					"Got a%s for a packet we don't have! "
					"tag= %d\n",
					(ac->type == APICOMMAND_MESSAGE_ACK ? "n ACK" : " NAK"),
					ac->tag);
			}
			apiCommandFree(ac);   /* This is the status AC.  nothing refers to it  */
		}
		break;
		case APICOMMAND_MESSAGE_REC:
		{
			/* incoming packets do not have tags.  look up the handler to call by its message type.  */

			MessageTypeNode *mth;
			mi=messageInfoUnmarshal(ac);
			mi->apiOriginated=0;
			mi->cs=cs;
			
			mth=messageTypeHandlerSearch(&(cs->typehandler),mi->type);

			if (mth)
			{
				(mth->handler)(mth->handlerData,mi);      /* and caller frees mi */
				if (cs->logFD>=0)
				{
					/* write apicommand to log file   */
					communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
				}
			}
			messageInfoDestroy(mi);
			/* DO NOT FREE ac, mi points into it, and messageInfoDestroy is expecting to free it */
		}
		break;
		case APICOMMAND_POSITION:
		{
			IDSPositionType position;
			IDSPositionStatus stat;
			IDSPosition *rn;
			
			idsPositionUnmarshal(ac,&position,&stat);
			rn=idsPositionSearch(&(cs->positionList),position);
#ifdef DEBUG_API
			cs->dlog("Got an APICOMMAND_POSITION %s %s %p\n",idsPosition2Str(position),idsPositionStatus2Str(stat),rn);
#endif
			if (rn)
			{
				rn->status=stat;
				(rn->update)(rn->updateData,rn->position,rn->status);
				if (cs->logFD>=0)
					communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
			}
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_NEIGHBOR:
		{
			CommunicationsNeighbor *cn;

			cn=communicationsNeighborUnmarshal(ac);
#ifdef DEBUG_API
			cs->dlog("Got an APICOMMAND_NEIGHBOR %d\n",cn->addr & 0xFF);
#endif
			if (cs->neighborHandler)
				(cs->neighborHandler)(cs->neighborHandlerData,cn);
			if (cs->logFD>=0)
				communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
			communicationsIntNeighborUpdate(&(cs->neighborList),cn);	/* takes ownership of cn  */

			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_INIT:
		{
			ApiInit *init;

			init=apiInitUnmarshal(ac);
			cs->localid=init->localid;
			cs->localmask=init->netmask;
			if (init->apiVersion!=API_VERSION)
			{
				cs->dlog("This client is compiled with the wrong version of the API for this demon!  client is %d demon is %d\n",API_VERSION,init->apiVersion);
				free(init);
				apiCommandFree(ac);

				return -1;
			}
			
			cs->initflag=1;
#ifdef DEBUG_API
			cs->dlog("got ApiInit, localid= %d mask= %d.%d.%d.%d\n",cs->localid & 0xFF,PRINTADDR(cs->localmask));
#endif
			cs->nexttag=(( cs->localid << 16) | ((getpid() & 0xFF) << 8)) & 0x00FFFFFF;
			free(init);

			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_STATUS:
		{
			ApiStatus *as;
		
			as=apiStatusUnmarshal(ac);
#ifdef DEBUG_API
			cs->dlog("idsCommunications: got APICOMMAND_STATUS\n");
#endif
			if (cs->statusHandler)
			{
				(cs->statusHandler)(cs->statusHandlerData,as);
			}
			if (cs->logFD>=0)
			{
				/* write apicommand to log file   */
				communicationsLogApiCommandWrite(cs->logFD,ac,cs->lastEventTime,cs->localid);
			}
			free(as);
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_POSITIONWEIGHT:
		{
			CommunicationsPositionWeight *cpw;

			cpw=communicationsPositionWeightUnmarshal(ac);

			communicationsPositionWeightInsert(&(cs->positionWeightList),cpw);

			/* Call callback here? (we could, but its not in the API right now) */

#ifdef DEBUG_API
			cs->dlog("idsCommunications: got a POSITIONWEIGHT addr= %d.%d.%d.%d pos= %s weight= 0x%x\n",PRINTADDR(cpw->addr),idsPosition2Str(cpw->position),cpw->weight);
#endif

			free(cpw);
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_LINKUP:
			cs->linkup=1;
			apiCommandFree(ac);
		break;
		case APICOMMAND_LINKDOWN:
			cs->linkup=0;
			apiCommandFree(ac);
		break;
		case APICOMMAND_NAMEGET:
		{
			if (cs->nameHandler)
			{
				ApiName *list=apiNameUnmarshal(ac);
				(cs->nameHandler)(cs->nameHandlerData,list);
				apiNameFree(list);
			}
			apiCommandFree(ac);
		}
		break;
		case APICOMMAND_NAMESET:
		/* These are only sent by clients, but are logged in goodwin
		 * logs too, so a client reading from log can see one. */
		apiCommandFree(ac);
		break;
                /* commands only sent by clients */
		case APICOMMAND_TIME_GET:
		case APICOMMAND_MESSAGE_SEND:
		case APICOMMAND_MESSAGETYPE_REQUEST:
		case APICOMMAND_MESSAGETYPE_REMOVE:
		case APICOMMAND_MESSAGE_NEXT:
		case APICOMMAND_RAWSEND:
		default:
			cs->elog( "idsCommunications: "
				"CommunicationsReadReady: "
				"unknown or unhandleable apiCommand type %d\n",
				ac->type);

			apiCommandFree(ac);
		break;
	}
	return 0;
}

int communicationsReadReady(CommunicationsState *cs)
{
/*

	This ac may be:
		returning status for an outgoing packet.
			(look up its tag in outstanding msgs, and call the callback)
		an incoming packet.
			(deliver it to the callback for that type)
		an incoming role update.
			(call the callback for updateing the Role)
		return data for a command.
			role request
			time request
			neighborhood request

*/
	ApiCommand *ac;
	MessageInfo *mi;

	if (cs->fd<0)
	{
		/* We lost the connection to the demon last read or write...  walk inflight, and call 
		 * status callbacks with error indications  */
		while ((mi=messageInfoRemove(cs,NULL)) != NULL)
		{
			messageInfoInvokeCallbackAndDestroy(mi,MESSAGE_FAILED);
		}
		/* Free neighbors list 
		*/
		communicationsIntNeighborFlush(&(cs->neighborList));

		communicationsAttemptOpen(cs);
		/* Don't want to do an apiCommandRead() here because it 
		 * could block. */
		return (cs->fd>=0)?0:-1;
	}

	ac=apiCommandRead(cs->fd, cs->elog);

	cs->lastEventTime = communicationsDestimeGet(cs);

	if (ac==NULL)
	{
		cs->elog( "idsCommunications: "
			"failed to read API command on fd %d\n", cs->fd);
		communicationsLostConnection(cs);
		return -1;
	}

	/* apiCommandHandle() takes ownership of ac */
	return apiCommandHandle(ac,cs);
}



/**********************************************************************************
 *
 * Calls to manipulate the neighbor list
 */

/* Register a callback function to be called when a neighbor arrives or departs
 */
void communicationsNeighborRegister(CommunicationsStatePtr cs, CommunicationsNeighborUpdateProc communicationsNeighborUpdateProc, void *communicationsNeighborUpdateProcData)
{
	cs->neighborHandler=communicationsNeighborUpdateProc;
	cs->neighborHandlerData=communicationsNeighborUpdateProcData;
}

/* Return current list of neighbors 
 */
CommunicationsNeighbor *communicationsNeighborList(CommunicationsStatePtr cs)
{
	return cs->neighborList;
}

/* Search current list of neighbors for a specific node
 */
CommunicationsNeighbor *communicationsNeighborSearch(
        CommunicationsStatePtr cs,
        ManetAddr neighborManetAddr)
{
	CommunicationsNeighbor *cn;
	for(cn=cs->neighborList;cn;cn=cn->next)
		if (cn->addr == neighborManetAddr)
			return cn;

	return NULL;
}

/* Return this node's address
 */
ManetAddr communicationsNodeAddress(CommunicationsStatePtr cs)
{
	if (cs==NULL)
		return 0;
	return cs->localid;
}

ManetAddr communicationsNodeMask(CommunicationsStatePtr cs)
{
	if (cs==NULL)
		return 0;
	return cs->localmask;
}


/**********************************************************************************
 *
 * Debugging entry points.  
 */

void communicationsStatusRegister(CommunicationsStatePtr cs, int period, CommunicationsStatusUpdateProc communicationsStatusUpdateProc, void *communicationsStatusUpdateProcData)
{
	ApiStatus status;
	ApiCommand *ac;

	cs->statusPeriod=period;
	cs->statusHandler=communicationsStatusUpdateProc;
	cs->statusHandlerData=communicationsStatusUpdateProcData;

	status.packetList=NULL;
	if (cs->statusHandler)
		status.period=period;
	else
		status.period=-1;

	status.numtypes=0;

	ac=apiStatusMarshal(&status);

	communicationsApiCommandSend(cs,ac);
	apiCommandFree(ac);
}

void communicationsWatcherFloatingLabel(CommunicationsStatePtr cs, FloatingLabel *lab)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);    /* "sufficient" */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	pos=communicationsWatcherFloatingLabelMarshal(pos, lab);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,msg,pos-msg);
        messageInfoSend(mi);
}

void communicationsWatcherFloatingLabelRemove(CommunicationsStatePtr cs,int bitmap, FloatingLabel *lab)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);    /* "sufficient" */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	MARSHALBYTE(pos,bitmap);
	pos=communicationsWatcherFloatingLabelMarshal(pos, lab);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL_REMOVE,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,msg,pos-msg);
        messageInfoSend(mi);
}

void communicationsWatcherLabel(CommunicationsStatePtr cs, NodeLabel *lab)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);    /* "sufficient" */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	if ((lab->node==NODE_LOCAL) || (lab->node==0))
		lab->node=cs->localid;

	pos=communicationsWatcherLabelMarshal(pos, lab);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,msg,pos-msg);
        messageInfoSend(mi);
}

void communicationsWatcherLabelRemove(CommunicationsStatePtr cs, int bitmap, NodeLabel *lab)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);    /* "sufficient" */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	if ((lab->node==NODE_LOCAL) || (lab->node==0))
		lab->node=cs->localid;

	MARSHALBYTE(pos,bitmap);
	pos=communicationsWatcherLabelMarshal(pos, lab);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL_REMOVE,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,msg,pos-msg);
        messageInfoSend(mi);
}

void communicationsWatcherProperty(CommunicationsStatePtr cs, ManetAddr node, WatcherPropertyInfo *prop)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);   /*  "sufficient"  */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	if ((node==NODE_LOCAL) || (node==0))
		node=cs->localid;

	if ((prop->identifier==NODE_LOCAL) || (prop->identifier==0))
		prop->identifier=cs->localid;

	pos=communicationsWatcherPropertyMarshal(pos, prop);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_PROPERTY,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,msg,pos-msg);
        messageInfoSend(mi);
}

void communicationsWatcherColor(CommunicationsStatePtr cs, ManetAddr node, unsigned char *color)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);   /*  "sufficient"  */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	if ((node==NODE_LOCAL) || (node==0))
		node=cs->localid;

	pos=watcherColorMarshal(msg, node, color);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,msg,pos-msg);
        messageInfoSend(mi);
}

void communicationsWatcherEdge(CommunicationsStatePtr cs, NodeEdge *edge)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);   /* "sufficient"  */
	unsigned char *pos=msg;
	int i;
	MessageInfoPtr mi;
	int labelflag=0;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	if ((edge->head==NODE_LOCAL) || (edge->head==0))
		edge->head=cs->localid;
	if ((edge->tail==NODE_LOCAL) || (edge->tail==0))
		edge->tail=cs->localid;

	MARSHALLONG(pos,edge->head);
	MARSHALLONG(pos,edge->tail);
	MARSHALLONG(pos,edge->tag);
	MARSHALBYTE(pos,edge->family);
	MARSHALBYTE(pos,edge->priority);
	for(i=0;i<4;i++)
		MARSHALBYTE(pos,edge->color[i]);

	if (edge->labelHead.text)
		labelflag|=1;
	if (edge->labelMiddle.text)
		labelflag|=2;
	if (edge->labelTail.text)
		labelflag|=4;
	MARSHALBYTE(pos,labelflag);
	if (edge->labelHead.text)
		pos=communicationsWatcherLabelMarshal(pos,&(edge->labelHead));
	if (edge->labelMiddle.text)
		pos=communicationsWatcherLabelMarshal(pos,&(edge->labelMiddle));
	if (edge->labelTail.text)
		pos=communicationsWatcherLabelMarshal(pos,&(edge->labelTail));
	MARSHALBYTE(pos,edge->width);
	MARSHALLONG(pos,edge->expiration);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE,dst,NULL,NULL);
	messageInfoRawPayloadSet(mi,msg,pos-msg);
	messageInfoSend(mi);
}

void communicationsWatcherEdgeRemove(CommunicationsStatePtr cs, int bitmap, ManetAddr head, ManetAddr tail, int family, int priority, int tag)
{
	CommunicationsDestination dst;
	unsigned char *msg = (unsigned char *)malloc(1024);   /* "sufficient"  */
	unsigned char *pos=msg;
	MessageInfoPtr mi;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	if ((head==NODE_LOCAL) || (head==0))
		head=cs->localid;
	if ((tail==NODE_LOCAL) || (tail==0))
		tail=cs->localid;

	MARSHALBYTE(pos,bitmap);
	MARSHALLONG(pos,head);
	MARSHALLONG(pos,tail);
	MARSHALBYTE(pos,family);
	MARSHALBYTE(pos,priority);
	MARSHALLONG(pos,tag);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE_REMOVE,dst,NULL,NULL);
	messageInfoRawPayloadSet(mi,msg,pos-msg);
	messageInfoSend(mi);
}

/**********************************************************************************
 *
 * Code to prettyprint messages
 */

/*
 * Returns a static buffer holding ddd.ddd.ddd.ddd style representation
 * of "x". NODE_LOCAL, NODE_ALL, and NODE_ROOTGROUP get treated separately
 * and return the corresponding string instead of a dotted quad.
 */
char const *manetAddr2Str(ManetAddr x)
{
    static char ret[16];
    if(x == NODE_LOCAL)
    {
        strcpy(ret, "NODE_LOCAL");
    }
    else if(x == NODE_ALL)
    {
        strcpy(ret, "NODE_ALL");
    }
    else if(x == NODE_ROOTGROUP)
    {
        strcpy(ret, "NODE_ROOTGROUP");
    }
    else
    {
        sprintf(ret, "%d.%d.%d.%d",
                (x >> 24) & 0xFF,
                (x >> 16) & 0xFF,
                (x >> 8) & 0xFF,
                x & 0xFF);
    }
    return ret;
} /* addr2Str */

/*
 * Returns a static buffer holding a text representation of a message
 * type.
 */
char const *messageType2Str(MessageType messageType)
{
	char const *ret;
	switch(messageType)
	{
	default:
	{
		static char unknown[50];
		sprintf(unknown, "Unknown (type %d)", (int)messageType);
		ret = unknown;
	}
	}
	return ret;
} /* messageType2Str */


char const *communicationsNeighborState2Str(CommunicationsNeighborState st)
{
	switch (st)
	{
		case COMMUNICATIONSNEIGHBOR_ARRIVING:
			return "Arriving";
		break;
		case COMMUNICATIONSNEIGHBOR_UPDATING:
			return "Updating";
		break;
		case COMMUNICATIONSNEIGHBOR_DEPARTING:
			return "Departing";
		break;
	}
	return "UNKNOWN";
}
/*
 * Returns a static buffer holding a text representation of a position.
 */
char const *idsPosition2Str(IDSPositionType position)
{
	char const *ret;
	switch(position)
	{
	case COORDINATOR_NEIGHBORHOOD:
		ret = "COORDINATOR_NEIGHBORHOOD";
	break;
	case COORDINATOR_REGIONAL:
		ret = "COORDINATOR_REGIONAL";
	break;
	case COORDINATOR_ROOT:
		ret = "COORDINATOR_ROOT";
	break;
	case COORDINATOR_ROOTGROUP:
		ret = "COORDINATOR_ROOTGROUP";
	break;
	default:
	{
		static char unknown[50];
		sprintf(unknown, "Unknown (position %d)", (int)position);
		ret = unknown;
	}
	}
	return ret;
} /* idsPosition2Str */

/*
 * Returns a static buffer holding a text representation of a position
 * status.
 */
char const *idsPositionStatus2Str(IDSPositionStatus idsPositionStatus)
{
	char const *ret;
	switch(idsPositionStatus)
	{
	case IDSPOSITION_ACTIVE:
		ret = "active";
	break;
	case IDSPOSITION_UNDEFINED:
		ret = "undefined";
	break;
	case IDSPOSITION_INACTIVE:
		ret = "inactive";
	break;
	case IDSPOSITION_INFORM:
		ret = "inform";
	break;
	default:
	{
		static char unknown[50];
		sprintf(unknown, "Unknown (position status %d)",
		(int)idsPositionStatus);
		ret = unknown;
	}
	break;
	}
	return ret;
} /* idsPositionStatus2Str */

char const *communicationsDestinationType2Str(CommunicationsDestinationType t)
{
	switch(t)
	{
		case COMMUNICATIONSDESTINATION_DIRECT:
			return "Direct";
		case COMMUNICATIONSDESTINATION_DIRECTBACKUP:
			return "DirectBackup";
		case COMMUNICATIONSDESTINATION_BROADCAST:
			return "Broadcast";
		case COMMUNICATIONSDESTINATION_NEIGHBORSOF:
			return "NeighborsOf";
		case COMMUNICATIONSDESTINATION_CHILDRENOF:
			return "ChildrenOf";
		case COMMUNICATIONSDESTINATION_RECURSIVECHILDRENOF:
			return "RecursiveChildrenOf";
		case COMMUNICATIONSDESTINATION_PARENTSOF:
			return "ParentsOf";
		case COMMUNICATIONSDESTINATION_RECURSIVEPARENTSOF:
			return "RecursiveParentsOf";
		case COMMUNICATIONSDESTINATION_NEARESTCOORD:
			return "NearestCoord";
		case COMMUNICATIONSDESTINATION_MULTICAST:
			return "Multicast";
	}
        return "IllegalVal";
}

/* converts a string into an arbitrary int, used for specifying tags for edges
 * and labels.
 */
int communicationsTagHash(char *str)
{
	int h=0;
	while(*str)
	{
		h=h*257 + *str;
		str++;
	}
	return h;
}

/* 
 * buffer is a pointer to a char* to put a pointer to the malloced buffer into
 * the length of the buffer will be put into *len
 */
void graphMarshal(const char *graphName, const float *floatData, int numPoints, unsigned char **buffer, int *len)
{
    int i;
    unsigned char *buff,*pos;

    buff=(unsigned char *)malloc(1024); // "sufficent"
    pos=buff;

    MARSHALSTRINGSHORT(pos, graphName);

    MARSHALLONG(pos, numPoints);
    for(i=0;i<numPoints;i++)
    {
        char floatBuf[64];
        snprintf(floatBuf, sizeof(floatBuf), "%f", floatData[i]); 
        // fprintf(stderr, "Marshalled float: %f, as string: %s\n", floatData[i], floatBuf);
        MARSHALSTRINGSHORT(pos, floatBuf);
    }

    *buffer=buff;
    *len=pos-buff; // This is bad.
}

// void graphUnmarshal(float *graph, int numnodes, const unsigned char *buffer)
// {
//     assert(0);  // unmarshalling done in watcher for now. 
// 
// 	int i,j;
// 	const unsigned char *pos = buffer;
// 	int dat;
// 
// 	for(i=0;i<numnodes;i++)
// 		for(j=0;j<numnodes;j++)
// 		{
// 			UNMARSHALSHORT(pos,dat);
// 			graph[i+(j*numnodes)]=dat/100.0;
// 		}
// }

void graphSend(CommunicationsStatePtr cs, const char *graphName, const float *dataPts, int numDataPts)
{
	unsigned char *payload;
	int payloadlen;
	MessageInfoPtr mi;
	CommunicationsDestination dst;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	graphMarshal(graphName, dataPts, numDataPts, &payload, &payloadlen);

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,payload,payloadlen);
        messageInfoSend(mi);
}

void graphSendEdge(CommunicationsStatePtr cs, CommunicationsGraphEdge *graph, int numnodes)
{
	unsigned char *payload,*hp;
	int payloadlen, dat, i;
	MessageInfoPtr mi;
	CommunicationsDestination dst;

	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	payloadlen=numnodes*10;
	payload=(unsigned char *)malloc(payloadlen);

	hp=payload;
	for(i=0;i<numnodes;i++)
	{
//		fprintf(stderr,"sending edge %d.%d.%d.%d to %d.%d.%d.%d  len %f\n",PRINTADDR(graph[i].a),PRINTADDR(graph[i].b),graph[i].value);
		MARSHALLONG(hp,graph[i].a);
		MARSHALLONG(hp,graph[i].b);
		dat=graph[i].value * 100.0;
		MARSHALSHORT(hp, dat);
	}

	mi=messageInfoCreate(cs,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH_EDGE,dst,NULL,NULL);
        messageInfoRawPayloadSet(mi,payload,payloadlen);
        messageInfoSend(mi);
}

void timevalFromDestime(destime t, struct timeval *tv)
{
	tv->tv_sec = t / 1000;
	tv->tv_usec = (t % 1000) * 1000;
}
