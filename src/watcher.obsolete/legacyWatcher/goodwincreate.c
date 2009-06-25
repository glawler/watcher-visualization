#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apisupport.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: goodwincreate.c,v 1.7 2007/07/16 01:41:15 dkindred Exp $";

/* Copyright (C) 2005  McAfee Inc.
 * Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
  */

ManetAddr  localid=NODE_BROADCAST;
ManetAddr  localmask=0;
destime curtime;

/* example line:
 * timestamp vis family priority  tag expiration floatinglabel x,y,z r.g.b.a r.g.b.a text
 */

#define WALKOVER(p) do { while((*p) && (!isspace(*p))) p++; } while(0)
#define WALKNEXT(p) { while((*p) && (isspace(*p))) p++; } while(0)

static ApiCommand *lineParse(char *line)
{
	destime offset;
	int family, priority, tag, expiration;
	char *pos=line,*t;

	while(isspace(*pos))
		pos++;
	if (*pos=='@')
	{
		sscanf(pos+1,"%lld",&offset);
		curtime+=offset;
	}
	else
		sscanf(pos,"%lld",&curtime);

	WALKOVER(pos);   /* over timestamp  */

	WALKNEXT(pos);   /* to type  (vis)  */
	t=pos;
	WALKOVER(pos);   /* over type  */
	WALKNEXT(pos);

	if (strncmp(t,"vis",3)==0)
	{
		sscanf(pos,"%d",&family);
		WALKOVER(pos);
		WALKNEXT(pos);
		sscanf(pos,"%d",&priority);
		WALKOVER(pos);
		WALKNEXT(pos);
		sscanf(pos,"%d",&tag);
		WALKOVER(pos);
		WALKNEXT(pos);
		sscanf(pos,"%d",&expiration);
		WALKOVER(pos);
		WALKNEXT(pos);

		fprintf(stderr,"got a vis, %d %d %d %d  ",family, priority, tag, expiration);

		if (strncmp(pos,"floatinglabel",13)==0)
		{
			FloatingLabel lab;
			char *sep = ",";
			char *sav;
			char *num;
			unsigned int tmpcolor;
			unsigned char buff[1024],*buffend;
			ApiCommand *ac;
			MessageInfo mi;

			lab.family=family;
			lab.priority=priority;
			lab.tag=tag;
			lab.expiration=expiration;

			WALKOVER(pos);
			WALKNEXT(pos);

			t=pos;
			WALKOVER(pos);
			WALKNEXT(pos);

			num=strtok_r(t, sep, &sav);
			if (num)
				sscanf(num,"%d",&lab.x);
			num=strtok_r(NULL, sep, &sav);
			if (num)
				sscanf(num,"%d",&lab.y);
			if (sav)
				sscanf(sav,"%d",&lab.z);

			tmpcolor=ntohl(inet_addr(pos));
			lab.fgcolor[0]=tmpcolor >> 24;
			lab.fgcolor[1]=tmpcolor >> 16;
			lab.fgcolor[2]=tmpcolor >> 8;
			lab.fgcolor[3]=tmpcolor;

			WALKOVER(pos);
			WALKNEXT(pos);

			tmpcolor=ntohl(inet_addr(pos));
			lab.bgcolor[0]=tmpcolor >> 24;
			lab.bgcolor[1]=tmpcolor >> 16;
			lab.bgcolor[2]=tmpcolor >> 8;
			lab.bgcolor[3]=tmpcolor;

			WALKOVER(pos);
			WALKNEXT(pos);

			lab.text=pos;

			fprintf(stderr," floatinglabel %d,%d,%d %d.%d.%d.%d %d.%d.%d.%d \"%s\"\n",lab.x, lab.y, lab.z, lab.fgcolor[0],lab.fgcolor[1],lab.fgcolor[2],lab.fgcolor[3], lab.bgcolor[0],lab.bgcolor[1],lab.bgcolor[2],lab.bgcolor[3], lab.text);

			/* create a Messageinfo Structure...  */
			buffend=communicationsWatcherFloatingLabelMarshal(buff, &lab);

			mi.statusCallback=NULL;
			mi.statusData=NULL;
			mi.origin=localid;
			mi.dest.addr=NODE_BROADCAST;
			mi.dest.type=COMMUNICATIONSDESTINATION_DIRECT;
			mi.dest.ttl=1;
			mi.type=IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL;
			mi.cs=NULL;
			mi.apiOriginated=1;
			mi.tag=0;
			mi.demonId=0;
			mi.dataId=0;
			mi.routeFlags=0;

			mi.payload=buff;
			mi.payloadPtr=NULL;
			mi.payloadLen=buffend-buff;

			/* Put it in an ApiCommand structure  */

			ac=messageInfoMarshal(&mi);
			ac->type=APICOMMAND_MESSAGE_REC;    /* and make it look like an incoming msg */

			return ac;


		}
		else
			return NULL;
	}
	else
		return NULL;

}

int main(int argc, char *argv[])
{
	FILE *in=stdin;
	char line[8192],*t;
	int out=1;
	destime eventTime=0;
	ApiCommand *ac,*initlist=NULL;
	int firstevent=1;
        ApiInit init;

/* Create a goodwin header (such as it is..  */

	init.localid=localid;             /* and the init command...  */
	init.netmask=localmask;
	init.apiVersion=API_VERSION;
	ac=apiInitMarshal(&init);
	initlist=apiCommandConcatenate(initlist,ac);

	while(fgets(line,sizeof(line),in))
	{
		if (line[0]=='#')
			continue;

		if ((t=strchr(line,'\n')) != NULL)
			*t=0;

		eventTime=curtime;
		ac=lineParse(line);
		if (ac)
		{
			if (firstevent)
			{
				eventTime=curtime;
				communicationsLogApiCommandWrite(out,initlist,eventTime, localid);
				apiCommandFree(initlist);
				firstevent=0;
			}
			fprintf(stderr,"writing time= %lld\n",curtime);
			communicationsLogApiCommandWrite(out, ac,curtime, localid);

		}
	}
	return 0;
}
