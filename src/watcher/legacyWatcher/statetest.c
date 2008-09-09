#include <stdio.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "idsCommunications.h"

/*  
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * Test program for issuing the clustering algorithm a state vector.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: statetest.c,v 1.6 2007/07/20 20:24:28 dkindred Exp $";



/* Read a file of the form
 * nodeaddr parentaddr ["rootgroup"]
 *
 * where nodeaddr and parentaddr are dotted quad IP addresses
 * and create an IDSState vector containing them.
 * If the string "rootgroup" is present, nodeaddr is a rootgroup member.
 * Lines beginging with a # will be ignored (as comments).
 */
static IDSState *IDSStateRead(CommunicationsStatePtr cs, FILE *fil)
{
	IDSState *vect;
	IDSStateElement tmplist[1024];
	int i,num=0,lnum=0;
	char line[1024];
	char *p;

	memset(tmplist,0,sizeof(tmplist));

	while((fgets(line,sizeof(line),fil)!=NULL) && (num < (int)(sizeof(tmplist)/sizeof(tmplist[0]))))
	{
		lnum++;
		p=line;
		if (*p=='#')
			continue;
		if (*p=='\r')
			continue;
		if (*p=='\n')
			continue;

		while(isspace(*p) && (*p!=0))           /* walk to end of leading WS  */
			p++;

		if (*p==0)
			return NULL;

		tmplist[num].node=communicationsHostnameLookup(p);

		while(!isspace(*p) && (*p!=0))           /* walk to end of first address */
			p++;

		if (*p==0)
			return NULL;

		while(isspace(*p) && (*p!=0))            /* walk to end of WS after first addr */
			p++;

		if (*p==0)
			return NULL;

		tmplist[num].parent=communicationsHostnameLookup(p);

		while(!isspace(*p) && (*p!=0))           /* walk to end of second address */
			p++;

		/* read additional words */
		while (1)
		{
			char *word;
			/* skip leading whitespace */
			while(isspace(*p))
				p++;
			if (*p==0)
				break;
			word = p;
			while(!isspace(*p) && *p!=0)
			{
				p++;
			}
			*p = 0;
			if (strcasecmp(word,"rootgroup") == 0)
			{
				tmplist[num].rootgroupflag = 1;
			}
			else
			{
				fprintf(stderr, "line %d: ignoring unrecognized extra word \"%s\"\n",
					lnum, word);
			}
			*p = 0;
		}
		printf("item %d:\n", num);
		printf("  node:   %d.%d.%d.%d\n", PRINTADDR(tmplist[num].node));
		printf("  parent: %d.%d.%d.%d\n", PRINTADDR(tmplist[num].parent));
		printf("  rootgroupflag: %d\n", tmplist[num].rootgroupflag);
		num++;
	}
	vect=IDSStateMalloc(cs, num);
	for(i=0;i<num;i++)
	{
		vect->state[i].node=tmplist[i].node;
		vect->state[i].parent=tmplist[i].parent;
		vect->state[i].rootgroupflag=tmplist[i].rootgroupflag;
	}
	return vect;
}

int main(int argc, char *argv[])
{
	CommunicationsStatePtr cs;
	IDSState *vect;
	ManetAddr attach=0;
	int ch;
	FILE *fil=stdin;
	int noflood=0;
	int lock=0;

	while ((ch = getopt(argc, argv, "nld:f:?")) != -1)
		switch (ch)
		{
			case 'n':
				noflood=1;
			break;
			case 'd':
				attach=communicationsHostnameLookup(optarg);
			break;
			case 'l':
				lock=1;
			break;
			case 'f':
				fil=fopen(optarg,"r");
				if (fil==NULL)
				{
					perror("main: ");
					return(1);
				}
			break;
			case '?':
			default:
				fprintf(stderr,"State Vector testing program\n"
					"-d IPaddr - node to connect to (duck)\n"
					"-f filename - read state vector from filename, instead of stdin\n"
					"-n - noflood - do not flood the state vector\n"
					"-l - lock (freeze clustering state after loading)\n"
				);
				return(1);
		}

	

	cs=communicationsInit(attach);

	if (cs==NULL)
	{
		fprintf(stderr,"communicationsInit() failed\n");
		exit(1);
	}
	communicationsNameSet(cs,"statetest","");

	vect=IDSStateRead(cs,fil);
	vect->noflood=noflood;
	vect->lock=lock;

	IDSStateSet(vect);

	communicationsClose(cs);
	return 0;

}


