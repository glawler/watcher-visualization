#ifndef WATCHERGRAPH_H
#define WATCHERGRAPH_H

/*
 * $Id: watcherGraph.h,v 1.7 2006/06/14 15:04:34 tjohnson Exp $
 *
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#include "des.h"
#include "idsCommunications.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0

/* Goal is to make labels act just like edges...  
 * the edge delete function with the bitmap is Real Nice
 */
typedef enum
{
	WATCHERGRAPHEDGE,
	WATCHERGRAPHLABEL
} WatcherGraphType;

struct WatcherGraphEdge
{
	ManetAddr head, tail;
	node *nodeHead, *nodeTail;
	unsigned char color[4];
	NodeLabel labelHead;
	NodeLabel labelMiddle;
	NodeLabel labelTail;
};

typedef struct WatcherGraphLabel
{
        unsigned char bgcolor[4],fgcolor[4];    /* background color, forground color   */
        char *text;
} WatcherGraphLabel;

typedef struct WatcherGraph
{
	watcherGraphType type;
	int priority;
	int family;
	int tag;
	struct NodeEdge *next;

	union
	{
		WatcherGraphEdge edge;
		WatcherGraphLabel label;
	} data;

} WatcherGraph;

#endif

void watcherGraphEdgeInsert(NodeEdge **g, NodeEdge *nw, destime curtime);
void watcherGraphEdgeRemove(NodeEdge **g, int bitmap, NodeEdge *nw);   /* remove a specific edge  */
void watcherGraphEdgeRemoveFamily(NodeEdge **g,int family);      /* remove all edges from a specific family */
void watcherGraphEdgeNuke(NodeEdge **g);      /* remove all edges  */
void watcherGraphDraw(NodeEdge **g, NodeDisplayType dispType, NodeDisplayStatus *dispStat, destime curtime);

/* bits in the bitmap passed to watcherGraphEdgeRemove are in idsCommunications.h
 * If a bit is set, the values must match, otherwise any value will match.
 * So, a bitmap of 0 will remove all edges (watcherGraphEdgeNuke)
 */

#ifdef __cplusplus
}
#endif
#endif
