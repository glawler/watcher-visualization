#include "watchermovement.h"
#include "node.h"
#include "marshal.h"
#include "watcher.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


static const char *rcsid __attribute__ ((unused)) = "$Id: watchermovement.cpp,v 1.14 2007/04/23 18:51:07 dkindred Exp $";

/*
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

typedef struct
{
	int controlfd;
	char curname[1024];
} state;

int tcpOpen(unsigned int addr, int port);
int udpOpen(unsigned int addr, int port);

/* This compares nodes based only on the least significant octet of their
 * address.  This works on all our testbeds, but not in general.  The real
 * intention is to solve this problem by no longer using tealab, and getting
 * the mobility data more tightly bound to the nodes.
 */
static int watcherMobilityGetNodeNum(manet *m,ManetAddr addr)
{
	int i;
	for(i=0;i<m->numnodes;i++)
		if ((addr & 0xFF) == (m->nlist[i].addr & 0xFF))
			return i;
	return -1;
}

static void
watcherMobilityInitFile(manet *m, char *filname,state *)
{
	FILE *fil;
	char line[8192];
	char *p,*inp;
	int i=0;
	unsigned int addr;
	double x,y,z;

	fil=fopen(filname,"r");

	if (fil==NULL)
	{
		fprintf(stderr,"nodeMobilityInitFile: could not open location file.  errno= %d %s\n",errno,strerror(errno));
		return;
	}
	while(fgets(line,sizeof(line),fil))
	{
		if ((line[0]=='#'))
			continue;

		inp=line;
		p=strsep(&inp," \t");

		addr=0;
		addr=ntohl(inet_addr(p));
		if ((addr!=0xFFFFFFFF) && (i<m->numnodes))
		{

#warning nasty manetaddr hack
			i=watcherMobilityGetNodeNum(m,addr);

			fprintf(stderr,"addr= %d.%d.%d.%d index= %d  \n",PRINTADDR(addr),i);

			p=strsep(&inp," \t");

			if (strcmp(p,"at")==0)
			{
				p=strsep(&inp," \t");
				sscanf(p,"%lf",&x);
				p=strsep(&inp," \t");
				sscanf(p,"%lf",&y);
				if (inp)
				{
					p=strsep(&inp," \t");
					sscanf(p,"%lf",&z);
				}
				else
					z=0.0;

				m->nlist[i].x=x;
				m->nlist[i].y=y;
				m->nlist[i].z=z;
			}
		}
	}

	fclose(fil);
}

/* Called by watcher to initialize the Motion library
 *
 * returns a pointer to its state info, which will be passed back
 * to any other watcherMotion calls.
 */
void *watcherMovementInit(manet *)
{
	state *st;

	st=(state*)malloc(sizeof(*st));

	fprintf(stderr,"mobility: init called\n");

	st->controlfd=udpOpen(INADDR_ANY,7099);
	if (st->controlfd<0)
	st->controlfd=udpOpen(INADDR_ANY,7100);
	st->curname[0]=0;

	if (st->controlfd<0)
		fprintf(stderr,"mobility: failed to connect to tealabcontrol (ignoring)  errno= %d\n",errno);

	/* connect to tealabcontrol , listen to the FD  */
	return st;
}

/* if the watcherMotion implementation needs to listen on a
 * file descriptor, it may return it with this call (which the
 * watcher will call regularly in its select loop.)  Return -1
 * to indicate there is no FD.
 */
int watcherMovementFD(void *motionData)
{
	state *st=(state*)motionData;
	return st->controlfd;
}

/* When the FD returned by watcherMotionFD is readable, the
 * watcher will call this function.  (If there is no FD,
 * this function WILL NOT be called.)
 *
 * return non-zero if there is actually new data.
 */
int watcherMovementRead(void *motionData,manet *m)
{
	state *st=(state*)motionData;

	char buff[128];
	int len;

	len=read(st->controlfd,buff,128);
	buff[len]=0;
	strcat(buff,".locations");
	if (strcmp(buff,st->curname))
	{
		fprintf(stderr,"mobility: reading coords from %s\n",buff);
		watcherMobilityInitFile(m,buff,st);
		strcpy(st->curname,buff);
	}

	return 1;
}

/* If there is no FD, this function will be called regularly instead.
 *
 * return non-zero if there is actually new data.
 */
int watcherMovementUpdate(void *, manet *)
{
	return 0;
}

int tcpOpen(unsigned int addr, int port)
{
	int fd;
	struct sockaddr_in sock;
	int rc;

	memset(&sock,0,sizeof(sock));
	sock.sin_family = AF_INET;
	sock.sin_port=htons(port);
	sock.sin_addr.s_addr=htonl(addr);

	fd=socket(AF_INET,SOCK_STREAM,0);

	if (fd<0)
		return fd;
	
	fprintf(stderr,"mobility: socket worked\n");

	rc=connect(fd,(struct sockaddr*)&sock,sizeof(sock));
	if (rc<0)
	{
		close(fd);
		return rc;
	}

	fprintf(stderr,"mobility: connect worked\n");

	return fd;
}


int udpOpen(unsigned int addr, int port)
{
	int fd;
	struct sockaddr_in sock;
	int rc;

	memset(&sock,0,sizeof(sock));
	sock.sin_family = AF_INET;
	sock.sin_port=htons(port);
	sock.sin_addr.s_addr=htonl(addr);

	fd=socket(AF_INET,SOCK_DGRAM,0);

	if (fd<0)
		return fd;
	
	fprintf(stderr,"mobility: udp socket worked\n");

	rc=bind(fd,(struct sockaddr*)&sock,sizeof(sock));
	if (rc<0)
	{
		close(fd);
		return rc;
	}

	fprintf(stderr,"mobility: connect worked\n");

	return fd;
}
