/* $Id: watcherGPS.h,v 1.4 2007/07/11 03:50:45 dkindred Exp $
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#ifndef WATCHERGPS_H
#define WATCHERGPS_H

#ifdef __cplusplus
extern "C" {
#endif

// GTL - took destime declaration from --> #include "des.h"
// as I did't want to pull in everything else, when all that was needed
// was destime.
typedef long long int destime;

typedef struct WatcherGPS
{
	double lat, lon, alt;
	destime time;
} WatcherGPS;

WatcherGPS *watcherGPSUnmarshal(const void *payload, int payloadlen, WatcherGPS *);
int watcherGPSMarshal(void *payload, int payloadlen, const WatcherGPS *gps);

#ifdef __cplusplus
}
#endif

#endif
