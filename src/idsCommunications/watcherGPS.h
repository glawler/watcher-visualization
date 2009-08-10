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

#ifndef WATCHERGPS_H
#define WATCHERGPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "des.h"

typedef struct WatcherGPS
{
	double lat, lon, alt;
	destime time;
} WatcherGPS;

WatcherGPS *watcherGPSUnmarshal(const void *payload, int payloadlen);
int watcherGPSMarshal(void *payload, int payloadlen, const WatcherGPS *gps);

WatcherGPS *watcherGPSUnmarshalUTM(const void *payload, int payloadlen);
int watcherGPSMarshalUTM(void *payload, int payloadlen, const WatcherGPS *gps);

#ifdef __cplusplus
}
#endif

#endif
