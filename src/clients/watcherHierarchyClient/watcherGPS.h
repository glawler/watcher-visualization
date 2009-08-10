/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file watcherGPS.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
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
