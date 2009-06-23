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
