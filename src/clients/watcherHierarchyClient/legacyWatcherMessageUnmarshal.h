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
 * @file legacyWatcherMessageUnmarshal.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#ifndef WATCHER_MESSAGE_UNMARHSLASD_H
#define WATCHER_MESSAGE_UNMARHSLASD_H

#include "idsCommunications.h"

/*
 * These were stolen from apisupport.c in idsCommunications. 
 */
unsigned char *communicationsWatcherLabelUnmarshal(unsigned char *hp, NodeLabel *lab);
unsigned char *communicationsWatcherFloatingLabelUnmarshal(unsigned char *hp, FloatingLabel *lab);
unsigned char *watcherColorUnMarshal(unsigned char *hp, uint32_t *nodeAddr, unsigned char *color);

/*
 * Stolen from watcher.cpp 
 *
 * edge is malloc'd into, so free() it after use.
 */
unsigned char *communicationsWatcherEdgeUnmarshal(unsigned char *hp, NodeEdge *&edge);

#endif // WATCHER_MESSAGE_UNMARHSLASD_H
