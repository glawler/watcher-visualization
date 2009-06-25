#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>

#include "idsCommunications.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: labeltest.c,v 1.13 2007/04/23 17:00:47 dkindred Exp $";

/* Test program for putting labels on nodes in the watcher
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

int main(int argc, char *argv[])
{
	CommunicationsStatePtr cs;
	ManetAddr attach;
	int msgflag=0;
	int ch;
	int colorflag=0;
	int removebitmap=0;
	unsigned char *nodecolor=NULL, nodecolorbuff[4];
	NodeLabel lab;

	lab.node=0;
	attach=0;
	lab.bgcolor[0]=255;
	lab.bgcolor[1]=255;
	lab.bgcolor[2]=255;
	lab.bgcolor[3]=255;
	lab.fgcolor[0]=0;
	lab.fgcolor[1]=0;
	lab.fgcolor[2]=0;
	lab.fgcolor[3]=255;
	lab.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
	lab.family=0;
	lab.tag=0;
	lab.text=NULL;
	lab.expiration=0;

	while ((ch = getopt(argc, argv, "e:c:l:n:d:f:b:r:p:a:g:")) != -1)
		switch (ch)
		{
			case 'p':
				sscanf(optarg,"%d",&lab.priority);
			break;
			case 'a':
				sscanf(optarg,"%d",&lab.family);
			break;
			case 'e':
			{
				int i;
				sscanf(optarg,"%d",&i);
				lab.expiration=i;
			}
			break;
			case 'g':
				if (isalpha(optarg[0]))
				{
					lab.tag=communicationsTagHash(optarg);
				}
				else
					sscanf(optarg,"%d",&lab.tag);
			break;
			case 'n':
				lab.node=communicationsHostnameLookup(optarg);
			break;
			case 'd':
				attach=communicationsHostnameLookup(optarg);
			break;
			case 'l':
				lab.text=strdup(optarg);
				msgflag=1;
			break;
			case 'r':
			{
				char *p;

				p=optarg;
				while(*p)
				{
					switch(*p)
					{
						case 'f': removebitmap|=COMMUNICATIONS_LABEL_REMOVE_FAMILY; break;
						case 'p': removebitmap|=COMMUNICATIONS_LABEL_REMOVE_PRIORITY; break;
						case 'n': removebitmap|=COMMUNICATIONS_LABEL_REMOVE_NODE; break;
						case 'g': removebitmap|=COMMUNICATIONS_LABEL_REMOVE_TAG; break;
						default:
							fprintf(stderr,"remove flag: bad flag\n -r [fpng] - remove only labels with the specified Family, Priority, Node, or taG\n");
							exit(1);
					}
					p++;
				}
			}
			break;
			case 'f':
			{
				int tmpcolor;
				tmpcolor=ntohl(inet_addr(optarg));
				lab.fgcolor[0]=tmpcolor >> 24;
				lab.fgcolor[1]=tmpcolor >> 16;
				lab.fgcolor[2]=tmpcolor >> 8;
				lab.fgcolor[3]=tmpcolor;
			}
			break;
			case 'b':
			{
				int tmpcolor;
				tmpcolor=ntohl(inet_addr(optarg));
				lab.bgcolor[0]=tmpcolor >> 24;
				lab.bgcolor[1]=tmpcolor >> 16;
				lab.bgcolor[2]=tmpcolor >> 8;
				lab.bgcolor[3]=tmpcolor;
			}
			break;
			case 'c':
			{
				int tmpcolor;
				if (strcasecmp(optarg,"NULL")==0)
				{
					nodecolor=NULL;
				}
				else
				{
					tmpcolor=ntohl(inet_addr(optarg));
					nodecolor=nodecolorbuff;
					nodecolor[0]=tmpcolor >> 24;
					nodecolor[1]=tmpcolor >> 16;
					nodecolor[2]=tmpcolor >> 8;
					nodecolor[3]=tmpcolor;
				}

				colorflag=1;
			}
			break;
			default:
			case '?':
				fprintf(stderr,"Label and color testing program\n"
					"-d IPaddr - node to connect to (duck)\n"
					"-n IPaddr - node to affect\n"
					"-a int - family\n"
					"-g int|string - tag\n"
					"-p int - priority\n"
					"-l string - label to apply\n"
					"-r [fpng] - remove existing label (specify a ? to get list)\n"
					"-e int - Expiration time (milliseconds)\n"
					"-c red.green.blue.alpha - color to apply to node\n"
					"-f red.green.blue.alpha - forground label color\n"
					"-b red.green.blue.alpha - background label color\n");
				return 1;
			break;
		}

	cs=communicationsInit(attach);
	communicationsNameSet(cs,"labeltest","");

	if (cs==NULL)
	{
		fprintf(stderr,"communicationsInit() failed\n");
		exit(1);
	}

	if ((msgflag) && (removebitmap==0))
		communicationsWatcherLabel(cs,&lab);

	if (removebitmap)
		communicationsWatcherLabelRemove(cs,removebitmap,&lab);

	if (colorflag)
		communicationsWatcherColor(cs,lab.node,nodecolor);

	communicationsClose(cs);
	return 0;
}
