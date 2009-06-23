#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>

#include <zlib.h>

#include "des.h"
#include "apisupport.h"
#include "marshal.h"
#include "packetapi.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: apisupport.c,v 1.79 2007/08/22 18:20:09 dkindred Exp $";

/* Copyright (C) 2005  McAfee Inc.
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */


/* convenience function, to return unix time in milliseconds
 * simulator timestamps are milliseconds.
 */
destime getMilliTime(void) 
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	return (long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;
}

#define HEADERSIZE (4*3)


/*********************************************************************************
 *
 * The API (whose external interface is described in idsCommunications.h)
 * communicates with a demon via a TCP network connection.  The datastructures
 * which pass over that TCP connection are apiCommands.  The API has a second
 * .h file for its internal interfaces named apisupport.h
 *
*/

/* Create an apiCommand.  The type is defined in apisupport.h
*/
ApiCommand *apiCommandMalloc(ApiCommandType type,int len)
{
	ApiCommand *ac;

	ac=(ApiCommand*)malloc(sizeof(*ac)+len);
	ac->type=type;
	ac->len=len;
	ac->tag=0;
	ac->payload=((unsigned char*)ac)+sizeof(*ac);
	ac->payloadptr=NULL;
	ac->next=NULL;

	// fprintf(stderr,"apiCommandMalloc: ac at %p payload at %p len= %d\n",ac,ac->payload, len);
	return ac;
}

/* apiCommands have a next pointer, for building linked lists.  
 * Thus, if you have an apiCommand, and need to send it to multiple clients,
 * you will need to copy the apiCommand, to put an individual apiCommand in
 * each client's queue.  
 *
 * Note that this needlessly duplicates the payload.  A future optimization
 * is a reference counting thing, which should be able to be hidden in the
 * apiCommand calls.
*/
ApiCommand *apiCommandCopy(ApiCommand *oac)
{
	ApiCommand *ac,*aclist=NULL;

	while(oac)
	{
		ac=apiCommandMalloc(oac->type,oac->len);
		memcpy(ac->payload,oac->payload,oac->len);
		ac->next=aclist;
		aclist=ac;

		oac=oac->next;
	}

	return aclist;
}

/* Concatenate two lists of ApiCommands.  
 */
ApiCommand *apiCommandConcatenate(ApiCommand *first, ApiCommand *second)
{
	ApiCommand *ac;

	if (first==NULL)
		return second;

	ac=first;
	while(ac->next!=NULL)
		ac=ac->next;
	ac->next=second;
	return first;
}



/* wrapper function to retry reads which get interupted
 *
 *   Copied from Steven's Unix Network Programming
 */

