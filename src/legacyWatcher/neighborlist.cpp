/*  TELCORDIA TECHNOLOGIES PROPRIETARY - INTERNAL USE ONLY
 *  This file contains proprietary information that shall be distributed,
 *  routed or made available only within Telcordia, except with written
 *  permission of Telcordia.
 */
/* DAG: mods to set neighbor flags 2005-09-27 */

#include <assert.h>				/* DAG: added */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "des.h"
#include "node.h"				/* DAG: added */
#include "amroute.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: neighborlist.cpp,v 1.14 2007/04/25 14:20:05 dkindred Exp $";

extern int notifyneighbor;
extern int notifytree;


void addNeighbor(int ntype, manetNode *us, int index)
    {
    int oldStatus;
    neighbor *n;				/* DAG: added */
    extern int treeExamineOnChange;
    STATIC void notifyNeighbor(int, manetNode *, int, const char *);
    void treeTracker(manet *);

#ifdef DEBUG_AMROUTE				/* DAG: added */
    fprintf(stderr, "addNeighbor(ntype=%d,us=%p,index=%d)\n", ntype, us, index);
#endif
    if (index < 0)
        return;
    oldStatus = us->cluster->nodes[index].isNeighbor[ntype];
    if (oldStatus == 0)
        us->cluster->nNeighbors[ntype]++;
    us->cluster->nodes[index].isNeighbor[ntype] = 1;
    us->cluster->nodes[index].neighborTime[ntype] = us->manet->curtime;

    /* DAG: added */
    /* XXX  ought to merge all neighbor info into generic neighbor struct */
    if ((n = neighborSearch(us, us->manet->nlist[index].addr, 0)) == NULL)
	{
	assert(ntype == MESHNEIGHBOR);	/* else not in tree */
#ifdef DEBUG_AMROUTE
	fprintf(stderr, "node %d inserting new neighbor %d\n",
	    (int)(us->addr & 0xFF), (int)(us->manet->nlist[index].addr & 0xFF));
#endif
	n = neighborInsert(us, us->manet->nlist[index].addr, 0);
	assert(n != NULL);		/* XXX	need better error handling */
	}
    if ((n->flags & NEIGHBOR_HEARS) == 0)
	n->firstheard = us->manet->curtime;
    n->lastheard = us->manet->curtime;
    if (ntype == MESHNEIGHBOR)
	{
	/* JOIN-REQ was broadcast, so it's one hop (not forwarded). */
	/* (Technically the JOIN-ACK might have been relayed.) */
	/* XXX	ought to handle HEARD vs HEARS more carefully */
#ifdef DEBUG_AMROUTE
	fprintf(stderr, "node %d setting mesh flags for %d\n",
	    (int)(us->addr & 0xFF), (int)(n->addr & 0xFF));
#endif
	n->flags |= NEIGHBOR_1HOP | NEIGHBOR_HEARD | NEIGHBOR_HEARS;
	n->clusterhead = NODE_BROADCAST;	/* (none) */
	n->clusterheadflag = 0;
	n->hopcount = 1;		/* XXX belongs to interim algorithm */
	}
    else
	{
	assert(ntype == TREENEIGHBOR);
#if 0	/* XXX fails in simulation - why? */
	assert(n->hopcount == 1);	/* XXX */
#else
	n->hopcount = 1;		/* XXX belongs to interim algorithm */
#endif
#ifdef DEBUG_AMROUTE
	fprintf(stderr, "node %d setting tree flags for %d\n",
	    (int)(us->addr & 0xFF), (int)(n->addr & 0xFF));
#endif
	n->flags &= ~(NEIGHBOR_PARENT | NEIGHBOR_CHILD);
	if (n->addr > us->addr)
	    {
	    n->flags |= NEIGHBOR_PARENT;
	    n->clusterhead = n->addr;
	    n->clusterheadflag = 1;
	    /* Note that we have only local information about the tree. */
	    }
	else	/* we be da man */
	    {
	    n->flags |= NEIGHBOR_CHILD;
	    n->clusterhead = us->addr;
	    n->clusterheadflag = 0;
	    }
	}

    if (notifyneighbor != 0)
        notifyNeighbor(ntype, us, index, (oldStatus == 0 ? "add" : "readd"));
    if (ntype == TREENEIGHBOR && treeExamineOnChange != 0)
        treeTracker(us->manet);
    }


