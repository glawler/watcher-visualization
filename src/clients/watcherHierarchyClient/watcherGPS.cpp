/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file watcherGPS.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include "watcherGPS.h"
#include "marshal.h"

/*
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: watcherGPS.cpp,v 1.7 2007/07/11 03:50:45 dkindred Exp $";

#define WATCHERGPSPAYLOADSIZE (4*3+8*1) /* 3 longs: lat lon alt, one longlong: time */

WatcherGPS *watcherGPSUnmarshal(const void *payload, int payloadlen, WatcherGPS *gps)
{
	unsigned int tmp; // ,tmp2;
	const unsigned char *hp=(const unsigned char*)payload;

	if (payloadlen < WATCHERGPSPAYLOADSIZE) 
		return NULL;

	UNMARSHALLONG(hp,tmp);
	gps->lat=(tmp / 5965230.0) - 180.0;
	UNMARSHALLONG(hp,tmp);
	gps->lon=(tmp / 5965230.0) - 180.0;
	UNMARSHALLONG(hp,tmp);
	gps->alt=tmp;

	//UNMARSHALLONG(hp,tmp);
	//UNMARSHALLONG(hp,tmp2);
	//gps->time=tmp*1000 + tmp2/1000;

	destime tmp3;
	UNMARSHALLONGLONG(hp,tmp3);
	gps->time=tmp3;

	return gps;
}

int watcherGPSMarshal(void *payload, int payloadlen, const WatcherGPS *gps)
{
	unsigned char *hp=(unsigned char*)payload;
	unsigned int tmp;

	if (payloadlen<WATCHERGPSPAYLOADSIZE)
		return 0;
	tmp=(int)((gps->lat+180.0) * 5965230.0);      /* lat goes from -180 to +180  */
	MARSHALLONG(hp,tmp);
	tmp=(int)((gps->lon+180.0) * 5965230.0);      /* lon goes from -180 to +180  */
						/* The big number is 2^31 / 360 rounded down  */
	MARSHALLONG(hp,tmp);
	tmp=(unsigned int)gps->alt;
	MARSHALLONG(hp,tmp);

	// tmp=gps->time / 1000;                  /* Time is represented as whole seconds   */
	// MARSHALLONG(hp,tmp);
	// tmp=(gps->time % 1000 ) *1000;         /* and microseconds   */
	// MARSHALLONG(hp,tmp);

	destime tmp2=gps->time;
	MARSHALLONGLONG(hp, tmp2);

	return WATCHERGPSPAYLOADSIZE;
}