static ssize_t                                  /* Read "n" bytes from a descriptor. */
mread(int fd, void *vptr, size_t n)
{
	size_t  nleft;
	ssize_t nread;
	char    *ptr;

	ptr = (char*)vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ( (nread = read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
				nread = 0;              /* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
		break;                          /* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);              /* return >= 0 */
}
/* end mread */

/* Read an apiCommand from a filedescriptor, and unmarshal it into an apiCommand
 * struct.  Those structs are then unmarshaled a second time, using code further down
 * in this file
*/
ApiCommand *apiCommandRead(
        int fd,
        void (*elog)(char const *fmt, ...) __attribute__ ((format(printf, 1, 2))))
{
	ApiCommand *ac;
	unsigned char header[HEADERSIZE],*hp;
	ssize_t rc;
	int len;
	int type;

	rc=mread(fd,header,HEADERSIZE);
	if (rc!=HEADERSIZE)
	{
		if (elog)
			elog("apiCommandRead: failed to read command header fd= %d rc= %d errno= %d %s\n", fd, rc, errno, strerror(errno));
		return NULL;
        }

	hp=header;
	UNMARSHALLONG(hp,len);
	UNMARSHALLONG(hp,type);

	if ((len>128*1024) || (len<0))
	{
		if (elog)
			elog("apiCommandRead: failed to read command header fd= %d len= %d type= 0x%x sanity fail\n",fd,len,type);
		return NULL;
	}
	
	ac=apiCommandMalloc(type,len);

	rc=mread(fd,ac->payload,ac->len);
	if (rc != (ssize_t)ac->len)
	{
		if (elog)
			elog("apiCommandRead: failed to read command body fd= %d rc= %d errno= %d %s\n", fd, rc, errno, strerror(errno));
		free(ac);
		return NULL;
	}

	UNMARSHALLONG(hp,ac->tag);
	return ac;
}

/* Opposite of apiCommandRead, takes an apiCommand struct, and marshals it into a
 * file descriptor.
 */
int apiCommandWriteOrSend(int fd, const ApiCommand *ac, int useSendMsg)
{
	int rc;
	int count = 0;
	ApiCommand const *aciter = ac;
	for(aciter = ac, count = 0; aciter; ++count, aciter = aciter->next) {}
	if(count > 1)
	{
		size_t ioveclen = sizeof(struct iovec)*2*count;
		size_t headerlen = HEADERSIZE*count;
		uint8_t *store = malloc(ioveclen + headerlen);
		if(store)
		{
			struct iovec *vector = (struct iovec*)store;
			uint8_t *header = store + ioveclen;
			struct iovec *v = vector;
			for(aciter = ac; aciter; aciter = aciter->next, header += HEADERSIZE, v += 2)
			{
				uint8_t *hp = header;
				MARSHALLONG(hp, ac->len);
				MARSHALLONG(hp, ac->type);
				MARSHALLONG(hp, ac->tag);
				v[0].iov_base = header;
				v[0].iov_len = HEADERSIZE;
				v[1].iov_base = ac->payload;
				v[1].iov_len = ac->len;
			}
			if(!useSendMsg)
			{
				rc = writev(fd, vector, 2*count);
			}
			else
			{
				struct msghdr msgHeader={NULL, 0, vector, 2*count, NULL, 0, 0};
				rc = sendmsg(fd, &msgHeader, MSG_NOSIGNAL); 
			}
			free(store);
		}
		else
		{
			fprintf(stderr, "%s: Failed to allocate %u bytes\n", 
					__func__, ioveclen + headerlen);
			rc = 0;
		}
	}
	else if(count == 1)
	{
		uint8_t header[HEADERSIZE];
		struct iovec vector[2];
		uint8_t *hp = header;
		MARSHALLONG(hp,ac->len);
		MARSHALLONG(hp,ac->type);
		MARSHALLONG(hp,ac->tag);
		vector[0].iov_base = header;
		vector[0].iov_len = HEADERSIZE;
		vector[1].iov_base = ac->payload;
		vector[1].iov_len = ac->len;

		if(!useSendMsg)
		{
			rc = writev(fd,vector,sizeof(vector)/sizeof(vector[0]));
		}
		else
		{
			struct msghdr msgHeader={NULL, 0, vector, 2*count, NULL, 0, 0};
			rc = sendmsg(fd, &msgHeader, MSG_NOSIGNAL); 
		}
	}
	else
	{
		// nothing to send
		rc = 0;
	}
	return rc;
} // apiCommandWriteOrSend

#define COMMUNICATIONSLOGHEADERSIZE 12

/* Read an ApiCommand structure from a goodwin file
 */

ApiCommand *communicationsLogApiCommandRead(int fd, destime *tim, ManetAddr *localid)
{
	unsigned char header[COMMUNICATIONSLOGHEADERSIZE], *hp;
	ApiCommand *ac;
	int rc=0;
	destime curtime;
	int nodeid;

	rc=mread(fd, header,COMMUNICATIONSLOGHEADERSIZE);

	if (rc!=COMMUNICATIONSLOGHEADERSIZE)
		return NULL;

	hp=header;
	UNMARSHALLONGLONG(hp,curtime);
	UNMARSHALLONG(hp,nodeid);
	*localid=nodeid;
	*tim=curtime;

	ac=apiCommandRead(fd,NULL);

	return ac;
}

/* This is similar to apiCommandWrite, only it writes an additional header
 * with the timestamp and IP addr, for logging api events.  (IE: goodwin files)
 */
int communicationsLogApiCommandWrite(int fd, ApiCommand *ac,destime tim, ManetAddr localid)
{
	unsigned char header[COMMUNICATIONSLOGHEADERSIZE], *hp;
	int rc=0;
	ApiCommand *next;

	while((ac) && (rc>=0))
	{
		hp=header;
		MARSHALLONGLONG(hp,tim);
		MARSHALLONG(hp,localid);

		rc=write(fd,header,COMMUNICATIONSLOGHEADERSIZE);

		if (rc!=COMMUNICATIONSLOGHEADERSIZE)
			break;

		next=ac->next;     /* save next pointer, and then NULL it, since we're looping through the AC list here, and don't want apiCommandWrite() to do so */
		ac->next=NULL;
		rc=apiCommandWriteOrSend(fd,ac,0);
		ac->next=next;

		ac=ac->next;
	}

	return rc;
}


/* Frees an entire list of ApiCommands
 */
void apiCommandFree(ApiCommand *ac)
{
	ApiCommand *nac;

	while(ac)
	{
		// fprintf(stderr,"apiCommandFree: ac at %p payload at %p len= %d\n",ac,ac->payload, ac->len);

		nac=ac->next;
		if (ac->payloadptr)
			free(ac->payloadptr);
		free(ac);
		ac=nac;
	}
}


/*********************************************************************************
 *
 * Type marshal/unmarshal functions, to convert ApiCommands into other things, and
 * back.
 */


/* The resulting MessageInfo will point into the ApiCommand
 * It will thus save a copy of the pointer to the ApiCommand, and will take care of 
 * freeing it.  THUS CALLER DOES NOT FREE the ac!
 */
MessageInfoPtr messageInfoUnmarshal(ApiCommand *ac)
{
	MessageInfo *mi;
	unsigned char *hp;
	int i;

	hp=ac->payload;

	mi=(MessageInfo*)malloc(sizeof(*mi));

	mi->next=NULL;
	mi->cs=NULL;
	mi->originApi=NULL;
	mi->chainApi=NULL;
	mi->dataId=0;
	mi->statusCallback=NULL;
        mi->statusData=NULL;
	mi->routeFlags=0;

	UNMARSHALLONG(hp,mi->origin);
	UNMARSHALLONG(hp,mi->dest.addr);
	UNMARSHALLONG(hp,i);
    if(!COMMUNICATIONSDESTINATION_IS_VALID(i))
    {
        fprintf(stderr, "%s: Got invalid CommunicationsDestinationType "
                "%d. Setting to COMMUNICATIONSDESTINATION_DIRECT\n",
                __func__, i);
        i = COMMUNICATIONSDESTINATION_DIRECT;
    }
	mi->dest.type=(CommunicationsDestinationType)i;
	UNMARSHALLONG(hp,i);
    if(i < 0 || MESSAGE_MAXTTL < i)
    {
        fprintf(stderr, "%s: Got invalid TTL of %d. Must be between "
                "0 and %d. Clamping to %d\n", __func__, i,
                MESSAGE_MAXTTL, i < 0 ? 0 : MESSAGE_MAXTTL);
        i = i < 0 ? 0 : MESSAGE_MAXTTL;
    }
	mi->dest.ttl=i;
	UNMARSHALLONG(hp,i);
	mi->type=(MessageType)i;
	UNMARSHALLONG(hp,mi->tag);
	UNMARSHALLONG(hp,mi->demonId);
	UNMARSHALLONG(hp,mi->routeFlags);

	mi->payload=hp;
	mi->payloadPtr=ac;
        mi->payloadLen=ac->len - (4+4+4+4+4+4+4+4); //  (hp - ac->payload);

	return mi;
}


/* The resulting ApiCommand will be a copy of the messageinfo struct
 *
 */

ApiCommand *messageInfoMarshal(const MessageInfo *mi)
{
	ApiCommand *ac;
	unsigned char *hp;
	int len;

	len=mi->payloadLen+(8*4);

	ac=apiCommandMalloc(APICOMMAND_MESSAGE_SEND,len);

	hp=ac->payload;
	MARSHALLONG(hp,mi->origin);
	MARSHALLONG(hp,mi->dest.addr);
	MARSHALLONG(hp,mi->dest.type);
	MARSHALLONG(hp,mi->dest.ttl);
	MARSHALLONG(hp,mi->type);
	MARSHALLONG(hp,mi->tag);
	MARSHALLONG(hp,mi->demonId);
	MARSHALLONG(hp,mi->routeFlags);
	memcpy(hp,mi->payload,mi->payloadLen);

	return ac;
}


ApiCommand *idsPositionMarshal(IDSPositionType position,IDSPositionStatus positionstatus)
{
	ApiCommand *ac;
	unsigned char *hp;

	ac=apiCommandMalloc(APICOMMAND_POSITION,4*2);
	hp=ac->payload;
	MARSHALLONG(hp,position);
	MARSHALLONG(hp,positionstatus);

	return ac;
}

void idsPositionUnmarshal(const ApiCommand *ac,IDSPositionType *position, IDSPositionStatus *positionstatus)
{
	unsigned char *hp;
	int i;

	assert(ac->type==APICOMMAND_POSITION);
	hp=ac->payload;
	UNMARSHALLONG(hp,i);
	*position=(IDSPositionType)i;
	UNMARSHALLONG(hp,i);
	*positionstatus=(IDSPositionStatus)i;
}

/* This really makes a APICOMMAND_RAWSEND, and sends the message all
 * the way through packetapi to the clustering algorithm.  
 * So, this is really simulatorland
 */
#define STATEVEC_VERSION ((unsigned char) 2)

#define TEST_BIT(f,n) (!!((f) & (0x1 << (n))))
#define SET_BIT(f,n,v)  do { (f) |= ((!!(v)) << (n)); } while (0)
#define IDSSTATE_STATE_FLAG_LOCKED_GET(f)     TEST_BIT(f,0)
#define IDSSTATE_STATE_FLAG_LOCKED_SET(f,v)   SET_BIT(f,0,v)
#define IDSSTATE_NODE_FLAG_ROOTGROUP_GET(f)   TEST_BIT(f,0)
#define IDSSTATE_NODE_FLAG_ROOTGROUP_SET(f,v) SET_BIT(f,0,v)

static size_t IDSStateElementMarshaledSize(const IDSStateElement *e)
{
	return (4      /* LONG node addr */
		+ 4    /* LONG parent addr */
		+ 1    /* BYTE nodeflags */
		+ 2    /* SHORT clusteringDataLen */
		+ e->clusteringDataLen);
}

static size_t IDSStateMarshaledSize(const IDSState *vect)
{
	size_t s = (1		/* BYTE version	 */
		    + 1		/* BYTE stateflags */
		    + 2		/* SHORT numNodes */
		);
	unsigned int i;
	for (i = 0; i < vect->numNodes; i++)
	{
		s += IDSStateElementMarshaledSize(&vect->state[i]);
	}
	return s;
}

ApiCommand *IDSStateMarshal(const IDSState *vect)
{
	ApiCommand *ac;
	MessageInfoPtr mi;
	unsigned char *payload, *hp;
	unsigned char stateflags = 0;
	size_t payloadSize = IDSStateMarshaledSize(vect);
	unsigned int i;
	CommunicationsDestination dst;
	unsigned int maxNodes = 65535; /* even this many won't fit in a packet */

	if (vect->numNodes > maxNodes)
	{
		fprintf(stderr, "%s: too many nodes (%d > %d)",
			__func__, vect->numNodes, maxNodes);
		return NULL;
	}
	payload=hp=(unsigned char*)malloc(payloadSize);
	dst.addr=NODE_LOCAL;
	dst.type=COMMUNICATIONSDESTINATION_DIRECT;
	dst.ttl=255;

	MARSHALBYTE(hp, STATEVEC_VERSION);
	IDSSTATE_STATE_FLAG_LOCKED_SET(stateflags, vect->lock);
	MARSHALBYTE(hp, stateflags);
	MARSHALSHORT(hp, vect->numNodes);

	for(i=0;i<vect->numNodes;i++)
	{
		/* adjust IDSStateElementMarshaledSize, and of course
		 * IDSStateUnmarshal(), if you change these */
		unsigned char nodeflags = 0;
		MARSHALLONG(hp,vect->state[i].node);
		MARSHALLONG(hp,vect->state[i].parent);
		IDSSTATE_NODE_FLAG_ROOTGROUP_SET(nodeflags, vect->state[i].rootgroupflag);
		/* the rest of the node flags byte is set to zero (reserved) */
		MARSHALBYTE(hp,nodeflags);
		MARSHALSHORT(hp,vect->state[i].clusteringDataLen);
		MARSHALBUFFERLONG(hp,vect->state[i].clusteringData,
				  vect->state[i].clusteringDataLen);
	}
	assert(hp == payload + payloadSize);

	mi=messageInfoCreate(vect->cs,vect->noflood?PACKET_STATEVECT_NOFLOOD:PACKET_STATEVECT,dst,NULL,NULL);
	messageInfoRawPayloadSet(mi,payload,payloadSize);   /* takes ownership of payload...  */

	ac=messageInfoMarshal(mi);
	ac->type=APICOMMAND_RAWSEND;

	messageInfoDestroy(mi);	  /* which frees payload...  */
	return	ac;
}

/* note that this takes a PACKET_STATEVECT{,_FLOOD,_NOFLOOD} packet, 
 * not an apicommand */
IDSState *IDSStateUnmarshal(const packet *p)
{
	size_t i;
	IDSState *ret;
	unsigned char version;
	unsigned char stateflags;
	unsigned char *hp = p->data;
	unsigned int numNodes;
	
	UNMARSHALBYTE(hp, version);
	if (version != STATEVEC_VERSION)
	{
		fprintf(stderr, "%s: statevec version %d not supported (expected %d)\n",
			__func__, version, STATEVEC_VERSION);
		return NULL;
	}
	UNMARSHALBYTE(hp, stateflags);
	UNMARSHALSHORT(hp, numNodes);

	if ((ret = IDSStateMalloc(NULL, numNodes)) != NULL)
	{
		ret->lock = IDSSTATE_STATE_FLAG_LOCKED_GET(stateflags);

		for (i = 0; i < numNodes; i++)
		{
			unsigned char nodeflags;
			UNMARSHALLONG(hp, ret->state[i].node);
			UNMARSHALLONG(hp, ret->state[i].parent);
			UNMARSHALBYTE(hp, nodeflags);
			ret->state[i].rootgroupflag = IDSSTATE_NODE_FLAG_ROOTGROUP_GET(nodeflags);
			/* the rest of the flags byte is ignored (reserved) */
			UNMARSHALSHORT(hp, ret->state[i].clusteringDataLen);
			if (ret->state[i].clusteringDataLen != 0)
			{
				ret->state[i].clusteringData = malloc(ret->state[i].clusteringDataLen);
				UNMARSHALBUFFERLONG(hp, ret->state[i].clusteringData, ret->state[i].clusteringDataLen);
			}
		}
		if (hp != (unsigned char*) p->data + p->len)
		{
			fprintf(stderr, "%s: error: read %u bytes but packet is %u bytes long\n",
				__func__, hp - (unsigned char*) p->data, p->len);
			IDSStateFree(ret);
			ret = NULL;
		}
	}
	return ret;
}

ApiCommand *messageHandlerRequestMarshal(
        CommunicationsMessageDirection communicationsMessageDirection, 
        unsigned int position,
        CommunicationsMessageAccess communicationsMessageAccess, 
        MessageType messageType)
{
	ApiCommand *ac;
	unsigned char *hp;

	ac=apiCommandMalloc(APICOMMAND_MESSAGETYPE_REQUEST,4*4);
	hp=ac->payload;
	MARSHALLONG(hp,communicationsMessageDirection);
	MARSHALLONG(hp,position);
	MARSHALLONG(hp,communicationsMessageAccess);
	MARSHALLONG(hp,messageType);

	return ac;
}

MessageHandlerRequest *messageHandlerRequestUnmarshal(const ApiCommand *ac)
{
	unsigned char *hp;
	int i;
	MessageHandlerRequest *mhr;

	mhr=(MessageHandlerRequest*)malloc(sizeof(*mhr));

	assert((ac->type==APICOMMAND_MESSAGETYPE_REQUEST) || (ac->type==APICOMMAND_MESSAGETYPE_REMOVE));
	hp=ac->payload;
	mhr->request=ac->type;
	UNMARSHALLONG(hp,i);
	mhr->direction=i;
	UNMARSHALLONG(hp,i);
	mhr->priority=i;
	UNMARSHALLONG(hp,i);
	mhr->access=i;
	UNMARSHALLONG(hp,i);
	mhr->type=(MessageType)i;

	return mhr;
}

ApiCommand *communicationsNeighborMarshal(const CommunicationsNeighbor *cn)
{
	ApiCommand *ac;
	unsigned char *hp;

	ac=apiCommandMalloc(APICOMMAND_NEIGHBOR,4*4);
	hp=ac->payload;
	MARSHALLONG(hp,cn->addr);
	MARSHALLONG(hp,cn->type);
	MARSHALLONG(hp,cn->state);
	MARSHALLONG(hp,cn->distance);

	return ac;
}

CommunicationsNeighbor *communicationsNeighborUnmarshal(const ApiCommand *ac)
{
	CommunicationsNeighbor *cn;
	unsigned char *hp;

	assert(ac->type==APICOMMAND_NEIGHBOR);
	cn=(CommunicationsNeighbor*)malloc(sizeof(*cn));
	hp=ac->payload;
	UNMARSHALLONG(hp,cn->addr);
	UNMARSHALLONG(hp,cn->type);
	UNMARSHALLONG(hp,cn->state);
	UNMARSHALLONG(hp,cn->distance);
	cn->next = NULL;

	return cn;
}

ApiCommand *apiInitMarshal(const ApiInit *in)
{
	ApiCommand *ac;
	unsigned char *hp;

	ac=apiCommandMalloc(APICOMMAND_INIT,4*2+1);
	hp=ac->payload;
	MARSHALLONG(hp,in->localid);
	MARSHALBYTE(hp,in->apiVersion);
	MARSHALLONG(hp,in->netmask);

	return ac;
}

ApiInit *apiInitUnmarshal(const ApiCommand *ac)
{
	ApiInit *ai;
	unsigned char *hp;

	assert(ac->type==APICOMMAND_INIT);
	ai=(ApiInit*)malloc(sizeof(*ai));
	hp=ac->payload;
	UNMARSHALLONG(hp,ai->localid);
	UNMARSHALBYTE(hp,ai->apiVersion);
	UNMARSHALLONG(hp,ai->netmask);

	return ai;
}

ApiCommand *apiNameMarshal(const ApiName *name)
{
	ApiCommand *ac;
	unsigned char *hp;
	int namelen, keylen;
	int totlen=0;
	const ApiName *i;

	for(i=name;i!=NULL;i=i->next)
	{
		namelen=strlen(i->name);
		keylen=strlen(i->key);
		if (namelen>255)
			namelen=255;
		if (keylen>65535)
			keylen=65535;

		totlen+=namelen+1+keylen+2 +4+4+4+4;
	}

	ac=apiCommandMalloc(APICOMMAND_NAMESET,totlen);
	hp=ac->payload;

	for(i=name;i!=NULL;i=i->next)
	{
		namelen=strlen(i->name);
		keylen=strlen(i->key);
		if (namelen>255)
			namelen=255;
		if (keylen>65535)
			keylen=65535;

		MARSHALBYTE(hp,namelen);
		MARSHALSHORT(hp,keylen);
		memcpy(hp,i->name,namelen);
		hp+=namelen;
		memcpy(hp,i->key,keylen);
		hp+=keylen;
		
		MARSHALLONG(hp,i->messagesSent);
		MARSHALLONG(hp,i->messagesRec);
		MARSHALLONG(hp,i->messagesUnacked);
		MARSHALLONG(hp,i->messagesAcked);
	}

	return ac;
}

/* returns a copy... does not refer to the original ApiCommand.
 */
ApiName *apiNameUnmarshal(const ApiCommand *ac)
{
	int namelen, keylen;
	ApiName *name,*list=NULL;
	unsigned char *hp=ac->payload;

	while(hp < (ac->payload + ac->len))
	{
		UNMARSHALBYTE(hp,namelen);
		UNMARSHALSHORT(hp,keylen);

		name = (ApiName*)malloc(sizeof(*name)+namelen+1+keylen+1);
		name->name=(char*)(name+1);
		name->key=name->name+namelen+1;
		name->next=list;
		list=name;

		memcpy(name->name,hp,namelen);
		name->name[namelen]=0;
		hp+=namelen;
		memcpy(name->key,hp,keylen);
		name->key[keylen]=0;
		hp+=keylen;
		
		UNMARSHALLONG(hp,name->messagesSent);
		UNMARSHALLONG(hp,name->messagesRec);
		UNMARSHALLONG(hp,name->messagesUnacked);
		UNMARSHALLONG(hp,name->messagesAcked);
	}

	return list;
}

void apiNameFree(ApiName *an)
{
	ApiName *d;
	while(an)
	{
		d=an;
		an=an->next;
		free(d);
	}
}

ApiCommand *communicationsPositionWeightMarshal(const CommunicationsPositionWeight *cpw)
{
	ApiCommand *ac;
	unsigned char *hp;

	ac=apiCommandMalloc(APICOMMAND_POSITIONWEIGHT,4*3);
	hp=ac->payload;
	MARSHALLONG(hp,cpw->addr);
	MARSHALLONG(hp,cpw->position);
	MARSHALLONG(hp,cpw->weight);

	return ac;
}

CommunicationsPositionWeight *communicationsPositionWeightUnmarshal(const ApiCommand *ac)
{
	CommunicationsPositionWeight *cpw;
	unsigned char *hp;

	assert(ac->type==APICOMMAND_POSITIONWEIGHT);
	cpw=(CommunicationsPositionWeight*)malloc(sizeof(*cpw));
	hp=ac->payload;
	UNMARSHALLONG(hp,cpw->addr);
	UNMARSHALLONG(hp,cpw->position);
	UNMARSHALLONG(hp,cpw->weight);

	return cpw;
}
/*********************************************************************************
 */

MessageTypeNode *messageTypeHandlerSearch(MessageTypeNode **list, MessageType typ)
{
	MessageTypeNode *mth;

	mth=*list;

	while(mth)
	{
		if (mth->type==typ)
			return mth;
		mth=mth->nextType;
	}
	return NULL;
}

void messageTypeHandlerDelete(MessageTypeNode **list, MessageType typ)
{
	MessageTypeNode *mth,*q;

	mth=*list;
	q=NULL;

	while(mth)
	{
		if (mth->type==typ)
			break;
		q=mth;
		mth=mth->nextType;
	}
	if (mth==NULL)               /* not found...  */
		return;

	if (q==NULL)
		(*list)=(*list)->nextType;
	else
		q->nextType=mth->nextType;

	free(mth);
}

/* This function inserts a MessageTypeNode into the "top row" of the 
 * chains data structure.  
 * only that top row is used in the API, this function is API only.
 */
MessageTypeNode *messageTypeHandlerInsert(
        MessageTypeNode **list, 
        CommunicationsMessageDirection communicationsMessageDirection, 
        unsigned int priority, 
        CommunicationsMessageAccess communicationsMessageAccess, 
        MessageType messageType, 
        MessageHandler messageHandlerFunc, 
        void *messageHandlerData,
        int replace)
{
	MessageTypeNode *mth;

	mth=messageTypeHandlerSearch(list,messageType);                /* otherwise insert/replace  */
	if (mth==NULL)
	{
		mth=(MessageTypeNode*)malloc(sizeof(*mth));
		mth->nextType=(*list);
		*list=mth;
	}
        else if (!replace)
        {
		return NULL;
        }
	mth->direction=communicationsMessageDirection;
	mth->priority=priority;
	mth->access=communicationsMessageAccess;
	mth->type=messageType;
	mth->handler=messageHandlerFunc;
	mth->handlerData=messageHandlerData;

	return mth;
}

/*********************************************************************************
 *
 * Calls to handle a neighbor list
 *
 * Note that primary key of the neighbor list is address /and/ type.  IE: an individual node
 * may have more than one entry on the list, if for example it is a 1 hop neighbor, and a parent.
 *
 * Also node, this takes ownership of the CommunicationsNeighbor passed into it.  (do not free it!)
 *
 */

void communicationsIntNeighborFlush(CommunicationsNeighbor **list)
{
	CommunicationsNeighbor *p,*q;

	p=*list;
	while(p)
	{
		q=p->next;

		free(p);
			
		p=q;
	}
	*list=NULL;
}

void communicationsIntNeighborUpdate(CommunicationsNeighbor **list, CommunicationsNeighbor *n)
{
	CommunicationsNeighbor *p,*q;
	switch(n->state)
	{	
		case COMMUNICATIONSNEIGHBOR_ARRIVING:
		case COMMUNICATIONSNEIGHBOR_UPDATING:
			p=communicationsIntNeighborSearch(list,n->addr);
			if (p)
			{
				p->type=n->type;
				p->distance=n->distance;
				free(n);
			}
			else
			{
				n->next=*list;
				*list=n;
			}
		break;
		case COMMUNICATIONSNEIGHBOR_DEPARTING:
			p=*list;
			q=NULL;

			while(p)
			{
				if ((p->addr==n->addr))
					break;
				else
				{
					q=p;
					p=p->next;
				}
			}

			if (p)
			{
				if (q)
					q->next=p->next;
				else
					*list=p->next;
				free(p);
			}
			if (n!=p)
				free(n);
		break;
		default:
		break;
	}
}

/* Given an address, return the entry in the neighbor list for that address, or NULL if it is not there
 */
CommunicationsNeighbor *communicationsIntNeighborSearch(CommunicationsNeighbor **neighborlist, ManetAddr addr)
{
	CommunicationsNeighbor *cn;

	for(cn=(*neighborlist);cn!=NULL;cn=cn->next)
		if (cn->addr==addr)
			return cn;

	return NULL;
}

/* Allocate a new neighbor structure for the neighbor list.  Used for unmarshaling.  
 *
 * To put the entry on the neighbor list, use communicationsIntNeighborUpdate.
 */
CommunicationsNeighbor *communicationsIntNeighborMalloc(ManetAddr addr,CommunicationsNeighborType type, int distance)
{
	CommunicationsNeighbor *cn;

	cn=(CommunicationsNeighbor*)malloc(sizeof(*cn));

	cn->addr=addr;
	cn->type=type;
	cn->distance=distance;
	cn->state=COMMUNICATIONSNEIGHBOR_ARRIVING;
	cn->next=NULL;
	return cn;
}

ApiCommand *apiStatusMarshal(const ApiStatus *as)
{
	ApiCommand *ac;
	unsigned char *hp;
	int numtypes=0;
	int i;

	for(i=0;i<as->numtypes;i++)
		if ((as->packetList[i].unicastRecCount>0)  ||
		    (as->packetList[i].repUnicastXmitCount>0) ||
		    (as->packetList[i].origUnicastXmitCount>0) ||
		    (as->packetList[i].bcastRecCount>0)  ||
		    (as->packetList[i].repBcastXmitCount>0) ||
		    (as->packetList[i].repBcastXmitCount>0))
			numtypes++;

	ac=apiCommandMalloc(APICOMMAND_STATUS,4*3 + 1 + 8 + (numtypes * (1 + 8 * 8) ));

	hp=ac->payload;

	MARSHALBYTE(hp,numtypes);
	MARSHALLONG(hp,as->period);
	MARSHALLONG(hp,as->level);
	MARSHALLONG(hp,as->rootflag);
	MARSHALLONGLONG(hp,as->timestamp);

	for(i=0;i<as->numtypes;i++)
		if ((as->packetList[i].unicastRecCount>0)  ||
		    (as->packetList[i].repUnicastXmitCount>0) ||
		    (as->packetList[i].origUnicastXmitCount>0) ||
		    (as->packetList[i].bcastRecCount>0)  ||
		    (as->packetList[i].repBcastXmitCount>0) ||
		    (as->packetList[i].repBcastXmitCount>0))
		    {
			MARSHALBYTE(hp,as->packetList[i].type);
			MARSHALLONGLONG(hp,as->packetList[i].unicastRecCount);
			MARSHALLONGLONG(hp,as->packetList[i].repUnicastXmitCount + as->packetList[i].origUnicastXmitCount);
			MARSHALLONGLONG(hp,as->packetList[i].unicastRecByte);
			MARSHALLONGLONG(hp,as->packetList[i].repUnicastXmitByte + as->packetList[i].origUnicastXmitByte);
			MARSHALLONGLONG(hp,as->packetList[i].bcastRecCount);
			MARSHALLONGLONG(hp,as->packetList[i].repBcastXmitCount + as->packetList[i].origBcastXmitCount);
			MARSHALLONGLONG(hp,as->packetList[i].bcastRecByte);
			MARSHALLONGLONG(hp,as->packetList[i].repBcastXmitByte + as->packetList[i].origBcastXmitByte);
		    }
		
	return ac;
}

ApiStatus *apiStatusUnmarshal(const ApiCommand *ac)
{
	unsigned char *hp;
	ApiStatus *as;
	int numtypes;
	int i;

	assert(ac->type==APICOMMAND_STATUS);

	hp=ac->payload;
	UNMARSHALBYTE(hp,numtypes);

	as=(ApiStatus*)calloc(1,sizeof(*as) + ( sizeof(as->packetList[0]) * numtypes ));
	as->packetList=(ApiPacketCount*)((unsigned char *)as + sizeof(*as));

	as->numtypes=numtypes;
	UNMARSHALLONG(hp,as->period);
	UNMARSHALLONG(hp,as->level);
	UNMARSHALLONG(hp,as->rootflag);
	UNMARSHALLONGLONG(hp,as->timestamp);

	for(i=0;i<numtypes;i++)
	{
		UNMARSHALBYTE(hp,as->packetList[i].type);
		UNMARSHALLONGLONG(hp,as->packetList[i].unicastRecCount);
		UNMARSHALLONGLONG(hp,as->packetList[i].repUnicastXmitCount);
		as->packetList[i].origUnicastXmitCount=as->packetList[i].repUnicastXmitCount;
		UNMARSHALLONGLONG(hp,as->packetList[i].unicastRecByte);
		UNMARSHALLONGLONG(hp,as->packetList[i].repUnicastXmitByte);
		as->packetList[i].origUnicastXmitByte=as->packetList[i].repUnicastXmitByte;
		UNMARSHALLONGLONG(hp,as->packetList[i].bcastRecCount);
		UNMARSHALLONGLONG(hp,as->packetList[i].repBcastXmitCount);
		as->packetList[i].origBcastXmitCount=as->packetList[i].repBcastXmitCount;
		UNMARSHALLONGLONG(hp,as->packetList[i].bcastRecByte);
		UNMARSHALLONGLONG(hp,as->packetList[i].repBcastXmitByte);
		as->packetList[i].origBcastXmitByte=as->packetList[i].repBcastXmitByte;
	}

	return as;
}

/* Add (or update) an entry for an IDSPosition in a list of IDSPositions.
 *
 * In the API, the client mantains a list of its positions, and callback addrs.
 * In the demon, each client has a list of its positions (though the callback addrs are blank
 */
IDSPosition *idsPositionInsert(IDSPosition **list, IDSPositionType position, IDSPositionStatus eligibility, IDSPositionUpdateProc update, void *data)
{
	IDSPosition *rn;

	rn=idsPositionSearch(list,position);		/* If we already have an entry for this position, replace its values */
	if (rn==NULL)
	{
		rn=(IDSPosition*)malloc(sizeof(*rn));
		rn->next=(*list);
		*list=rn;
		rn->position=position;
		rn->status=IDSPOSITION_UNDEFINED;
	}
	rn->eligibility=eligibility;
	rn->update=update;
	rn->updateData=data;

	return rn;
}

void idsPositionDelete(IDSPosition **list, IDSPositionType position)
{
	IDSPosition *rn,*q;

	rn=*list;
	q=NULL;

	while(rn)
	{
		if (rn->position==position)
			break;
		q=rn;
		rn=rn->next;
	}
	if (rn==NULL)               /* not found...  */
		return;

	if (q==NULL)
		(*list)=(*list)->next;
	else
		q->next=rn->next;

	free(rn);
}

IDSPosition *idsPositionSearch(IDSPosition **list,IDSPositionType position)
{
	IDSPosition *rn;

	rn=*list;

	while(rn)
	{
		if (rn->position==position)
			return rn;
		rn=rn->next;
	}
	return NULL;
}


void communicationsClose(CommunicationsStatePtr cs)
{
	IDSPosition *ppn,*dpn;
	MessageInfo *pmi,*dmi;
	MessageTypeNode *pmt,*dmt;
	CommunicationsNeighbor *pcn,*dcn;

	if (cs->fd >= 0) close(cs->fd);
	cs->fd=-1;

	ppn=cs->positionList;
	while(ppn)
	{
		dpn=ppn;
		ppn=ppn->next;

		/* call callback and say inactive.  
		 * send ineligible message to demon?   (no, let demon cleanup on close, then we implement that once instead of twice)
		 */
		free(dpn);
	}
	cs->positionList = NULL;

	pmi=cs->inflight;
	while(pmi)
	{
		dmi=pmi;
		pmi=pmi->next;

		/* don't call any status callbacks, just destroy the messageInfo */
                messageInfoDestroy(dmi);
	}
	cs->inflight=NULL;

	pmt=cs->typehandler;
	while(pmt)
	{
		dmt=pmt;
		pmt=pmt->nextType;

		free(dmt);
	}
	cs->typehandler=NULL;

	pcn=cs->neighborList;
	while(pcn)
	{
		dcn=pcn;
		pcn=pcn->next;

		free(dcn);
	}
	cs->neighborList=NULL;

	while(cs->positionWeightList)
	{
		CommunicationsPositionWeight *tmp;
		tmp=cs->positionWeightList;
		cs->positionWeightList=cs->positionWeightList->next;
		free(tmp);
	}
	cs->positionWeightList=NULL;

	if (cs->name)
	{
		free(cs->name);
		cs->name=NULL;
	}

	/* If we're reading from a log, the CommunicationsLogState structure owns cs,
	 * so a close should only reset (unregister callbacks), and we'll wait until
	 * the CommunicationsLogClose() to actually free cs. */
	if (cs->readLog == NULL) 
	{
		free(cs);
	}

}


/*
 * Code to mantain the demon's list of node weights.
 */


/* Insert a new CommunicationsPositionWeight into the list.
 *
 * This does a copy.  caller keeps ownership of *entry
 */
CommunicationsPositionWeight *communicationsPositionWeightInsert(CommunicationsPositionWeight **list,const CommunicationsPositionWeight *entry)
{
	CommunicationsPositionWeight *wt;

	assert(list != NULL);

	wt=communicationsPositionWeightSearchList(*list,entry);

	if (wt==NULL)
	{
		wt=(CommunicationsPositionWeight*)malloc(sizeof(*wt));
		wt->addr=entry->addr;
		wt->position=entry->position;

		wt->next=*list;
		*list=wt;
	}

#if 0
	if (entry->weight < wt->weight)      /* if there is already a value in the list, use that which */
#endif
		wt->weight=entry->weight;    /* bans the node more  */

	return wt;
}

/* Delete an entry from the list
 * Caller keeps ownership of *entry
 */
void communicationsPositionWeightRemove(CommunicationsPositionWeight **list,const CommunicationsPositionWeight *entry)
{
        CommunicationsPositionWeight *wt,*pwt;

	pwt=NULL;
	wt=*list;
	while(wt)
	{
		if ((wt->addr==entry->addr) && (wt->position==entry->position))
			break;
		pwt=wt;
		wt=wt->next;
	}

	if (wt)     /* If we found it on the list...   */
	{
		if (pwt)                          /* linked list remove */
			pwt->next=wt->next;
		else
			*list=wt->next;

		free(wt);
	}
}


/* attempt to load a position weight configuration file.
 * the file name is in the config variable positionweightfile,
 * and has the format:
 * ipaddr [root|neighborhood|region] weight|banned|assigned
 *
 * the ipaddr is the one specified in the locations file, or if
 * a locations file was not used, the index number.
 */
void communicationsPositionWeightLoad(char const *weightfile,CommunicationsPositionWeight **list)
{
	FILE *fil;
	char line[8192];
	char *p,*inp;
	CommunicationsPositionWeight cpw;

	if (weightfile==NULL)
		return;

	fil=fopen(weightfile,"r");

	if (fil==NULL)
	{
		fprintf(stderr,"communicationsPositionWeightLoad: coult not open file %s\n",weightfile);
		abort();
		return;
	}

	cpw.next=NULL;

	while(fgets(line,sizeof(line),fil))
	{
		if ((line[0]=='#') || (line[0]=='\r') || (line[0]=='\n'))
			continue;


		inp=line;
		p=strsep(&inp," \t");

		cpw.addr=ntohl(inet_addr(p));
fprintf(stderr, "posw8addr: %s\n", p); 

		p=strsep(&inp," \t");
fprintf(stderr, "posw8position: %s\n", p); 

		if (strcasecmp(p,"root")==0)
			cpw.position=COORDINATOR_ROOT;
		else
		if (strcasecmp(p,"neighborhood")==0)
			cpw.position=COORDINATOR_NEIGHBORHOOD;
		else
		if (strcasecmp(p,"region")==0)
			cpw.position=COORDINATOR_REGIONAL;
		else
		if (strcasecmp(p,"rootgroup")==0)
			cpw.position=COORDINATOR_ROOTGROUP;
		else
		{
			fprintf(stderr,"illegal position in position weight file!\n");
			exit(1);
		}

		p=strsep(&inp," \t");
fprintf(stderr, "posw8weight: %s\n", p); 

		if (strncasecmp(p,"banned",6)==0)
			cpw.weight=COMMUNICATIONSPOSITIONWEIGHT_BANNED;
		else
		if (strncasecmp(p,"assigned",8)==0)
			cpw.weight=COMMUNICATIONSPOSITIONWEIGHT_ASSIGNED;
		else
			sscanf(p,"%d",&cpw.weight);

		communicationsPositionWeightInsert(list,&cpw);
	}

	fclose(fil);
}

/* Marshal a floating label into an existing message, and return ptr so that
 * more things can be marshaled after it
 */
unsigned char *communicationsWatcherFloatingLabelMarshal(unsigned char *hp, const FloatingLabel *lab)
{
	int i;

	MARSHALLONG(hp,lab->x);
	MARSHALLONG(hp,lab->y);
	MARSHALLONG(hp,lab->z);

	for(i=0;i<4;i++)
		MARSHALBYTE(hp,lab->fgcolor[i]);
	for(i=0;i<4;i++)
		MARSHALBYTE(hp,lab->bgcolor[i]);
	MARSHALBYTE(hp,lab->family);
	MARSHALBYTE(hp,lab->priority);
	MARSHALLONG(hp,lab->tag);

	MARSHALSTRINGSHORT(hp,lab->text);
	MARSHALLONG(hp,lab->expiration);

	return hp;
}

/* Unmarshal a single label into an existing label structure.  Assumes that the label already has some
 * memory to put the text into.
 */
unsigned char *communicationsWatcherFloatingLabelUnmarshal(unsigned char *hp, FloatingLabel *lab)
{
	int i;

	UNMARSHALLONG(hp,lab->x);
	UNMARSHALLONG(hp,lab->y);
	UNMARSHALLONG(hp,lab->z);

	for(i=0;i<4;i++)
		UNMARSHALBYTE(hp,lab->fgcolor[i]);
	for(i=0;i<4;i++)
		UNMARSHALBYTE(hp,lab->bgcolor[i]);
	UNMARSHALBYTE(hp,lab->family);
	UNMARSHALBYTE(hp,lab->priority);
	UNMARSHALLONG(hp,lab->tag);

	UNMARSHALSTRINGSHORT(hp,lab->text);

	UNMARSHALLONG(hp,lab->expiration);

	return hp;
}

/* Marshal a single label into an existing message, and return
 * ptr so that more things can be marshaled after it
 */
unsigned char *communicationsWatcherLabelMarshal(unsigned char *hp, const NodeLabel *lab)
{
	int i;

	MARSHALLONG(hp,lab->node);
	for(i=0;i<4;i++)
		MARSHALBYTE(hp,lab->fgcolor[i]);
	for(i=0;i<4;i++)
		MARSHALBYTE(hp,lab->bgcolor[i]);
	MARSHALBYTE(hp,lab->family);
	MARSHALBYTE(hp,lab->priority);
	MARSHALLONG(hp,lab->tag);

	MARSHALSTRINGSHORT(hp,lab->text);
	MARSHALLONG(hp,lab->expiration);

	return hp;
}

/* Unmarshal a single label into an existing label structure.  Assumes that the label already has some
 * memory to put the text into.
 */
unsigned char *communicationsWatcherLabelUnmarshal(unsigned char *hp, NodeLabel *lab)
{
	int i;

	UNMARSHALLONG(hp,lab->node);
	for(i=0;i<4;i++)
		UNMARSHALBYTE(hp,lab->fgcolor[i]);
	for(i=0;i<4;i++)
		UNMARSHALBYTE(hp,lab->bgcolor[i]);
	UNMARSHALBYTE(hp,lab->family);
	UNMARSHALBYTE(hp,lab->priority);
	UNMARSHALLONG(hp,lab->tag);

	UNMARSHALSTRINGSHORT(hp,lab->text);

	UNMARSHALLONG(hp,lab->expiration);

	return hp;
}

unsigned char *communicationsWatcherPropertyMarshal(unsigned char *hp, WatcherPropertyInfo *props)
{
    int i;
    float f;
    MARSHALLONG(hp, props->identifier);
    MARSHALLONG(hp, props->property);
    switch(props->property)
    {
        case WATCHER_PROPERTY_SHAPE:
            MARSHALLONG(hp, props->data.shape);
            break;
        case WATCHER_PROPERTY_COLOR:
            for(i=0;i<4;i++) 
                MARSHALBYTE(hp,props->data.color[i]);
            break;
        case WATCHER_PROPERTY_EFFECT:
            MARSHALLONG(hp, props->data.effect);
            break;
        case WATCHER_PROPERTY_SIZE:
            f=props->data.size*1000000;
            MARSHALLONG(hp, (long)f); 
            break;
    }
    return hp;
}

unsigned char *communicationsWatcherPropertyUnmarshal(unsigned char *hp, WatcherPropertyInfo *props)
{
    int i;
    long l;
    UNMARSHALLONG(hp, props->identifier); 
    UNMARSHALLONG(hp, props->property);
    switch(props->property)
    {
        case WATCHER_PROPERTY_SHAPE:
            UNMARSHALLONG(hp, props->data.shape);
            break;
        case WATCHER_PROPERTY_COLOR:
            for(i=0;i<4;i++) 
                UNMARSHALBYTE(hp,props->data.color[i]);
            break;
        case WATCHER_PROPERTY_EFFECT:
            UNMARSHALLONG(hp, props->data.effect);
            break;
        case WATCHER_PROPERTY_SIZE:
            UNMARSHALLONG(hp, l);
            props->data.size=l/1000000;
            break;
    }
    return hp;
}

/* Marshal a color message
 */
unsigned char *watcherColorMarshal(unsigned char *hp, ManetAddr node, const unsigned char *color)
{
	int i;

	MARSHALLONG(hp,node);
	if (color)
	{
		MARSHALBYTE(hp,1);
		for(i=0;i<4;i++)
			MARSHALBYTE(hp,color[i]);
	}
	else
		MARSHALBYTE(hp,0);
	return hp;
}

/* Unmarshal a color message
 */
unsigned char *watcherColorUnMarshal(unsigned char *hp, ManetAddr *node, unsigned char *color)
{
	int colorflag;
	int i;

	UNMARSHALLONG(hp,*node);

	UNMARSHALBYTE(hp,colorflag);
	if (colorflag)
	{
		for(i=0;i<4;i++)
			UNMARSHALBYTE(hp,color[i]);
	}
	else
		memset(color,0,4);

	return hp;
}


#include "idmef-message.dtd.c"   /* init compression dictionary, using the idmef DTD with most of the comments removed */

/* Table of packet types and the dictionary to use.
 * Lookups on the table are done with communicationsMessageTypeSearch() which
 * is currently a linear search.  (worry about changing to binary search or
 * similar when we get 10 or so entries...)
 */
static CommunicationsMessageType messageTypeList[]=
{
/* obsolete: {IDSCOMMUNICATIONS_MESSAGE_IDMEF,(void*)dictionary,sizeof(dictionary)-1}, */
{IDSCOMMUNICATIONS_MESSAGE_IDMEF_ALERT,(void*)dictionary,sizeof(dictionary)-1},
{IDSCOMMUNICATIONS_MESSAGE_IDMEF_HEARTBEAT,(void*)dictionary,sizeof(dictionary)-1},
{0,0,0}
};
#undef dictionary

CommunicationsMessageType *communicationsMessageTypeSearch(MessageType type)
{
	int i=0;
	while(messageTypeList[i].type!=0)
	{
		if (messageTypeList[i].type==type)
		return &messageTypeList[i];
		i++;
	}
	return NULL;
}

/* returns (ManetAddr)0 on failure */
ManetAddr communicationsHostnameLookup(const char *nam)
{
	ManetAddr maddr;
	struct hostent *hst;

	/* This is redundant, gethostbyname will do it too, but we do it
	 * anyway, so we can conditionally compile the gethostbyname code
	 * away.
	 */
#if 0
	struct in_addr addr;
	if (inet_aton(nam, &addr)!=0)
		return ntohl(addr.s_addr);
#else
	if ((maddr=inet_addr(nam))!=INADDR_NONE)
		return ntohl(maddr);
#endif

	if ((hst=gethostbyname(nam))!=NULL)
	{
		switch(hst->h_addrtype)
		{
			case AF_INET:
			{
			 	ManetAddr addr;
				memcpy(&addr, hst->h_addr_list[0], sizeof(addr));
				return ntohl(addr);
				break;
			}
			default:
				return 0;
		}
	}

//	addr=ntohl(inet_addr(argv[1]));

	return 0;
}
