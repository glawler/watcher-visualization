#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "idsCommunications.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: watchergraphtest.c";

/* Test program for feeding the scrolling graphs in the (new old) watcher.
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

void usage(void);

void usage(void)
{
    fprintf(stderr,"Watcher scrolling graph test program:\n" 
            "-c IPaddr - node to connect to (duck), the daemon's address\n"
      // GTL not implemented       "-n IPaddr - node to affect, of not given effect the local node\n"
            "-g string (graph name)\n"
            "-d float (data point), this argument can be given multiple times for multiple data points, if needed\n"); 
}

int main(int argc, char *argv[])
{
	CommunicationsStatePtr cs;
	ManetAddr attach=0;
    char *graphName=NULL;
    float data[255];
    unsigned int numDataPts=0;
    int ch;
    char *endPtr=NULL;

	while ((ch = getopt(argc, argv, "c:g:d:hH?")) != -1)
		switch (ch)
        {
            case 'c':
                attach=communicationsHostnameLookup(optarg);
                break;
            case 'd':
                if (numDataPts>sizeof(data))
                {
                    usage();
                    fprintf(stderr, "\n\nOnly %d data points supported on the command line!\n", sizeof(data));
                    return EXIT_FAILURE;
                }
                errno=0;
                data[numDataPts++]=strtod(optarg, &endPtr);
                if ((errno == ERANGE && (data[numDataPts] == LONG_MAX || data[numDataPts] == LONG_MIN)) || 
                        (errno != 0 && data[numDataPts] == 0) || 
                        (optarg==endPtr) || 
                        (*endPtr))

                {
                    usage();
                    fprintf(stderr, "\nI don't understand the -d arguement '%s'. It should be a float.\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'g':
                graphName=strdup(optarg);
                break;
            default:
            case '?':
            case 'H':
            case 'h':
                usage();
                return EXIT_FAILURE;
                break;
        }

    if (!graphName)
    {
        usage();
        fprintf(stderr, "\nThe -g string (graphname) argument is mandatory\n"); 
        return EXIT_FAILURE;
    }

	cs=communicationsInit(attach);
	communicationsNameSet(cs,"watchergraphtest","");

	if (cs==NULL)
	{
		fprintf(stderr,"communicationsInit() failed\n");
		return EXIT_FAILURE;
	}

    graphSend(cs, graphName, data, numDataPts); 

    free (graphName); 

	communicationsClose(cs);
	return EXIT_SUCCESS;
}