void deleteNeighbor(int ntype, manetNode *us, int index)
    {
    int oldStatus;
    neighbor *n;				/* DAG: added */
    extern int treeExamineOnChange;
    STATIC void notifyNeighbor(int, manetNode *, int, const char *);
    void treeTracker(manet *);

#ifdef DEBUG_AMROUTE
    fprintf(stderr, "deleteNeighbor(ntype=%d,us=%p,index=%d)\n",
	ntype, us, index);
#endif
    if (index < 0)
        return;
    oldStatus = us->cluster->nodes[index].isNeighbor[ntype];
    if (oldStatus == 1)
        us->cluster->nNeighbors[ntype]--;
    us->cluster->nodes[index].isNeighbor[ntype] = 0;
    us->cluster->nodes[index].neighborTime[ntype] = us->manet->curtime;

    /* DAG: added */
    /* XXX  ought to merge all neighbor info into generic neighbor struct */
    if ((n = neighborSearch(us, us->manet->nlist[index].addr, 0)) == NULL)
	{
	assert(ntype == MESHNEIGHBOR);    /* else not in tree */
#ifdef DEBUG_AMROUTE
	fprintf(stderr, "node %d clearing unknown neighbor %d\n",
	     (int)(us->addr & 0xFF), (int)(n->addr & 0xFF)
	      );
#endif
	}
    else    {
	if (ntype == MESHNEIGHBOR)
	    {
#ifdef DEBUG_AMROUTE
	    fprintf(stderr, "node %d clearing mesh flags for %d\n",
		(int)(us->addr & 0xFF), (int)(n->addr & 0xFF));
#endif
#if 0	/* tried this but it seemed not to work */
	    neighborDelete(us, n);
#else	/* retain neighbor data, but alter its state */
	    n->flags &= ~(NEIGHBOR_HEARD | NEIGHBOR_HEARS);
	    n->clusterhead = NODE_BROADCAST;	/* (none) */
	    n->clusterheadflag = 0;
	    n->hopcount = MAXHOP;	/* XXX belongs to interim algorithm */
#endif
	    }
	else
	    {
	    assert(ntype == TREENEIGHBOR);
#ifdef DEBUG_AMROUTE
	    fprintf(stderr, "node %d clearing tree flags for %d\n",
		(int)(us->addr & 0xFF), (int)(n->addr & 0xFF));
#endif
	    n->flags &= ~(NEIGHBOR_PARENT | NEIGHBOR_CHILD);
	    n->clusterhead = NODE_BROADCAST;	/* (none) */
	    n->clusterheadflag = 0;
	    n->hopcount = MAXHOP;	/* XXX belongs to interim algorithm */
	    }
	}

    if (notifyneighbor != 0)
        notifyNeighbor(ntype, us, index, (oldStatus != 0 ? "delete" : "redelete"));
    if (ntype == TREENEIGHBOR && treeExamineOnChange != 0)
        treeTracker(us->manet);
    }


int includesNeighbor(int ntype, manetNode *us, int index)
    {
    return (index >= 0 && us->cluster->nodes[index].isNeighbor[ntype]);
    }
STATIC void notifyNeighbor(int ntype, manetNode *us, int index, const char *action)
    {
    int i;
    const char *dir = "";		/* tree child/parent info */

    /* DAG: added */
    if (ntype == TREENEIGHBOR)
	{
	neighbor *n = neighborSearch(us, us->manet->nlist[index].addr, 0);
	assert(n != NULL);		/* must still be in mesh */
	switch (n->flags & (NEIGHBOR_PARENT | NEIGHBOR_CHILD))
	    {
	case NEIGHBOR_PARENT:
	    dir = " parent";
	    break;

	case NEIGHBOR_CHILD:
	    dir = " child";
	    break;

	default:
	    dir = "?";
	    }
	}

    (void) printf("time %lld: %u %s %s%s neighbor %u -> [",
            us->manet->curtime, us->addr, action,
            (ntype == MESHNEIGHBOR ? "mesh" :
            (ntype == TREENEIGHBOR ? "tree" : "unknown")),
	    dir,				/* DAG: added (also %s above) */
            us->manet->nlist[index].addr);
    for (i = 0; i < us->manet->numnodes; i++)
        if (us->cluster->nodes[i].isNeighbor[ntype] != 0)
            (void) printf(" %u", us->manet->nlist[i].addr);
    (void) printf(" ]\n");
    }


int addrToIndex(manet *m, ManetAddr addr)
    {
    int i;
    manetNode *n;

    for (i = 0, n = m->nlist; i < m->numnodes; i++, n++)
        if (n->addr == addr)
            return (i);
    return (-1);
    }


struct TREE_STATUS {
    int cyclic, asymmetric;
    int ncomponents;
    int cost;
    };

/* cycles are okay */
#define	goodTree(_t)	((_t)->asymmetric == 0 && (_t)->ncomponents == 1)


/* tree statistics */
static destime treeTime[2];		/* amount of time in each state */
static destime costTime;		/* divide by m->curtime for average */
static int componentsTime;		/* divide by m->curtime for average */
void periodicTreeExamine(manetNode *us, void *data)
    {
    extern int treeExamineInterval;
    void treeTracker(manet *);

    if (us->manet->curtime > 0)
        treeTracker(us->manet);
    timerSet(us, periodicTreeExamine, treeExamineInterval, NULL);
    }


