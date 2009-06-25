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
#include <fcntl.h>
#include <limits.h>

#include <zlib.h>

#include "config.h"
#include "des.h"
#include "routing.h"
#include "data.h"
#include "node.h"
#include "flood.h"

#include "apisupport.h"
#include "marshal.h"

#ifdef USE_PACKETPROTECTION
#include "packetProtection.h"
#endif 

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: livenetwork.cpp,v 1.113 2007/07/18 15:35:16 mheyman Exp $";

// #define DEBUG_SELECT

/* This is in the simulation module.  Dunno which one we're actually linking to though.  (nifty eh? :-)
 * (These prototypes should be somewhere else)
 */
void nodeInit(manetNode *us);

/* code which replaces most of main.[ch], and executes code written
 * to the simulator interface on real networks.
 */

#define MAXINTERFACE 16

/* This is all the demons' state information.
 * There will be exactly one of these.
 */
typedef struct
{
	int udpport;		/* UDP port demon is listening on for MANET traffic  */
	int udpfd;		/* FD for incoming unicast packets */
	manet *themanet;	/* stub datastructure to make the simulator-API code happy */

	ManetAddr localaddr;	/* our manet address, for putting in the src field for outgoing packets */
	ManetAddr localmask;	/* manet network netmask  */
	ManetAddr localbcast[MAXINTERFACE];	/* manet address, for putting in the src field for outgoing broadcast packets */
	ManetAddr localbcastmask[MAXINTERFACE];
	int numbcast;
	int promisc;		/* accept manet packets from any address */

	Config *conf;

	int exitFlag;

} State;

MessageTypeNode *messageNextChainEnqueue(MessageTypeNode *mt, MessageInfo *mi);

static int realPacketSend(State *st, packet *p);
static packet *realPacketRec(int fd, State *st);
static int selectLoop(State *st);
static State *stateInit(Config *conf);

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

/* Sends an error log message to stderr.
 * Like "printf()" but to "stderr" and no return value.
 */
static void elog(char const *format, ...)  __attribute__((unused));
static void elog(char const *format, ...)
{
	va_list arglist;
	va_start(arglist, format);    
	vfprintf(stderr, format, arglist);
	va_end(arglist);
	return;
} /* elog */


/* stateInit - create an instance of an infrastructure demon.
 *
 */

