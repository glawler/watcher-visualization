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
 * @file legacyWatcherMessageUnmarshal.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include "legacyWatcherMessageUnmarshal.h"
#include "marshal.h"
#include "logger.h"

/* Unmarshal a single label into an existing label structure.  Assumes that the label already has some
 *  * memory to put the text into.
 *   */
unsigned char *communicationsWatcherLabelUnmarshal(unsigned char *hp, NodeLabel *lab)
{
    TRACE_ENTER();

    int i;

    UNMARSHALLONG(hp,lab->node);
    for(i=0;i<4;i++)
        UNMARSHALBYTE(hp,lab->fgcolor[i]);
    for(i=0;i<4;i++)
        UNMARSHALBYTE(hp,lab->bgcolor[i]);
    UNMARSHALBYTE(hp,lab->family);
    UNMARSHALBYTE(hp,lab->priority);
    UNMARSHALLONG(hp,lab->tag);

    UNMARSHALSTRINGSHORT(hp,lab->text);

    UNMARSHALLONG(hp,lab->expiration);

    TRACE_EXIT();
    return hp;
}

/* Unmarshal a color message
 *  */
unsigned char *watcherColorUnMarshal(unsigned char *hp, uint32_t *nodeAddr, unsigned char *color)
{
    TRACE_ENTER();

    int colorflag;
    int i;

    UNMARSHALLONG(hp,*nodeAddr);

    UNMARSHALBYTE(hp,colorflag);
    if (colorflag)
    {
        for(i=0;i<4;i++)
            UNMARSHALBYTE(hp,color[i]);
    }
    else
        memset(color,0,4);

    TRACE_EXIT();
    return hp;
}

unsigned char *communicationsWatcherFloatingLabelUnmarshal(unsigned char *hp, FloatingLabel *lab)
{
    TRACE_ENTER();

    int i;

    UNMARSHALLONG(hp,lab->x);
    UNMARSHALLONG(hp,lab->y);
    UNMARSHALLONG(hp,lab->z);

    for(i=0;i<4;i++)
        UNMARSHALBYTE(hp,lab->fgcolor[i]);
    for(i=0;i<4;i++)
        UNMARSHALBYTE(hp,lab->bgcolor[i]);
    UNMARSHALBYTE(hp,lab->family);
    UNMARSHALBYTE(hp,lab->priority);
    UNMARSHALLONG(hp,lab->tag);

    UNMARSHALSTRINGSHORT(hp,lab->text);

    UNMARSHALLONG(hp,lab->expiration);

    TRACE_EXIT();
    return hp;
}

unsigned char *communicationsWatcherEdgeUnmarshal(unsigned char *hp, NodeEdge *&edge)
{ 
    TRACE_ENTER();

    edge = (NodeEdge *)malloc(sizeof(*edge) + 256 * 3); 
    memset(edge, 0, (sizeof(*edge) + 256 * 3));                 // GTL added so we know when strings end
    edge->labelHead.text = ((char*)edge) + sizeof(*edge); 
    edge->labelMiddle.text = (edge->labelHead.text) + 256; 
    edge->labelTail.text = (edge->labelMiddle.text) + 256; 

    edge->next = NULL; 
    UNMARSHALLONG(hp, edge->head); 
    UNMARSHALLONG(hp, edge->tail); 
    UNMARSHALLONG(hp, edge->tag); 
    UNMARSHALBYTE(hp, edge->family); 
    UNMARSHALBYTE(hp, edge->priority); 
    for(int i = 0; i < 4; i++) 
        UNMARSHALBYTE(hp, edge->color[i]); 

    char msgflag;
    UNMARSHALBYTE(hp, msgflag); 
    if (msgflag & 1) 
        hp = communicationsWatcherLabelUnmarshal(hp, &(edge->labelHead)); 
    else 
        edge->labelHead.text = NULL;       /* not memory leak because we malloced a big chunk for e, and left pieces for the text blocks... */ 
    if (msgflag & 2) 
        hp = communicationsWatcherLabelUnmarshal(hp, &(edge->labelMiddle)); 
    else 
        edge->labelMiddle.text = NULL;         /* not memory leak because we malloced a big chunk for e, and left pieces for the text blocks... */ 
    if (msgflag & 4) 
        hp = communicationsWatcherLabelUnmarshal(hp, &(edge->labelTail)); 
    else 
        edge->labelTail.text = NULL;       /* not memory leak because we malloced a big chunk for e, and left pieces for the text blocks... */ 


    UNMARSHALBYTE(hp, edge->width); 
    UNMARSHALLONG(hp, edge->expiration); 

    // edge->nodeHead = manetNodeSearchAddress(us->manet, edge->head); 
    // edge->nodeTail = manetNodeSearchAddress(us->manet, edge->tail); 
    // watcherGraphEdgeInsert(&userGraph, e, us->manet->curtime); 

    TRACE_EXIT();
    return NULL;
} 

// void gotMessageEdgeRemove(void *data, const struct MessageInfo *mi) 
// { 
//     manetNode *us = (manetNode*)data; 
//     unsigned char *pos; 
//     NodeEdge buff, *e = &buff; 
//     int bitmap; 
// 
//     pos = (unsigned char *)messageInfoRawPayloadGet(mi); 
// 
//     UNMARSHALBYTE(pos, bitmap); 
//     UNMARSHALLONG(pos, e->head); 
//     UNMARSHALLONG(pos, e->tail); 
//     UNMARSHALBYTE(pos, e->family); 
//     UNMARSHALBYTE(pos, e->priority); 
//     UNMARSHALLONG(pos, e->tag); 
// }

