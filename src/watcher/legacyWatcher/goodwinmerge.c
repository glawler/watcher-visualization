#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "des.h"
#include "apisupport.h"
#include "marshal.h"
#include "watcherGPS.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: goodwinmerge.c,v 1.7 2007/03/09 22:59:58 tjohnson Exp $";

/* Does a merge sort on a list of goodwin files, to generate a single
 * goodwin file.
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#define MAXBUFF 1024

typedef struct
{
	int fd;
	ManetAddr addr;
	int bufflen, buffpos;
	ApiCommand *ac[MAXBUFF];
	destime time[MAXBUFF];
	destime clockdelta;			/* difference between GPS clock and node clock */
} CommunicationsLogMerge;

static void readNext(CommunicationsLogMerge *clm, int usegpsclock)
{
	if (clm->buffpos>=clm->bufflen)		/* if we've hit end of buffered packets... */
	{
		clm->buffpos=0;
		clm->bufflen=1;
		clm->ac[clm->buffpos]=communicationsLogApiCommandRead(clm->fd, &(clm->time[clm->buffpos]), &(clm->addr));
		if ((clm->ac[clm->buffpos]!=NULL) && (usegpsclock))
		{
			if (clm->ac[clm->buffpos]->type==APICOMMAND_MESSAGE_REC)
			{
				MessageInfoPtr mi;

				mi=messageInfoUnmarshal(clm->ac[clm->buffpos]);

/* add logic to delete message types?  */

				if (mi->type==IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS)
				{
					WatcherGPS *location;
					location=watcherGPSUnmarshal(messageInfoRawPayloadGet(mi),messageInfoRawPayloadLenGet(mi));
					clm->clockdelta=clm->time[clm->buffpos]-location->time;
					free(location);
					fprintf(stderr,"node %d.%d.%d.%d: new clock delta = %lld\n",PRINTADDR(clm->addr),clm->clockdelta);
				}
				free(mi);   /* This is scary, but correct, messageInfoDestroy would free the AC on us...  */
			}
			clm->time[clm->buffpos]-=clm->clockdelta;   /* convert node time to GPS time  */
		}
	}
}

