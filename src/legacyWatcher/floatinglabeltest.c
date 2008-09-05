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

static const char *rcsid __attribute__ ((unused)) = "$Id: floatinglabeltest.c,v 1.1 2007/03/09 22:48:08 tjohnson Exp $";

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
	int removebitmap=0;
	FloatingLabel lab;

	lab.x=0;
	lab.y=0;
	lab.z=0;
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
			case 'c':
			{ 
				char *sep = ",";
				char *pos;
				char *num;

				num=strtok_r(optarg, sep, &pos);
				if (num)
				sscanf(num,"%d",&lab.x);
				num=strtok_r(NULL, sep, &pos);
				if (num)
				sscanf(num,"%d",&lab.y);
				if (pos)
				sscanf(pos,"%d",&lab.z);
			}
			break;
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
			case 'd':
				attach=ntohl(inet_addr(optarg));
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
					"-f red.green.blue.alpha - forground label color\n"
					"-b red.green.blue.alpha - background label color\n"
					"-c int,int,int - coordinates of the label\n"
					);
				return 1;
			break;
		}

	cs=communicationsInit(attach);
        communicationsNameSet(cs, "floatinglabeltest", "");

	if (cs==NULL)
	{
		fprintf(stderr,"communicationsInit() failed\n");
		exit(1);
	}

	if ((msgflag) && (removebitmap==0))
		communicationsWatcherFloatingLabel(cs,&lab);

	if (removebitmap)
		communicationsWatcherFloatingLabelRemove(cs,removebitmap,&lab);

	communicationsClose(cs);
	return 0;
}
