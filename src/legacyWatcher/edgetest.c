#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "idsCommunications.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: edgetest.c,v 1.12 2007/04/23 17:00:47 dkindred Exp $";

/* Test program for putting labels on nodes in the watcher
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */


int main(int argc, char *argv[])
{
	CommunicationsStatePtr cs;
	ManetAddr attach=0;
	int ch;
	char *p;
	unsigned char bgcolor[4],fgcolor[4];
	int removeflag=0;
	int removebitmap=0;
	NodeLabel *labelCurrent;
	NodeEdge edge;

	edge.head=0;
	edge.tail=0;
	edge.priority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
	edge.family=0;
	edge.tag=0;
	edge.labelHead.text=NULL;
	edge.labelMiddle.text=NULL;
	edge.labelTail.text=NULL;
	edge.width=15;
	edge.expiration=0;
	labelCurrent=NULL;
	edge.color[0]=255;
	edge.color[1]=80;
	edge.color[2]=255;
	edge.color[3]=255;
	bgcolor[0]=255;
	bgcolor[1]=255;
	bgcolor[2]=255;
	bgcolor[3]=255;
	fgcolor[0]=0;
	fgcolor[1]=0;
	fgcolor[2]=0;
	fgcolor[3]=255;
	memcpy(edge.labelHead.fgcolor,fgcolor,4);
	memcpy(edge.labelMiddle.fgcolor,fgcolor,4);
	memcpy(edge.labelTail.fgcolor,fgcolor,4);
	memcpy(edge.labelHead.bgcolor,bgcolor,4);
	memcpy(edge.labelMiddle.bgcolor,bgcolor,4);
	memcpy(edge.labelTail.bgcolor,bgcolor,4);

	while ((ch = getopt(argc, argv, "e:h:t:o:d:l:mr:c:f:b:?a:g:w:")) != -1)
		switch (ch)
		{
			case 'd':
				attach=communicationsHostnameLookup(optarg);
			break;
			case 'h':
				edge.head=communicationsHostnameLookup(optarg);
				labelCurrent=&edge.labelHead;
			break;
			case 't':
				edge.tail=communicationsHostnameLookup(optarg);
				labelCurrent=&edge.labelTail;
			break;
			case 'm':
				labelCurrent=&edge.labelMiddle;
			break;
			case 'p':
				sscanf(optarg,"%d",&edge.priority);
			break;
			case 'w':
				sscanf(optarg,"%d",&edge.width);
			break;
			case 'e':
			{
				int i; 
				sscanf(optarg,"%d",&i);
				edge.expiration=i;
			}
			break;
			case 'a':
				sscanf(optarg,"%d",&edge.family);
			break;
			case 'g':
				if (isalpha(optarg[0]))
				{
					edge.tag=communicationsTagHash(optarg);
				}
				else
					sscanf(optarg,"%d",&edge.tag);

			break;
			case 'l':
				if (labelCurrent)
					labelCurrent->text=strdup(optarg);
				else
					fprintf(stderr,"which label does that -l go on?\n");
			break;
			case 'r':
				removeflag=1;
				p=optarg;
				while(*p)
				{
					switch (*p)
					{
						case 'h': removebitmap|=COMMUNICATIONS_EDGE_REMOVE_HEAD; break;
						case 't': removebitmap|=COMMUNICATIONS_EDGE_REMOVE_TAIL; break;
						case 'f': removebitmap|=COMMUNICATIONS_EDGE_REMOVE_FAMILY; break;
						case 'p': removebitmap|=COMMUNICATIONS_EDGE_REMOVE_PRIORITY; break;
						case 'g': removebitmap|=COMMUNICATIONS_EDGE_REMOVE_TAG; break;
						default:
							fprintf(stderr,"remove flag: bad flag\n -r [htfpg]+ - remove only edges which match Head, Tail, Family, Priority, or taG\n");
							exit(1);
						break;
					}
					p++;
				}
			break;
			case 'c':
			{
				int tmpcolor;
				tmpcolor=ntohl(inet_addr(optarg));
				edge.color[0]=tmpcolor >> 24;
				edge.color[1]=tmpcolor >> 16;
				edge.color[2]=tmpcolor >> 8;
				edge.color[3]=tmpcolor;
			}
			break;
			case 'f':
			{
				int tmpcolor;
				tmpcolor=ntohl(inet_addr(optarg));
				if (labelCurrent)
				{
					labelCurrent->fgcolor[0]=tmpcolor >> 24;
					labelCurrent->fgcolor[1]=tmpcolor >> 16;
					labelCurrent->fgcolor[2]=tmpcolor >> 8;
					labelCurrent->fgcolor[3]=tmpcolor;
				}
				else
					fprintf(stderr,"Which label does that forground color go on?\n");
			}
			break;
			case 'b':
			{
				int tmpcolor;
				tmpcolor=ntohl(inet_addr(optarg));
				if (labelCurrent)
				{
					labelCurrent->bgcolor[0]=tmpcolor >> 24;
					labelCurrent->bgcolor[1]=tmpcolor >> 16;
					labelCurrent->bgcolor[2]=tmpcolor >> 8;
					labelCurrent->bgcolor[3]=tmpcolor;
				}
				else
					fprintf(stderr,"Which label does that background color go on?\n");
			}
			break;
			default:
			case '?':
				fprintf(stderr,"Edge testing program\n"
				"-d IPaddr - node to connect to (duck)\n"
				"-h IPaddr - node to be head\n"
				"-t IPaddr - node to be tail\n"
				"-a int - family of edge\n"
				"-p int - priority of edge\n"
				"-c red.green.blue.alpha - color -f edge\n"
				"-f red.green.blue.alpha - forground label color\n"
				"-b red.green.blue.alpha - background label color\n"
				"-m next label and/or colors is for the middle label\n"
				"-g int|string - label tag\n"
				"-w int - width of edge (default is 15)\n"
				"-e int - expiration time for edge (in milliseconds)\n"
				"-r [htfpg]- remove edge  (specify a ? to get list)\n");
				return 1;
			break;
		}

	cs=communicationsInit(attach);
	communicationsNameSet(cs,"edgetest","");

	if (cs==NULL)
	{
		fprintf(stderr,"communicationsInit() failed\n");
		exit(1);
	}

	if (removeflag)
		communicationsWatcherEdgeRemove(cs, removebitmap, edge.head, edge.tail, edge.family, edge.priority,edge.tag);
	else
		communicationsWatcherEdge(cs,&edge);
#if 0
 head, tail, family, priority, color, (labelHead.text)?&labelHead:NULL, (labelMiddle.text)?&labelMiddle:NULL, (labelTail.text)?&labelTail:NULL);
#endif

	communicationsClose(cs);
	return 0;
}
