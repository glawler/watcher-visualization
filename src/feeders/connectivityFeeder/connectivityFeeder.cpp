/* 
 *  Copyright (C) 2009  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */
#include <sysexits.h> 	// portablish exit values. 
#include <stdio.h> 	// for all that crazy standard stuff
#include <stdlib.h>	// for exit() and others. 
#include <unistd.h> 	// getopt(), sleep()
#include <errno.h>
#include <signal.h>	// handle shutdown by flushing routes.  

#include <sys/types.h>  // open() and types
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std; 

#include "routeEntry.h"

#define DEFAULT_FILTER_NETWORK 	"192.168.2.0"
#define DEFAULT_FILTER_MASK	"255.255.255.0"
#define DEFAULT_FREQ 1

static unsigned int signalledShutdown=0; 
void handleShutdown(int sig)
{
	signalledShutdown=1; 
}

void usage(const char *progName, unsigned int exitValue)
{
	fprintf(stderr, "Usage: %s [-f frequency_of_route_dumping][-n network_to_filter]"
			"[-m network_mask_to_filter][-c dump_count][-i interface][-o outfile]"
			" -a localnode_address\n\n", progName);
	fprintf(stderr, "This program will dump route tables to stdout in a binary format.\nDefault frequency is 1 second.\n"); 
	fprintf(stderr, "Defaults: frequency=%d, network=%s, mask=%s\n", DEFAULT_FREQ, DEFAULT_FILTER_NETWORK, DEFAULT_FILTER_MASK); 
	fprintf(stderr, "If given, -i interface, will limit the routes only to those on interface\n"); 
	fprintf(stderr, "(-a parameter is required)\n"); 
	exit(exitValue); 
}

void printRoutes(const vector<routeEntry> &routes, const char *title, ostream &out)
{
	out << title << ":" << endl; 
	for(vector<routeEntry>::const_iterator i = routes.begin(); i != routes.end(); i++) out << *i << endl; 
}

bool writeRoute(const routeEntry &entry, const int fd)
{
	static unsigned char *buffer = NULL;
	static int bufferSize = routeEntry::getBufferSizeNeeded(); 
	if(buffer == NULL) buffer = new unsigned char[bufferSize];

	if(entry.marshal(buffer, bufferSize))
	{
		write(fd, buffer, bufferSize); 
		// flush(fd); 
	}
	else
	{
		fprintf(stderr, "Unable to represent route data in binary format!\n"); 
		exit(EX_SOFTWARE);  // ??? 
	}

	return true; 
}

