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
#include <errno.h>
#include <assert.h>

#include <stdio.h>

static const char *rcsid __attribute__ ((unused)) = "$Id: tealabcontrol.c,v 1.12 2007/06/27 22:08:47 mheyman Exp $";

/* tealabcontrol, sends control messages to the watcher and tealab, for mobility emulation
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

typedef struct 
{
	int port;
	int stepfd;   /* FD to send step messages to the tealabs  */
	unsigned int localaddr,localbcast;

	int step;
	int stepend;
	int steptime;
	int runflag;

	char *tag;

} state;


static state *init(int steptime, int stepstart, int stepend, int run, char *tag)
{
	state *st;
	struct sockaddr_in incoming;
	struct ifconf       ifc; 
	struct ifreq        ifreq, *ifr;
	char buff[1024];
	int rc;
	unsigned int netmask;
	char *manetnetworkconf;
	unsigned int manetnetwork;


	st = (state*)malloc(sizeof(*st));

	st->port=7099;
	st->step=stepstart;
	st->stepend=stepend;
	st->steptime=steptime;
	st->runflag=run;
	st->tag=tag;

	manetnetworkconf="192.168.2.0";

	manetnetwork=ntohl(inet_addr(manetnetworkconf));

	st->stepfd=socket(AF_INET, SOCK_DGRAM, 0);
	if (st->stepfd>=0)
	{
		int optval = 1;
		if ((rc=setsockopt(st->stepfd,SOL_SOCKET,SO_BROADCAST,&optval,sizeof(optval)))<0)
		{
			fprintf(stderr,"stateinit: setsockopt: rc= %d errno= %d\n",rc,errno);
			close(st->stepfd);
			free(st);
			return NULL;
		}
	}
	else
	{
		fprintf(stderr,"stateInit: stepfd socket failed\n");
		free(st);
	}

	ifc.ifc_len=sizeof(buff);
	ifc.ifc_buf=buff;

        if (ioctl(st->stepfd, SIOCGIFCONF, (char *)&ifc)>=0)
        {
#ifdef linux
                for(ifr=ifc.ifc_req;ifr<(struct ifreq *)((char*)ifc.ifc_req+ifc.ifc_len);ifr=(struct ifreq *)(((char*)ifr)+(sizeof(*ifr))
))
#else
                for(ifr=ifc.ifc_req;ifr<(struct ifreq *)((char*)ifc.ifc_req+ifc.ifc_len);ifr=(struct ifreq *)(((char*)ifr)+(ifr->ifr_addr
.sa_len + IFNAMSIZ)))
#endif
                {
                        ifreq = *ifr;

// fprintf(stderr,"interface %s\n",ifr->ifr_ifrn.ifrn_name);

                        if (ioctl(st->stepfd, SIOCGIFFLAGS, (char *)&ifreq) < 0) 
                                continue;

                        if (((ifreq.ifr_flags & IFF_BROADCAST)) && (ifreq.ifr_flags & IFF_UP))
                        {
                                if (ioctl(st->stepfd, SIOCGIFADDR, (char *)&ifreq) < 0)
                                        continue;
                                st->localaddr = ntohl(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr);
                                if ( ioctl(st->stepfd, SIOCGIFBRDADDR, &ifreq) < 0  )
                                        continue;
                                st->localbcast = ntohl(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr);
                                if ( ioctl(st->stepfd, SIOCGIFNETMASK, &ifreq) < 0  )
                                        continue;
                                netmask = ntohl(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr);

// fprintf(stderr,"interface %d.%d.%d.%d\n",PRINTADDR(st->localaddr));

#if 0
                                if ((manetnetworkconf) && (((st->localaddr & netmask) != manetnetwork)))
                                        continue;
#endif
                                break;
                        }
                }
        }



        memset(&incoming,0,sizeof(incoming));

#if 0
	incoming.sin_family = AF_INET;
        incoming.sin_addr.s_addr = htonl(st->localbcast);
        incoming.sin_port = htons(st->udpport);

        rc = bind (st->stepfd, (struct sockaddr *) &incoming,sizeof(incoming));
        if (rc<0)
        {
                fprintf(stderr,"stateInit: stepfd bind failed\n");
                close(st->stepfd);
                free(st);
                return NULL;
        }
#endif

	incoming.sin_family = AF_INET;
        incoming.sin_addr.s_addr = htonl(INADDR_ANY);
        incoming.sin_port = htons(st->port);

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF


#if 1
        fprintf(stderr,"addresses: %d.%d.%d.%d   %d.%d.%d.%d\n",PRINTADDR(st->localaddr),PRINTADDR(st->localbcast));
#endif

	return st;
}