static State *stateInit(Config *conf)
{
	State *st;
	struct sockaddr_in incoming;
	struct ifconf       ifc; 
	struct ifreq        ifreq, *ifr;
	char buff[1024];
	int rc,i;
	char const *manetnetworkconf;
	char const *manetaddrconf;
	ManetAddr manetaddr;
	unsigned int manetnetwork[MAXINTERFACE];
	int numnetwork=0;
	int foundflag=0;
	ManetAddr firstaddr=NODE_LOCAL,firstmask=0,firstbcast=0;

	st=(State*)malloc(sizeof(*st));

	st->conf=conf;

	if (st->conf==NULL)
	{
		free(st);
		return NULL;
	}

	st->numbcast=0;
	st->promisc=configSetInt(st->conf,"manetpromisc",0);
	st->udpport=configSetInt(st->conf,"udpport",API_DEFAULTPORT);

	st->udpfd=socket(AF_INET, SOCK_DGRAM, 0);
	if (st->udpfd>=0)
	{
		int optval = 1;
		if ((rc=setsockopt(st->udpfd,SOL_SOCKET,SO_BROADCAST,&optval,sizeof(optval)))<0)
		{
			fprintf(stderr,"stateinit: setsockopt: rc= %d errno= %d\n",rc,errno);
			close(st->udpfd);
			free(st);
			return NULL;
		}
	}
	else
	{
		fprintf(stderr,"stateInit: udpfd socket failed\n");
		free(st);
		return NULL;
	}
	i=1;
	if ((rc=setsockopt(st->udpfd,SOL_SOCKET,SO_BROADCAST,&i,sizeof(i)))<0)
	{
		fprintf(stderr,
			"stateInit: apiacceptfd socket failed. \"%s\"(%d)\n",
			strerror(errno), errno);
		close(st->udpfd);
		free(st);
		return NULL;
	}
	if ((rc=fcntl(st->udpfd,F_SETFL,O_NONBLOCK))<0)
	{
		fprintf(stderr,
			"stateInit: fcntl set O_NONBLOCK on socket failed. \"%s\"(%d)\n",
			strerror(errno), errno);
		close(st->udpfd);
		free(st);
		return NULL;
	}

	ifc.ifc_len=sizeof(buff);
	ifc.ifc_buf=buff;
	manetaddrconf=configSearchStr(st->conf,"manetaddr");
	if (manetaddrconf)
	{
		manetaddr=ntohl(inet_addr(manetaddrconf));
		fprintf(stderr,"manetaddr:    %d.%d.%d.%d\n",PRINTADDR(manetaddr));
	}
	else
	{
		manetaddr=0;
	}
	manetnetworkconf=configSearchStr(st->conf,"manetnetwork");
	if (manetnetworkconf!=NULL)
	{
		char const *p=manetnetworkconf;

		fprintf(stderr,"manetnetwork: %s\n",p ? p : "<not set>");

		while(p && *p)
		{
			while((*p) && !isdigit(*p))
			p++;
			if(*p)
			{
				char addrbuf[16];
				char *comma;
				strncpy(addrbuf, p, sizeof(addrbuf) - 1);
				addrbuf[sizeof(addrbuf) - 1] = 0;
				comma = strchr(addrbuf, ',');
				if(comma) *comma = 0;
				manetnetwork[numnetwork]=ntohl(inet_addr(addrbuf));
				p=strchr(p, ',');
				fprintf(stderr,"network %d.%d.%d.%d\n",PRINTADDR(manetnetwork[numnetwork]));
				numnetwork++;
			}
		}
	}

	if (ioctl(st->udpfd, SIOCGIFCONF, (char *)&ifc)>=0)
	{
#if defined(__linux__) || defined(__CYGWIN__)
		/* no sa_len */
		for(ifr=ifc.ifc_req;ifr<(struct ifreq *)((char*)ifc.ifc_req+ifc.ifc_len);ifr=(struct ifreq *)(((char*)ifr)+(sizeof(*ifr))))
#else
		for(ifr=ifc.ifc_req;ifr<(struct ifreq *)((char*)ifc.ifc_req+ifc.ifc_len);ifr=(struct ifreq *)(((char*)ifr)+(ifr->ifr_addr.sa_len + IFNAMSIZ)))
#endif
		{
			ifreq = *ifr;
			ManetAddr curaddr,curbcast,curmask;

#ifdef DEBUG_IO
			fprintf(stderr,"interface %s\n",ifr->ifr_name);
#endif

			if (ioctl(st->udpfd, SIOCGIFFLAGS, (char *)&ifreq) < 0) 
				continue;

			if (((ifreq.ifr_flags & IFF_BROADCAST)) && (ifreq.ifr_flags & IFF_UP))
			{
				if (ioctl(st->udpfd, SIOCGIFADDR, (char *)&ifreq) < 0)
					continue;
				curaddr = ntohl(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr);
				if ( ioctl(st->udpfd, SIOCGIFBRDADDR, &ifreq) < 0  )
					continue;
				curbcast = ntohl(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr);
				if ( ioctl(st->udpfd, SIOCGIFNETMASK, &ifreq) < 0  )
					continue;
				curmask = ntohl(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr);

				fprintf(stderr,"interface %-5s addr %d.%d.%d.%d bcast %d.%d.%d.%d mask %d.%d.%d.%d\n",ifr->ifr_name,PRINTADDR(curaddr),PRINTADDR(curbcast),PRINTADDR(curmask));

				if (firstaddr==NODE_LOCAL)
				{
					firstaddr=curaddr;
					firstmask=curmask;
					firstbcast=curbcast;
				}
				if ((manetaddrconf) && (curaddr==manetaddr))
				{
					foundflag=1;
					st->localaddr=curaddr;
					st->localmask=curmask;
					st->localbcast[st->numbcast]=curbcast;
					st->localbcastmask[st->numbcast]=curmask;
					st->numbcast++;
				} else {
					for(i=0;i<numnetwork;i++)
					{
						if ((curaddr & curmask) == manetnetwork[i])
						{
							int j;
							if (i==0 && !manetaddrconf)
							{
								foundflag=1;
								st->localaddr=curaddr;
								st->localmask=curmask;
							}
							for (j=0;j<st->numbcast;j++)
							{
								if (st->localbcast[j] == curbcast)
								{
									/* this situation is almost certainly bad - broadcasts may go to just one of the interfaces */
									fprintf(stderr,"Error: Multiple interfaces have the same broadcast address (%d.%d.%d.%d).  Exiting.\n",
										PRINTADDR(curbcast));
									exit(1);
								}
							}
							st->localbcast[st->numbcast]=curbcast;
							st->localbcastmask[st->numbcast]=curmask;
							st->numbcast++;
						}
					}
				}
			}
		}
	}

	if (!foundflag)
	{
#if 0
		fprintf(stderr,"None of the interfaces is on the specified subnet.  Using the first interface reported by ioctl(SIOCGIFCONF).\n");
		st->localaddr=firstaddr;
		st->localmask=firstmask;
		st->localbcast[0]=firstbcast;
		st->localbcastmask[0]=firstmask;
		st->numbcast=1;
#else
		if (manetaddrconf)
		{
			fprintf(stderr,"None of the interfaces has address %d.%d.%d.%d (manetaddr).  Exiting.\n",
				PRINTADDR(manetaddr));
		}
		else if (numnetwork > 0)
		{
			fprintf(stderr,"None of the interfaces is on subnet %d.%d.%d.%d (manetnetwork).  Exiting.\n",
				PRINTADDR(manetnetwork[0]));
		}
		else
		{
			fprintf(stderr,"Neither manetaddr nor manetnetwork was specified in the configuration.  Exiting.\n");
		}
		exit(1);
#endif
	}

#if 1
	fprintf(stderr,"local address:       %d.%d.%d.%-3d  %d.%d.%d.%d \n",PRINTADDR(st->localaddr),PRINTADDR(st->localmask));
	for(i=0;i<st->numbcast;i++)
		fprintf(stderr,"broadcast address %d: %d.%d.%d.%-3d  %d.%d.%d.%d \n",i,PRINTADDR(st->localbcast[i]),PRINTADDR(st->localbcastmask[i]));
#endif

	memset(&incoming,0,sizeof(incoming));

	incoming.sin_family = AF_INET;
	incoming.sin_addr.s_addr = htonl(INADDR_ANY);
	incoming.sin_port = htons(st->udpport);

	rc = bind (st->udpfd, (struct sockaddr *) &incoming,sizeof(incoming));
	if (rc<0)
	{
		fprintf(stderr,"stateInit: udpfd bind failed\n");
		close(st->udpfd);
		free(st);
		return NULL;
	}

	st->exitFlag=0;

	st->themanet=manetInit(st->conf,getMilliTime());

#ifdef USE_PACKETPROTECTION
        {
		const char *keyFile = configSearchStr(conf, "keyfile");
		char fullpath[PATH_MAX];
		
		if (keyFile)
		{
			if (0 == configGetPathname(conf,
						   keyFile,
						   fullpath,
						   sizeof(fullpath)))
			{
				keyFile = fullpath;
			}
			packetProtectionInit(&(st->themanet->packetProtection), 
					     keyFile);
			fprintf(stderr,"cryptographic packet source authentication enabled (keyfile \"%s\")\n",
				keyFile);
		}
		else
		{
			fprintf(stderr,"cryptographic packet source authentication disabled (no keyfile specified)\n");
		}
        }
#endif

	st->themanet->numnodes=1;
	st->themanet->nlist[0].addr=st->localaddr;
	st->themanet->nlist[0].bcastaddr=st->localbcast[0];
	st->themanet->nlist[0].netmask=st->localmask;

	firstStep(st->themanet,1);

	return st;
}