int main(int argc, char *argv[])
{
	unsigned int logFreq=DEFAULT_FREQ; 
	struct in_addr filterNetwork = { 0 }; 
	struct in_addr filterMask = { 0 }; 
	struct in_addr localhost = { 0 }; 
	char ch; 
	bool verbose=false;
	int loopCount=-1;
	char *ifaceFilter=NULL; 
	int ofd=1;	// default is STDOUT.

	while((ch = getopt(argc, argv, "c:f:n:m:a:o:i:vhH?")) != -1)
	{
		switch(ch)
		{
			case 'c': if(1 != sscanf(optarg, "%d", &loopCount)) { usage(argv[0], EX_USAGE); } 
				  break;
			case 'f': if(1 != sscanf(optarg, "%d", &logFreq)) { usage(argv[0], EX_USAGE); } 
				  break;
			case 'm': if(-1 == inet_pton(AF_INET, optarg, &filterMask))  { usage(argv[0], EX_USAGE); } 
				  break;
			case 'n': if(-1 == inet_pton(AF_INET, optarg, &filterNetwork))  { usage(argv[0], EX_USAGE); } 
				  break;
			case 'a': if(-1 == inet_pton(AF_INET, optarg, &localhost))  { usage(argv[0], EX_USAGE); } 
				  break;
			case 'v': verbose = true; 
				  break;
			case 'i': ifaceFilter = strdup(optarg);
				  break;
			case 'o': if(-1 == (ofd = open(optarg,O_CREAT|O_WRONLY|O_TRUNC)))
				  {
					  fprintf(stderr, "Error opening file \"optarg\"for writing: %s\n",
							  strerror(errno)); 
				  }
                                  break;
			case '?':
			case 'h':
			case 'H':
			default : usage(argv[0], EX_OK); 
		}
	}

	// check args, errors, etc before doing real work
	if(localhost.s_addr == 0) usage(argv[0], EX_USAGE);
	if(filterNetwork.s_addr == 0) 
	{ 
		if(-1 == inet_pton(AF_INET, DEFAULT_FILTER_NETWORK, &filterNetwork))  
		{ 
			fprintf(stderr, "Error parsing default network: %s\n", DEFAULT_FILTER_NETWORK); exit(EX_DATAERR); 
		}
	}
	if(filterMask.s_addr == 0) 
	{ 
		if(-1 == inet_pton(AF_INET, DEFAULT_FILTER_MASK , &filterMask))     
		{ 
			fprintf(stderr, "Error parsing default mask: %s\n",    DEFAULT_FILTER_MASK); exit(EX_DATAERR); 
		}
	}

	sigset_t sigs; 
	sigemptyset(&sigs);
	sigaddset(&sigs, SIGINT); 
	sigaddset(&sigs, SIGHUP); 
	sigaddset(&sigs, SIGTERM); 
	struct sigaction newAct;
	newAct.sa_handler=handleShutdown; 
	newAct.sa_mask=sigs; 
	newAct.sa_flags=0; 
	newAct.sa_restorer=NULL; 
	struct sigaction oldAct; 
	sigaction(SIGINT, &oldAct, NULL); 
	if(oldAct.sa_handler != SIG_IGN) sigaction(SIGINT, &newAct, NULL); 
	sigaction(SIGHUP, &oldAct, NULL); 
	if(oldAct.sa_handler != SIG_IGN) sigaction(SIGHUP, &newAct, NULL); 
	sigaction(SIGTERM, &oldAct, NULL); 
	if(oldAct.sa_handler != SIG_IGN) sigaction(SIGTERM, &newAct, NULL); 

	vector<routeEntry> existingRoutes, latestRoutes, endedRoutes; 
	time_t now=0; 
	while(1)
	{
		// now do some real work
		FILE *file;
		char line[1024];
		char iface[16];
		int flags,refcnt,use,metric,mtu,window;
		unsigned int dst,nexthop,mask;
		int rc;
		struct in_addr destAddr = { 0 }, nextHopAddr = { 0 }; 

		if(NULL == (file=fopen("/proc/net/route","r"))) 
		{
			fprintf(stderr, "Unable to open /proc/net/route for reading: %s\n", strerror(errno)); 
			exit(EX_NOINPUT); 
		}
		now = time(NULL); 
		latestRoutes.clear(); 
		while(fgets(line,sizeof(line)-1,file))
		{
			rc=sscanf(line,"%s\t%x\t%x\t%d\t%d\t%d\t%d\t%x\t%o\t%d\n",iface,&dst,&nexthop,&flags,&refcnt,&use,&metric,&mask,&mtu,&window);

			if ((rc==10) && ((dst & filterMask.s_addr) == filterNetwork.s_addr) && (!ifaceFilter || 0==strcmp(ifaceFilter, iface)) ) 
			{
				destAddr.s_addr = dst; 
				nextHopAddr.s_addr = nexthop; 
				latestRoutes.push_back(routeEntry(localhost, nextHopAddr, destAddr, metric, now, now));  
			}
		}
		fclose(file); 
		if(verbose) fprintf(stderr, ".");  // "verbose", heh. 

		// update times and put routes write routes that have ended to the log file. 
		for(vector<routeEntry>::iterator i = existingRoutes.begin(); i != existingRoutes.end(); i++)
		{
			vector<routeEntry>::iterator found = find(latestRoutes.begin(), latestRoutes.end(), *i); 
			if(found != latestRoutes.end()) i->m_endTime=now; 
			else 
			{	
				i->m_endTime=now; 
				writeRoute(*i, ofd); 
				existingRoutes.erase(i); 
				--i; 
			}
		}
		// add new (first time we've seen 'em) routes to existing routes. 
		for(vector<routeEntry>::iterator i = latestRoutes.begin(); i != latestRoutes.end(); i++)
		{
			vector<routeEntry>::iterator found = find(existingRoutes.begin(), existingRoutes.end(), *i); 
			if(found == existingRoutes.end()) existingRoutes.push_back(*i); 
		}
		// if(verbose) printRoutes(latestRoutes, "Latest Routes", cerr); 
		// if(verbose) printRoutes(existingRoutes, "Existing Routes", cerr); 

		if(loopCount!=-1 && --loopCount <= 0) break; 
		sleep(logFreq); 

		if(signalledShutdown==1)
		{
			break;	// flush routes, then exit cleanly.  
		}
	}

	// if there are routes hanging around, dump em out. 
	now = time(NULL); 
	for(vector<routeEntry>::iterator i = existingRoutes.begin(); i != existingRoutes.end(); i++) 
	{
		i->m_endTime=now; 
		writeRoute(*i, ofd);
	}

	if(ifaceFilter) free(ifaceFilter); 

	existingRoutes.clear(); 

	if(verbose) fprintf(stderr, "\n"); 
	return EX_OK; 
}