static int stepSend(state *st)
{
	char buff[128];
	struct sockaddr_in dest;
	struct msghdr mheader;
	struct iovec gather[1];
	int i;

	fprintf(stderr,"taking step %d\n",st->step);

	if (st->tag!=NULL)
		sprintf(buff,"%s/tick%d",st->tag, st->step);
	else
		sprintf(buff,"tick%d", st->step);


	memset(&dest,0,sizeof(dest));
	dest.sin_family=AF_INET;
	dest.sin_port=htons(st->port);
	dest.sin_addr.s_addr=htonl(st->localbcast);

	mheader.msg_name=&dest;
	mheader.msg_namelen=sizeof(dest);
	mheader.msg_iov=gather;
	mheader.msg_iovlen=(sizeof(gather)/sizeof(gather[0]));
	mheader.msg_control=NULL;
	mheader.msg_controllen=0;
	mheader.msg_flags=0;

	gather[0].iov_base=buff;
	gather[0].iov_len=strlen(buff);

	for(i=0;i<4;i++)   /* 4 duplicates, for reliability */
		sendmsg(st->stepfd,&mheader,0);

#warning Horrible hack sending two control packets, so watcher and goodwin can both hear it
	dest.sin_port=htons(st->port+1);

	for(i=0;i<4;i++)   /* 4 duplicates, for reliability */
		sendmsg(st->stepfd,&mheader,0);


	/* if watcher is connected, send it new locations file path  */

	st->step++;
	if ((st->stepend>0) && (st->step > st->stepend))
		exit(0);
	return 0;
}

#define GETMAXFD(mfd,nfd) mfd=(nfd>mfd)?nfd:mfd

/* Simple select loop to listen on the api FD, and break out every 2 seconds to
 * send messages.
 *
 * The API is not threadsafe!
 */
static void selectLoop(state *st)
{
	fd_set readfds,writefds;
        int maxfd;
	int rc;
	struct timeval nextreport,curtime;
	struct timeval timeout;

	gettimeofday(&nextreport,NULL);
	nextreport.tv_sec+=1;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		maxfd=-1;

		FD_SET(0,&readfds);
		GETMAXFD(maxfd,0);

		gettimeofday(&curtime,NULL);
		if (timercmp(&curtime,&nextreport,>))
		{
			if (st->runflag)
				stepSend(st);

			timeout.tv_sec=st->steptime / 1000;
			timeout.tv_usec=st->steptime % 1000;
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

				st->runflag=!st->runflag;
				if (st->runflag)
					printf("Starting time\n");
				else
					printf("Stopping time\n");
				fflush(stdout);
			}
		}
	}
}


int main(int argc, char *argv[])
{
	state *st;
	int steptime, ch;
	int stepstart=0, stepend=0;
	int run=0;
	char *tag=NULL;

	steptime = 1000;
	while ((ch = getopt(argc, argv, "g:s:e:t:rh?")) != -1)
	{
		switch (ch)
		{
			case 't':
				sscanf(optarg,"%d",&steptime);
			break;
			case 's':
				sscanf(optarg,"%d",&stepstart);
			break;
			case 'e':
				sscanf(optarg,"%d",&stepend);
			break;
			case 'g':
				tag=strdup(optarg);
			break;
			case 'r':
				run=1;
			break;
			case 'h':
			case '?':
			default:
				fprintf(stderr,"tealabcontrol - sends timing messages to a set of machines running tealab,\n"
					"and a instance of watcher.\n"
					"-t [number] - sets the length of a time step in milliseconds\n"
					"-s number - step to start on (default 0)\n"
					"-e number - step to end on (default infinity (or at least 32 bits of it...)\n"
					"-r - start running immediately (instead of waiting for a CR)\n"
					"-g string - tag to prepend onto step messages\n"
					);
				exit(1);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	st=init(steptime,stepstart,stepend, run, tag);

	printf("Press return to begin time.\n");
	selectLoop(st);
	return 0;
}