/*********************************************************************************************************
 *
 * Functions to transmit and receive packets on the wire
 *
 * Note that on-the-wire MANET packets have several extra fields, including a duplicate src address
 * (the original sender, as opposed to the link-layer sender(repeater)), an extra hopcount field,
 * an extra TTL field, and extra destination fields for the multicast addressing modes.  They are
 * mostly to keep things in userland where they are easy to rewrite.
 *
 */


/* This is part of the simulator API.  
 *
 * enqueue a packet from simulator land in the event list (which functions as both the outgoing packet
 * queue, and the rereceived (sortof incoming) packet queue
 * 
 */
void packetSend(manetNode *us, packet *p, int origflag)
{
#ifdef DEBUG_IO
	fprintf(stderr,"node %d: packetSend: sending to %d p= %p\n",us->addr & 0xFF, p->dst & 0xFF, p);
#endif
	assert(p!=NULL);
	packetEnqueue(&us->manet->nlist[0],p,0);   /* send packet to us->manet->nlist[i] */
}


/* This is the size of the header of extra fields prepended onto the payload
 */
#define HEADERSIZE 8

/* This is the maximum size of the packet payload (not including the HEADERSIZE bytes)
 * that the receiving function is prepared to handle.  If it was 64K, we could handle
 * anthing that UDP would hand us, at the cost of a great deal of ram.  That decision
 * may be a false optimization.  XXX
 */