int main(int argc, char *argv[])
{
	CommunicationsLogMerge *clm,*nodeList[1024],*least;
	MessageInfoPtr mi;
	ApiCommand *lastread;
	int numnodes=0;
	int i;
	int eof=0;
	int outputfd=1;
	int usegpsclock=0;
	int ch;

	while((ch=getopt(argc, argv,"g?"))!=-1)
	{
		switch(ch)
		{
			case 'g':
				usegpsclock=1;
			break;
			case '?':
			default:
				fprintf(stderr,"goodwinmerge [-g] goodwinfiles - merge a set of goodwin files into a single file\n"
					"-g - rewrite timestamps to GPS time\n"
					);
				exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	nodeList[numnodes]=NULL;
	for(i=0;i<argc;i++)
	{
//		fprintf(stderr,"Starting file %s\n",argv[i]);
		clm=(CommunicationsLogMerge*)malloc(sizeof(*clm));
		nodeList[numnodes++]=clm;
		nodeList[numnodes]=NULL;

		clm->fd=open(argv[i],O_RDONLY);
		if (clm->fd<0)
		{
			perror("main: open failed.");
			exit(1);
		}

		clm->bufflen=0;
		clm->buffpos=0;
		while(1)
		{
			lastread=clm->ac[clm->bufflen]=communicationsLogApiCommandRead(clm->fd, &(clm->time[clm->bufflen]), &(clm->addr));
			clm->bufflen++;
//			fprintf(stderr,"read type %d\n",lastread->type);

			if (lastread==NULL)
			{
				fprintf(stderr,"file %s: premature end of file, INIT section never ends...\n", argv[1]);
				exit(1);
			}

			if ((usegpsclock) && (lastread->type==APICOMMAND_MESSAGE_REC))
			{
				mi=messageInfoUnmarshal(lastread);
				if (mi->type==IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS)
				{
					free(mi);     /* This is scary, but correct, messageInfoDestroy would free the AC on us...  */
					break;
				}
				free(mi);     /* This is scary, but correct, messageInfoDestroy would free the AC on us...  */
			}
			if ((!usegpsclock) && (lastread->type==APICOMMAND_INIT))
				break;
		}
	}

	/* So now we have all the AC's up to the INIT if not in GPS mode, or up to the first GPS timestamp if we are in GPS mode */

	for(i=0;i<numnodes;i++)
	{
		WatcherGPS *location;

		clm=nodeList[i];
		if (usegpsclock)
		{
			int j;

			assert(clm->ac[clm->bufflen-1]->type==APICOMMAND_MESSAGE_REC);
			mi=messageInfoUnmarshal(clm->ac[clm->bufflen-1]);
			assert(mi->type==IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS);
			location=watcherGPSUnmarshal(messageInfoRawPayloadGet(mi),messageInfoRawPayloadLenGet(mi));

			clm->clockdelta=clm->time[clm->bufflen-1] - location->time;   /* compute clock delta between node time and GPS time */

			fprintf(stderr,"node %d.%d.%d.%d: node clock delta == %lld\n",PRINTADDR(clm->addr),clm->clockdelta);

			free(location);
			free(mi);     /* This is scary, but correct, messageInfoDestroy would free the AC on us...  */

			for(j=0;j<clm->bufflen;j++)		/* rewrite all buffered ACs to be GPS time */
				clm->time[j]-=clm->clockdelta;
		}

		while(clm->ac[clm->buffpos]->type!=APICOMMAND_INIT)     /* write this node's INIT section to the goodwin file */
		{
			communicationsLogApiCommandWrite(outputfd, clm->ac[clm->buffpos], clm->time[clm->buffpos], clm->addr);
			apiCommandFree(clm->ac[clm->buffpos]);
			clm->ac[clm->buffpos]=NULL;
			clm->buffpos++;
		}
			/* Then write the INIT packet */
		communicationsLogApiCommandWrite(outputfd, clm->ac[clm->buffpos], clm->time[clm->buffpos], clm->addr);
		apiCommandFree(clm->ac[clm->buffpos]);
		clm->ac[clm->buffpos]=NULL;
		clm->buffpos++;
		readNext(clm,usegpsclock);
	}

	/* So now, we have a pile of nodes, each with 0 or more buffered ACs  (the ACs between the end of the INIT section ad the first GPS AC...  */


/* This needs the code to interpolate global time from local error indications...
 *
 * So, read ACs, and do the linked list thing to buffer them, until we read a GPS message
 *  The GPS message will contain the GPS time, and we can then observe the delta between GPS and node time
 *  Use that first packet, to correct all the timestamps of the buffered ACs to GPS time
 *  Write all the ACs up till the init AC to the output file.
 *  Then proceed with the merge sort thing.  Every time we read an AC:
 *    If it is an INIT message, update the delta from node time to GPS time.
 *    Correct the AC's timestamp from node time to GPS time.
 *  And proceed with the canonical merge sort.
 */
	while(!eof)
	{
		eof=1;
		least=NULL;
		for(i=0;i<numnodes;i++)               /* and do the merge-sort thing...  */
		{
			if (nodeList[i]->ac[nodeList[i]->buffpos])
				eof=0;

			if ((nodeList[i]->ac[nodeList[i]->buffpos]) && ((least==NULL) || ((nodeList[i]->time[nodeList[i]->buffpos] < least->time[least->buffpos]))))
				least=nodeList[i];
		}
		if (least)
		{
			communicationsLogApiCommandWrite(outputfd, least->ac[least->buffpos], least->time[least->buffpos], least->addr);
			apiCommandFree(least->ac[least->buffpos]);
			least->ac[least->buffpos]=NULL;
			least->buffpos++;
			readNext(least,usegpsclock);
		}
		/* If EOF, least->ac will be NULL...  */
	}
	return 0;
}
