#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "idsCommunications.h"
#include "apisupport.h"
#include "watcherGPS.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: goodwintest.c,v 1.14 2007/08/17 20:02:10 dkindred Exp $";

/* Test program for reading goodwin files.
 * note that it is more or less a normal idsCommunications client, with a
 * different select loop
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */


static void detectorPositionUpdate(void *data, IDSPositionType position, IDSPositionStatus status)
{
	CommunicationsStatePtr cs=(CommunicationsStatePtr)data;
	destime t = communicationsLastEventTimeGet(cs);
	printf("node %d.%d.%d.%d: ",PRINTADDR(communicationsNodeAddress(cs)));

	switch(status)
	{
		case IDSPOSITION_ACTIVE:
			printf("position %s, active",idsPosition2Str(position));
		break;
		case IDSPOSITION_INACTIVE:
			printf("position %s, INactive",idsPosition2Str(position));
		break;
		default:
			printf("invalid position status!");
	}
	printf(" (time %lld.%03d)\n", t / 1000, (int)(t % 1000));
}

static void gotMessage(void *data,const struct MessageInfo *mi)
{
	CommunicationsStatePtr cs=(CommunicationsStatePtr)data;
	destime t = communicationsLastEventTimeGet(cs);
	printf("node %d.%d.%d.%d: got message type 0x%x at time %lld.%03d\n",
	       PRINTADDR(communicationsNodeAddress(cs)),messageInfoTypeGet(mi),
	       t / 1000, (int)(t % 1000));
}

static void gotMessageLabel(void *data, const struct MessageInfo *mi)
{
	CommunicationsStatePtr cs=(CommunicationsStatePtr)data;
	unsigned char *pos;
	char string[260];
	NodeLabel lab;
	lab.text=string;

	pos=(unsigned char *)messageInfoRawPayloadGet(mi);
	pos=communicationsWatcherLabelUnmarshal(pos, &lab);

	printf("node %d.%d.%d.%d: got message type 0x%x  messagelabel text is \"%s\"\n",PRINTADDR(communicationsNodeAddress(cs)),messageInfoTypeGet(mi),lab.text);
}

static void gotMessageGPS(void *data, const struct MessageInfo *mi)
{
	CommunicationsStatePtr cs=(CommunicationsStatePtr)data;
	WatcherGPS *location;

	location=watcherGPSUnmarshal(messageInfoRawPayloadGet(mi), messageInfoRawPayloadLenGet(mi));
	if (location==NULL)
	{
		printf("node %d.%d.%d.%d: got message type 0x%x  GPS data corrupted\n",PRINTADDR(communicationsNodeAddress(cs)),messageInfoTypeGet(mi));
		return;
	}

	printf("node %d.%d.%d.%d: got message type 0x%x  GPS data %f %f %f  time= %lld\n",PRINTADDR(communicationsNodeAddress(cs)),messageInfoTypeGet(mi), location->lat, location->lon, location->alt, location->time);

	free(location);
}


int main(int argc, char *argv[])
{
	CommunicationsLogStatePtr cl;
	CommunicationsStatePtr const * cs;
	int i;
	long totalevents = 0;
	int fd;
	char *source = NULL;

	if (argc < 2 || (argc == 2 && strcmp(argv[1],"-")==0)) {
		fd = fileno(stdin);
		source = "<stdin>";
	} else if (argc == 2 && argv[1][0] != '-') {
		source = argv[1];
		fd = open(source, O_RDONLY);
		if (fd == -1)
		{
			fprintf(stderr, "%s: error opening \"%s\": %s\n",
				argv[0], source, strerror(errno));	
			exit(1);
		}
	} else {
		fprintf(stderr, "usage: %s [goodwinfile.gdwn]\n", argv[0]);
		fprintf(stderr, "  (If no input file is specified, stdin is read.)\n");
		exit(1);
	}
	printf("reading from %s\n", source);
	cl=communicationsLogLoad(fd);
	cs=communicationsLogNodesGet(cl);

	for(i=0;cs[i];i++)
	{
		printf("node %d.%d.%d.%d\n",PRINTADDR(communicationsNodeAddress(cs[i])));

		idsPositionRegister(cs[i],COORDINATOR_NEIGHBORHOOD,IDSPOSITION_INFORM,detectorPositionUpdate,cs[i]);
		idsPositionRegister(cs[i],COORDINATOR_REGIONAL,IDSPOSITION_INFORM,detectorPositionUpdate,cs[i]);
		idsPositionRegister(cs[i],COORDINATOR_ROOT,IDSPOSITION_INFORM,detectorPositionUpdate,cs[i]);
		idsPositionRegister(cs[i],COORDINATOR_ROOTGROUP,IDSPOSITION_INFORM,detectorPositionUpdate,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL,gotMessageLabel,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL_REMOVE,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE_REMOVE,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS,gotMessageGPS,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH_EDGE,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL,gotMessage,cs[i]);
		messageHandlerSet(cs[i],COMMUNICATIONS_MESSAGE_INBOUND,COMMUNICATIONS_MESSAGE_AFTERALL,COMMUNICATIONS_MESSAGE_READONLY,IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL_REMOVE,gotMessage,cs[i]);
	}
	printf("goodwin file contains events for %d nodes\n",i);

	while(1)
	{
		long nevents;
		long long int t = communicationsLogNextEventTimeGet(cl);
		if (t >= 0)
		{
			printf("Next event time: %lld.%03d\n", t/1000, 
			       (int)(t % 1000));
		}
		if ((nevents = communicationsLogStep(cl,0,NULL)) < 0)
		{
			break;
		}
		totalevents += nevents;
	}
	printf("End of File -- processed %ld events.\n", totalevents);
	return 0;
}