#define MAXPACKETSIZE 65535

/* This takes a simulator packet structure, and writes it onto the local ethernet
 * It is called by the livenetwork code, to provide the simulator API.
 */

#define BROADCAST_BIT 0x80
#define CRYPT_BIT 0x40
#define VERSION_MASK 0x3F

static int realPacketSend(State *st, packet *p)
{
	struct sockaddr_in dest;
	struct msghdr mheader;
	struct iovec gather[2];
	unsigned char header[HEADERSIZE];
	int ttl;
	int rc;
#ifdef USE_PACKETPROTECTION
	packet *tmp;
	int cryptflag = 0;
#endif
	manetNode *us=&(st->themanet->nlist[0]);

	if (us->manet->packetProtection)
	{
#ifdef USE_PACKETPROTECTION
		if(packetProtect(us->manet->packetProtection, &tmp, us->addr, p) == 0)
		{
			if(tmp)
			{
				// why is this commented out? -dkindred
				// packetFree(p);
				p=tmp;
				cryptflag=1;
			}
		}
		else
		{
			fprintf(stderr,"node %d: %s: Failed protect\n", us->addr & 0xFF, __func__);
			packetFree(p);
			errno=0;
			return -1;
		}
#endif
	}


	/* map NODE_BROADCAST to local broadcast.  */

	memset(&dest,0,sizeof(dest));
	dest.sin_family=AF_INET;
	dest.sin_port=htons(st->udpport);
	dest.sin_addr.s_addr=htonl((p->dst==NODE_BROADCAST)?st->localbcast[0]:p->dst);

	memset(&mheader,0,sizeof(mheader));
	mheader.msg_name=&dest;
	mheader.msg_namelen=sizeof(dest);
	mheader.msg_iov=gather;
	mheader.msg_iovlen=(sizeof(gather)/sizeof(gather[0]));

	gather[0].iov_base=(char*)header;
	gather[0].iov_len=HEADERSIZE;
	gather[1].iov_base=(char*)p->data;
	gather[1].iov_len=p->len;

	header[0]=API_VERSION & VERSION_MASK;      /* This is here, because the current version is 3, and there are no packet types less than 0x10 (and will thus generate unknown packet type errors, on the new format).  Hopefully we'll get all the old versions out of the system before the version passes 0x10.  */

	if (p->dst==NODE_BROADCAST)		/* MS bit of header[0] indicates if this is a broadcast or not */
		header[0]|=BROADCAST_BIT;
#ifdef USE_PACKETPROTECTION
	if (cryptflag)
		header[0]|=CRYPT_BIT;
#endif
	header[1]=p->type;
	header[2]=p->hopcount;
	header[3]=(p->src >> 24) & 0xFF;
	header[4]=(p->src >> 16) & 0xFF;
	header[5]=(p->src >> 8) & 0xFF;
	header[6]=(p->src) & 0xFF;
	header[7]=p->ttl;

	if ((HEADERSIZE+p->len)>MAXPACKETSIZE)
	{
		fprintf(stderr,"realPacketSend: packet is too long! len= %d\n",p->len);
		abort();
	}

	ttl=p->ttl;
	setsockopt(st->udpfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

	rc=sendmsg(st->udpfd,&mheader,0);
#ifdef DEBUG_IO
	fprintf(stderr, "realPacketSend: sent message copy %d on fd %d to %s:%d rc= %d\n",
		0, st->udpfd, inet_ntoa(dest.sin_addr), ntohs(dest.sin_port),
		rc);
#endif
	if ((rc>=0) && (p->dst==NODE_BROADCAST))
	{
		int i;
		for(i=1;(i<st->numbcast) && (rc>=0);i++)
		{
			dest.sin_addr.s_addr=htonl(st->localbcast[i]);
			rc=sendmsg(st->udpfd,&mheader,0);
#ifdef DEBUG_IO
			fprintf(stderr, "realPacketSend: sent message copy %d on fd %d to %s:%d rc= %d\n",
				i, st->udpfd, inet_ntoa(dest.sin_addr), ntohs(dest.sin_port),
				rc);
#endif
		}
	}
	return rc;
}

/* This is the opposite of realPacketSend, and returns a simulator packet structure
 */
static packet *realPacketRec(int fd, State *st)
{
	packet *p;

	struct sockaddr_in dest;
	struct msghdr mheader;
	struct iovec gather[2];
	unsigned char header[HEADERSIZE];
	int len;
	int i, match=0;
	int cryptflag;
	ManetAddr from;

	p=packetMalloc(&(st->themanet->nlist[0]),MAXPACKETSIZE);

	memset(&dest,0,sizeof(dest));
	memset(&mheader,0,sizeof(mheader));
	mheader.msg_name=&dest;
	mheader.msg_namelen=sizeof(dest);
	mheader.msg_iov=gather;
	mheader.msg_iovlen=(sizeof(gather)/sizeof(gather[0]));

	gather[0].iov_base=(char*)header;
	gather[0].iov_len=HEADERSIZE;
	gather[1].iov_base=(char*)p->data;
	gather[1].iov_len=MAXPACKETSIZE;

	len=recvmsg(fd,&mheader,0);

	if (len<0)
	{
		fprintf(stderr,"realPacketRec: recvmsg failed.  rc= %d errno= %d\n",len,errno);
		packetFree(p);
		return NULL;
	}

	from=ntohl(dest.sin_addr.s_addr);

	if ((unsigned int)(len-HEADERSIZE) > (MAXPACKETSIZE))
	{
		fprintf(stderr,"realPacketRec: packet too big!  len= %d max= %d\n",len,MAXPACKETSIZE);
		abort();
	}

	cryptflag=header[0]&CRYPT_BIT;
	p->type= header[1];

	p->hopcount= header[2];
	p->len= len - HEADERSIZE;
	p->src=(header[3] << 24) | (header[4] <<16) | (header[5]<<8) | header[6];
	p->ttl= header[7];

#ifdef DEBUG_IO
	fprintf(stderr,"got a packet from %d.%d.%d.%d  bcast= %d type= 0x%x len= %d src= %d.%d.%d.%d ttl= %d crypt= %d\n",
		PRINTADDR(from),
		header[0] & BROADCAST_BIT, 
		p->type,
		p->len,
		PRINTADDR(p->src),
		p->ttl,
		cryptflag);
#endif

	if ((cryptflag) && (st->themanet->packetProtection==NULL))
	{
		fprintf(stderr, "%s: received encrypted packet from %u.%u.%u.%u, but encryption is disabled (dropping)\n", __func__, PRINTADDR(p->src));
		packetFree(p);
		return NULL;
	}

	/* Is this packet to a node on one of the networks we have?
	 */
	if (st->promisc)
		match=1;
	else
		if (header[0] & BROADCAST_BIT)
		{
		for(i=0;i<st->numbcast;i++)
			if ((from & st->localbcastmask[i]) == (st->localbcast[i] & st->localbcastmask[i]))
				match=1;
		}
		else
			match=1;

	if (!match)
	{
#ifdef DEBUG_IO
		fprintf(stderr,"discarding packet.  match==0\n");
#endif
		packetFree(p);
		return NULL;
	}

	if ((header[0] & (VERSION_MASK))!=API_VERSION)
	{
		fprintf(stderr,"realPacketRec: packet has the wrong protocol version! from %d.%d.%d.%d\n",PRINTADDR(ntohl(dest.sin_addr.s_addr)));
		packetFree(p);
		return NULL;
	}

	if (header[0]&BROADCAST_BIT)
		p->dst=NODE_BROADCAST;
	else
		p->dst=st->localaddr;

	p=packetRemalloc(&(st->themanet->nlist[0]),p->len,p);

	if (cryptflag)
	{
		assert(st->themanet->packetProtection!=NULL);     /* this is checked above...   */
		{
#ifdef USE_PACKETPROTECTION
			if(packetUnprotect(st->themanet->packetProtection, from, p) != 0)
#endif
			{
				fprintf(stderr, "%s: Failed unprotect of packet from %u.%u.%u.%u\n", __func__, PRINTADDR(p->src));
				packetFree(p);
				p = NULL;
			}
		}
	}
	else
	{
		if (st->themanet->packetProtection!=NULL)
		{
			fprintf(stderr, "%s: received unencrypted packet from %u.%u.%u.%u, but encryption is enabled (dropping)\n", __func__, PRINTADDR(p->src));
			packetFree(p);
			return NULL;
		}
	}

	return p;
}


/*********************************************************************************************
 *
 * Functions to handle packets coming in from the MANET
 *
 */


/* walk the event list, looking for rereceived packets, and receive them directly
**
**  This breaks the encapsulation of the event list, but we need to call getNode, and thats
** not available in this namespace.  :-P
*/
void weirdWalkReReceive(State *st)
{
	eventnode *en=NULL,*d=NULL;
	eventbucket *eb=NULL;
	packet *list=NULL,*p;
	int flag=0;

	do
	{
		eb=st->themanet->eventlist;
		while(eb)
		{
			en=eb->bucket;
			eb=eb->next;
			while(en)
			{
				d=en;
				en=en->next;
#if 0
				fprintf(stderr,"weirdWalkReReceive: event type %d  src= %d dst= %d  type= %x\n",d->type,((d->type&0x10)?d->data.pkt->src:0) & 0xFF,((d->type&0x10)?d->data.pkt->dst:0) & 0xFF,((d->type&0x10)?d->data.pkt->type:0) & 0xFF);
#endif
				if (((d->type==EVENT_PACKET) && ((d->data.pkt)->dst==st->themanet->nlist[0].addr))
					 || (d->type==EVENT_REPACKET))
				{
					d->data.pkt->next=list;      /* grab the /packet/ out of the eventnode...  */
					list=d->data.pkt;

					eventnodeDelete(st->themanet,d);   /* and then chuck the eventnode */
				}
			}
		}

		flag=0;
		while(list)
		{
			p=list;
			list=list->next;

#ifdef DEBUG_EVENTS
			fprintf(stderr,"weirdWalkReReceive: getting packet from %d to %d type %x\n",p->src & 0xFF, p->dst & 0xFF, p->type);
#endif
			desGotPacket(&st->themanet->nlist[0],p);
			packetFree(p);
			flag=1;

		}
	} while(flag==1);
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* This is THE select loop for the demon.
 * It implements the discrete event simulator API, using the live network, and real clock.
 */
static int selectLoop(State *st)
{
	fd_set readfds,writefds;
        int maxfd;
        int rc;
	eventnode *nextpack,*nexttimer;
	eventnode *en;
	int tout;
	struct timeval timeout;

	while(!st->exitFlag)
	{
		while(1)
		{
			nextpack=eventnodeNextPacket(st->themanet);	/* next packet queued to be transmitted onto the net */
			if ((nextpack!=NULL) && (nextpack->data.pkt->dst==st->themanet->nlist[0].addr))   /* The next packet is a loopback, which are not transmitted */
			{
				desGotPacket(&st->themanet->nlist[0],nextpack->data.pkt);
				packetFree(nextpack->data.pkt);
				eventnodeDelete(st->themanet,nextpack);
				nextpack=NULL;
			}
			else
				break;
		}

		nexttimer=eventnodeNextTimer(st->themanet);	/* next timer event to be called */

#ifdef DEBUG_SELECT
		fprintf(stderr,"top of select.  nextpack= %p nexttimer= %p\n",nextpack,nexttimer);
#endif
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;

		/* listen for incoming packets  */

		FD_SET(st->udpfd,&readfds);
		GETMAXFD(maxfd,st->udpfd);

		/* walk list of FD events, and generate bitmaps  */

		for(en=st->themanet->fdlist;en;en=en->next)
		{
			switch(en->type)
			{
				case EVENT_FDREAD:
					/* if we have a zillion packets, don't accept new stuff until we empty it (IE: no reads). */
					/* XXX note that this means that clients may block while writing to the API socket, which some may assume can't happen */
					if (eventnodeNumPackets(st->themanet) < 100)
						FD_SET(en->fd,&readfds);
				break;
				case EVENT_FDWRITE:
					FD_SET(en->fd,&writefds);
				break;
			}
			GETMAXFD(maxfd,en->fd);
		}

		/* do we have packets to transmit?  if yes, add udpfd as write  */
		if (nextpack)
		{
			FD_SET(st->udpfd,&writefds);
			GETMAXFD(maxfd,st->udpfd);
		}

		/* do we have a timer event?  set timeout  */
		if (nexttimer)
		{
			tout=nexttimer->rectime - st->themanet->curtime;
			timeout.tv_sec=tout/1000;
			timeout.tv_usec=(tout % 1000) * 1000;
		}
		else
			tout=0;
		
		errno=0;
		if ((nexttimer==NULL) || (tout>0))        /* if we are late on a timer, we do not enter select */
		{
#ifdef DEBUG_SELECT
			destime debug_select_time_stamp = getMilliTime();
			fprintf(stderr,"entering select.  TO= %ld.06%ld time= %lld\n",
				nexttimer?timeout.tv_sec:-1,
				timeout.tv_usec,debug_select_time_stamp);
#endif

			if ((tout>0) && (nextpack==NULL))    /* if we're really going to sleep, and we have no packets to xmit...  */
			{
				/* do tick callbacks */
				eventnode *enlist;

				st->themanet->curtime=getMilliTime();   /* update DES data structure with realtime clock  */

				enlist=st->themanet->ticklist;           /* execute all the pending tick events */
				st->themanet->ticklist=NULL;

				while(enlist)
				{
					en=enlist;
					enlist=enlist->next;

					(en->callback)(en->nodeaddr, en->data.undef);
					eventnodeFree(en);
				}
			}
				
			/* we're about to enter select, which means we have nothing better to do than sleep...  So lets look for loopbacked packets */
			weirdWalkReReceive(st);

			while (((rc=select(maxfd+1,&readfds,&writefds,NULL,nexttimer?&timeout:NULL))<0)&& (errno==EINTR))
				;
#ifdef DEBUG_SELECT
			{
				int i;
				int found = 0;

				destime tmpstamp = getMilliTime();
				fprintf(stderr,"exiting select. time= %lld(%lldms). Data ready on: ",
				tmpstamp, tmpstamp - debug_select_time_stamp);
				for(i = 0; i <= maxfd; ++i)
				{
					if(FD_ISSET(i, &readfds))
					{
						fprintf(stderr, "%d ", i);
						found = !0;
					}
				}
				fprintf(stderr, "%s\n", found ? "" : "-");
			}
#endif
		}
		else
			rc=0;
		
		st->themanet->curtime=getMilliTime();   /* update DES data structure with realtime clock  */

		if (rc>0)
		{
			if (FD_ISSET(st->udpfd,&readfds))
			{
				packet *p;
#ifdef DEBUG_SELECT
				fprintf(stderr,"select: got a packet\n");
#endif

				p=realPacketRec(st->udpfd,st);
				if (p)
				{
					statusCount(&st->themanet->nlist[0], PACKET_RECEIVE, p);
					desGotPacket(&st->themanet->nlist[0],p);
					packetFree(p);
				}
			}

			if ((nextpack) && (FD_ISSET(st->udpfd,&writefds)))
			{
				int myRc;

				//statusCount(st,PACKET_ORIGIN,nextpack->data.pkt);
				while(nextpack!=NULL)
				{
					errno=0;
					if (nextpack->data.pkt->dst==st->themanet->nlist[0].addr)   /* THe next packet is a loopback, which are not transmitted */
					{
						desGotPacket(&st->themanet->nlist[0],nextpack->data.pkt);
						myRc=0;
					}
					else
					{
#ifdef DEBUG_SELECT
						fprintf(stderr,"select: sending packet type %x to %d from %d\n",nextpack->data.pkt->type,nextpack->data.pkt->dst & 0xFF, nextpack->data.pkt->src & 0xFF);
#endif
						myRc=realPacketSend(st,nextpack->data.pkt);
					}

					if (myRc<=0)
					{
						if(errno!=EWOULDBLOCK)
						{
							fprintf(stderr,"realPacketSend: rc= %d errno= %d\n",myRc,errno);
							packetFree(nextpack->data.pkt);
							eventnodeDelete(st->themanet,nextpack);
						}
						break;
					}
					else
					{
						packetFree(nextpack->data.pkt);
						eventnodeDelete(st->themanet,nextpack);
					}
					nextpack=eventnodeNextPacket(st->themanet);    /* next packet queued to be transmitted onto the net */
				}
			}

			/* Walk event list, for file IO events.                               */
			/* This is hairy because a callback may schedule new events, and we   */
			/* don't want to look at the new ones, walking the current ones       */
			/* so we make a list of events to call from the main list             */
			{
				eventnode *curen=st->themanet->fdlist;
				eventnode *preven=NULL;
				eventnode *nexten=NULL;
				eventnode *calllist=NULL;
				int flag;
				while(curen)
				{
					nexten=curen->next;
					flag=0;
					switch(curen->type)
					{
						case EVENT_FDREAD:
							if (FD_ISSET(curen->fd,&readfds))
							{
#ifdef DEBUG_SELECT
								fprintf(stderr,"select: read on api %d\n",curen->fd);
#endif
								flag=1;
							}
						break;
/* If an API is closing and writeable, its readable, and in apiRead will delete the API
 * but it was writeable too, and the write may also get called in the same run through the call
 * list, on a now non-existante API.  Thus, if an FD is readable, then its writeablity will
 * be ignored.
 */
						case EVENT_FDWRITE:
							if (FD_ISSET(curen->fd,&writefds) &&
								(!FD_ISSET(curen->fd,&readfds)))
							{
#ifdef DEBUG_SELECT
								fprintf(stderr,"select: write on api %d\n",curen->fd);
#endif
								flag=1;
							}
						break;
					}
					if (flag)
					{
						if (preven)                          /* remove this en from the event list */
							preven->next=curen->next;
						else
							st->themanet->fdlist=curen->next;

						curen->next=calllist;                  /* and add it to a calllist we're accumulating */
						calllist=curen;

					}
					else
						preven=curen;
					curen=nexten;
				}
				while(calllist)                    /* if we have any events in the calllist, call them  */
				{
					curen=calllist;
					calllist=calllist->next;
					curen->callback(curen->nodeaddr, curen->data.undef);
					eventnodeFree(curen);
				}
			}
		}

		if ((nexttimer) && (nexttimer->rectime < st->themanet->curtime))
		{
#ifdef DEBUG_SELECT
			fprintf(stderr,"timer event\n");
#endif
			(nexttimer->callback)(nexttimer->nodeaddr, nexttimer->data.undef);
			eventnodeDelete(st->themanet,nexttimer);
			nexttimer=NULL;
		}
	}
	return 0;
}


State *st;

void sigint(int)
{
	st->exitFlag=1;
}

int main(int argc, char *argv[])
{
	Config *conf;
    if(argc > 1)
    {
        fprintf(stderr, "loading config: %s\n", argv[1]);
	    conf = configLoad(argv[1]);
    }
    else
    {
        fprintf(stderr, "loading default config\n");
	    conf = configLoad(0);
    }

	st=stateInit(conf);
	if(st)
	{
		signal(SIGPIPE,SIG_IGN);         /* we get a SIGPIPE under very heavy traffic...  but we're doing the select thing, so that event should be handled. */
		signal(SIGINT,sigint);
		st->themanet->curtime=getMilliTime();
		selectLoop(st);
	}
	else
	{
		fprintf(stderr,"could not load config file.\nUsage: %s filename\n",argv[0]);
	}
	return 0;
}
