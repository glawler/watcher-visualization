#ifndef WATCHER_H
#define WATCHER_H

/* $Id: watcher.h,v 1.3 2007/03/09 22:48:08 tjohnson Exp $
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *
 * Data structures used by the watcher, so as to make them available to watcher
 * modules likes watchermovement.
 */

typedef struct clusteringState
{
	CommunicationsState *cs;
	destime lastopen;           /* timestamp from watcher CPU   */
	CommunicationsNeighbor *neighborlist;
	neighbor fakeCH;          /* the us->clusterhead pointer will point to this when we have a CH.  */
	IDSPositionStatus positionStatus[3];
	int needupdate;
	int datavalid;

	ApiPacketCount totpackets;
	ApiPacketCount curpersec;

	long long int lasttime;      /* timestamp from node CPU */
	long long int lasttimewatcher;   /* watcher CPU timestamp of when the lasttime timestamp was observed.  */
	ManetAddr controladdr;

	NodeLabel *labelClockError;   /* our label for the clock error indication  */
	unsigned char color[4];
} clusteringState;


#endif