void treeTracker(manet *m)
    {
    struct TREE_STATUS treeStatus;
    static int treeState = 0;		/* 0 = no tree, 1 = tree */
    static destime lastTreeChange = (destime) 0;
    int newState;
    STATIC void evaluateTree(manet *, struct TREE_STATUS *);

    treeTime[treeState] += (m->curtime - lastTreeChange);
    evaluateTree(m, &treeStatus);
    newState = goodTree(&treeStatus);
    if (notifytree != 0)
        {
        (void) printf("time %lld: ", m->curtime);
        (void) printf("tree is %s, ", (newState ? "good" : "bad"));
        (void) printf("cyclic=%s, ", (treeStatus.cyclic ? "true" : "false"));
        (void) printf("asymmetric=%s, ", (treeStatus.asymmetric ? "true" : "false"));
        (void) printf("ncomponents=%d, ", treeStatus.ncomponents);
        (void) printf("cost=%d, ", treeStatus.cost);
        (void) printf("\n");
        }
    treeState = newState;
    costTime += treeStatus.cost * (m->curtime - lastTreeChange);
    componentsTime += treeStatus.ncomponents * (m->curtime - lastTreeChange);
    lastTreeChange = m->curtime;
    }


void finalTreeStatistics(manet *m)
    {
    void AMRoutePrintHierarchyGraph(manet *);

    AMRoutePrintHierarchyGraph(m);
    if (m->curtime <= 0)
        return;
    (void) printf("\n");
    (void) printf("valid tree time = %lld (%.2f%%)\n", treeTime[1],
            100. * (double) treeTime[1] / (double) m->curtime);
    (void) printf("average tree cost over time: %f\n",
            (double) costTime / (double) m->curtime);
    (void) printf("average number of components over time: %f\n",
            (double) componentsTime / (double) m->curtime);
    }
STATIC void evaluateTree(manet *m, struct TREE_STATUS *ts)
    {
    int i;
    manetNode *n;
    int *distances;
    STATIC void treeIterate(manetNode *, int, int, int *, struct TREE_STATUS *ts);

    (void) memset((void *) ts, 0, sizeof (struct TREE_STATUS));
    for (i = 0, n = m->nlist; i < m->numnodes; i++, n++)
	if (n->cluster != NULL)		/* DAG: added (kludge) */
        if (n->cluster->runningAMRoute != 0)
            n->cluster->treeMark = 0;
    distances = manetGetPhysicalGraph(m);
    graphMinPath(m, distances);
    for (i = 0, n = m->nlist; i < m->numnodes; i++, n++)
	if (n->cluster != NULL)		/* DAG: added (kludge) */
        if (n->cluster->runningAMRoute != 0 && n->cluster->treeMark == 0)
            {
            ts->ncomponents++;
            treeIterate(n, i, -1, distances, ts);
            }
    free((void *) distances);
    }


STATIC void treeIterate(manetNode *us, int usIndex, int parentIndex, int *distances,
        struct TREE_STATUS *ts)
    {
    int i;
    manetNode *n;

    if (parentIndex >= 0)	/* assume equal weighting if asymmetric */
        {
        if (distances[parentIndex * us->manet->numnodes + usIndex] <= 0 ||
                distances[usIndex * us->manet->numnodes + parentIndex] <= 0)
            return;		/* infinite cost => disconnected */
        else
            ts->cost += (distances[parentIndex * us->manet->numnodes + usIndex] +
                    distances[usIndex * us->manet->numnodes + parentIndex]) / 2;
        }
    if (us->cluster->treeMark != 0)
        {
        ts->cyclic = 1;
        return;
        }
    us->cluster->treeMark = 1;
    for (i = 0, n = us->manet->nlist; i < us->manet->numnodes; i++, n++)
        if (n->cluster->runningAMRoute != 0 &&
                includesNeighbor(TREENEIGHBOR, us, i) && i != parentIndex)
            {
            if (!includesNeighbor(TREENEIGHBOR, n, usIndex))
                ts->asymmetric = 1;
            else
                treeIterate(n, i, usIndex, distances, ts);
            }
    }
int *AMRouteGetHierarchyGraph(manet *m)
    {
    int *graph;
    int i, j;
    int n;

    n = m->numnodes;
    graph = (int *) malloc(n * n * sizeof (int));
    (void) memset(graph, 0, n * n * sizeof (int));
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if ((i != j) && (m->nlist[i].cluster) && (m->nlist[i].cluster->nodes[j].isNeighbor[TREENEIGHBOR]))
                graph[(i * n) + j] = 1;
    return (graph);
    }


void AMRoutePrintHierarchyGraph(manet *m)
    {
    int *graph;
    int i, n;
    int *AMRouteGetHierarchyGraph(manet *);

    graph = AMRouteGetHierarchyGraph(m);
    n = m->numnodes;
    (void) printf("begin AMRoute hierarchy\n");
    for (i = 0; i < n * n; i++)
        (void) printf("%d%c", graph[i], ((i + 1) % n == 0 ? '\n' : ' '));
    (void) printf("end AMRoute hierarchy\n");
    free((void *) graph);
    }
