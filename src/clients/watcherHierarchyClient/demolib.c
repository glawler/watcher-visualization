#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "demolib.h"
#include "idsCommunications.h"

/*  Copyright (C) 2004  Networks Associates Technology, Inc.
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 *
 * This is an example aggregator, to show how to use the Infrastructure API.
 * It listens for messages from the demodetector, and sends aggregated messages
 * every 2 seconds.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: demolib.c,v 1.7 2007/08/21 03:48:53 dkindred Exp $";

#define PRINTADDR(a) ((a)>>24)&0xFF,((a)>>16)&0xFF,((a)>>8)&0xFF,(a)&0xFF

/* This callback is called when a message is delivered to the destination, acked, and the ack
 * delivered to the source.  
 *
 * It is specified using the messageInfoCreate() API call, when a message is sent.
 */
void detectorMessageStatus(const struct MessageInfo *mi, void *messageStatusHandlerData,MessageStatus status)
{
	int msgnum=(int)messageStatusHandlerData;
	switch(status)
	{
		case MESSAGE_SUCCESSFUL:
			printf("message %d transmitted successfully\n",msgnum);
		break;
		default:
			printf("message %d lost (might have been delivered)\n",msgnum);
		break;
	}
}


/* This is called by the API when this node's position in the hierarchy changes
 * It is defined using the API function idsPositionRegister().
 */
void detectorPositionUpdate(void *data, IDSPositionType position, IDSPositionStatus status)
{
	switch(status)
	{
		case IDSPOSITION_ACTIVE:
			printf("position %s, active\n",idsPosition2Str(position));
		break;
		case IDSPOSITION_INACTIVE:
			printf("position %s, INactive\n",idsPosition2Str(position));
		break;
		default:
			printf("invalid position status!\n");
	}
}

/* Initialize idsCommunications API.
 *   us - connect to the daemon at this address (0 means local host)
 *   readlog - if non-NULL, read events from that log instead of connecting 
 *             to a daemon.  "-" is stdin.
 *   writeog - if non-NULL, write events (e.g., received msgs) to that file
 *             ("-" is stdout)
 *   name - name of this client (e.g., argv[0]) to register with daemon 
 *          via communicationsNameSet()
 *
 * Returns NULL on failure.
 */
CommunicationsStatePtr detectorCommsInit(ManetAddr us, 
                                         const char *readlog, 
                                         const char *writelog, 
                                         const char *name)
{
	CommunicationsLogStatePtr cl = NULL;
	CommunicationsStatePtr cs = NULL;
	int readfd = -1;
	int writefd = -1;

	if (writelog)
	{
		if (0 == strcmp(writelog,"-"))
		{
			writefd = fileno(stdin);
		} 
		else
		{
			writefd = open(writelog, O_WRONLY|O_CREAT|O_TRUNC, 0666);
		}
		if (writefd == -1)
		{
			fprintf(stderr, "ERROR: %s: can't open log \"%s\" for writing: %s\n",
				__func__, writelog, strerror(errno));
			goto fail;
		}
	}

	if (readlog)
	{
		readfd = (strcmp(readlog,"-") == 0
			  ? fileno(stdin)
			  : open(readlog, O_RDONLY));

		if (readfd == -1)
		{
			fprintf(stderr, "ERROR: %s: can't open log \"%s\" for reading: %s\n",
				__func__, readlog, strerror(errno));
			goto fail;
		} else {
			cl = communicationsLogLoad(readfd);
			CommunicationsStatePtr const * csArray;
			if (cl == NULL)
			{
				fprintf(stderr, "ERROR: %s: communicationsLogLoad() failed on \"%s\"\n",
					__func__, readlog);
				goto fail;
			}
			csArray = communicationsLogNodesGet(cl);
			if (csArray == NULL)
			{
				fprintf(stderr, "ERROR: %s: communicationsLogNodesGet() failed on \"%s\"\n",
					__func__, readlog);

				goto fail;
			}
			cs = csArray[0];
			if (cs == NULL)
			{
				fprintf(stderr, "ERROR: %s: communicationsLogNodesGet returned no nodes -- is \"%s\" empty?\n",
					__func__, readlog);
				goto fail;
			}
			if (csArray[1] != NULL)
			{
				int n;
				for (n=0; csArray[n] != NULL; n++)
				{
				}
				fprintf(stderr, "ERROR: %s: input log file (%s) contains data from multiple nodes (%d)\n",
					__func__, readlog, n);
				goto fail;
			}
		}
	}
	else
	{
		cs = communicationsInit(us);
		if (cs == NULL) goto fail;
	}
	if (writefd != -1)
	{
		communicationsLogEnable(cs, writefd);
	}
	communicationsNameSet(cs, name, NULL);
	return cs;
fail:
        if (cs) communicationsClose(cs);
        if (cl) communicationsLogClose(cl);
	if (writefd != -1 && writefd != fileno(stdout)) close(writefd);
	if (readfd != -1 && readfd != fileno(stdin)) close(readfd);
	return NULL;
}
