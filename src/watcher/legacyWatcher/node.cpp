#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "des.h"
#include "node.h"
#include "simulation.h"
#include "metric.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: node.cpp,v 1.49 2007/08/14 20:17:52 dkindred Exp $";


/* Stuff to handle the recent packets list, for flood routing without completely dieing
 */
packet *nodePacketSearch(manetNode *us, packet *p)
{
        int i;

        for(i=0;i<us->packetlistlen;i++)
        {
                if ((us->packetlist[i]->src==p->src) && (us->packetlist[i]->dst==p->dst) && (us->packetlist[i]->type==p->type) && (us->packetlist[i]->len==p->len) && (memcmp(us->packetlist[i]->data,p->data,p->len)==0))
                        return p;
        }
        return NULL;
}

void nodePacketInsert(manetNode *us, packet *p)
{
        if (us->packetlist[us->packetlistpos])
                packetFree(us->packetlist[us->packetlistpos]);

        us->packetlist[us->packetlistpos]=(packet*)packetDup(p);

        us->packetlistpos=(us->packetlistpos+1) % NODE_PACKETLIST;
        if (us->packetlistpos>us->packetlistlen)
                us->packetlistlen=us->packetlistpos;
}


/* Count the number of nodes with level equal to level arg
*/
int neighborCount(manetNode *us, int level)
{
	int i=0;
	neighbor *n;

	for(n=us->neighborlist;n!=NULL;n=n->next)
	{
		if (n->level==level)
			i++;
	}
	return i;
}



/* Code to handle the neighbor list
*/
neighbor *neighborSearch(manetNode *us, ManetAddr addr,int level)
{
        neighbor *n;

        n=us->neighborlist;
        while((n)) //  (test optimization code)  && (n->addr < addr)
        {
                if ((n->addr==addr) && (n->level==level))
                        return n;
                n=n->next;
        }
        return NULL;
}

/* Insert a neighbor into the neighbor list.  
 * neighbors are sorted by address, (least first)
 */
neighbor *neighborInsert(manetNode *us, int addr, int level)
{
        neighbor *n;
	neighbor *p, *q;

        n=(neighbor*)malloc(sizeof(*n));
        n->addr=addr;
        n->level=level;
        n->hopcount=MAXHOP;

        n->flags=0;
	n->cluster=NULL;
	n->groupCluster=NULL;
        n->lastheard=us->manet->curtime;

#if 1
	q=NULL;
	p=us->neighborlist;
	while((p) && (p->addr < n->addr))
	{
		q=p;
		p=p->next;
	}

	if (q)
	{
		n->next=q->next;
		q->next=n;
	}
	else
	{
		n->next=us->neighborlist;
		us->neighborlist=n;
	}
#else
	n->next=us->neighborlist;
	us->neighborlist=n;
#endif
        return n;
}

static void neighborFree(manetNode *us, neighbor *n)
{
	if (n->cluster)
		free(n->cluster);
	free(n);
	if (us->clusterhead==n)
		us->clusterhead=NULL;
}


void neighborNuke(manetNode *us)
{
	neighbor *n,*d;

	n=us->neighborlist;
	us->neighborlist=NULL;
	while(n)
	{	
		d=n;
		n=n->next;

		neighborFree(us,d);
	}
}

void neighborDelete(manetNode *us, neighbor *n)
{
	neighbor *p,*q;

	q=NULL;
	p=us->neighborlist;

	while((p) && (p!=n))
	{
		q=p;
		p=p->next;
	}

	assert(p!=NULL);

	if (q)
		q->next=p->next;
	else
		us->neighborlist=p->next;

	neighborFree(us,n);
}

/* given a hierarchy graph, compute the node levels within it
*/
int *graphComputeLevels(manet *m, int *hierarchygraph)
{
	int i,j,flag,level;
	int *levels=(int*)malloc(sizeof(levels[0])*m->numnodes);
	int valid[m->numnodes];
	int havekids;
	int done=0;

	for(i=0;i<m->numnodes;i++)
	{
		valid[i]=0;
		levels[i]=0;
	}

	while(!done)
	{
		done=1;
		for(i=0;i<m->numnodes;i++)
		{
			if (valid[i])
				continue;

			flag=1;
			level=0;
			havekids=0;
			for(j=0;j<m->numnodes;j++)
				if (hierarchygraph[j*m->numnodes+i]>0)       /* if j is a child of i  */
				{
					havekids=1;
					if (valid[j])
					{
						if (levels[j] > level)
							level=levels[j];
					}
					else
						flag=0;                        /* else we don't know all kid's level, so node i isn't valid  */
				}
			if (flag)
			{
				valid[i]=1;
				levels[i]=level;
				if (havekids)
					levels[i]++;
				done=0;
			}
		}
	}

	flag=1;
	for(i=0;i<m->numnodes;i++)
		if (valid[i]==0)
			flag=0;
		
	return levels;
}


static void nodeMetricsCallback(manetNode *us, void *)
{
	nodeMetrics(us,1);
}

/* callback function, typically attached to node 0, which makes a bunch of observations
** 
** should be algorithm agnostic
*/
void nodeMetrics(manetNode *us,int schedflag)
{
	int i,j;
	int maxlevel;
	int levelhistogram[MAXLEVEL];
	static metric *childdegree[MAXLEVEL];
	static metric *clusterheadDistance[MAXLEVEL];
	static metric *nodedegree=NULL;
	static metric *pathlen=NULL;
	int numroots=0;
	int totalkids;
	int totalcoverage;
	int *physicalgraph,*hierarchygraph;
	int *physicalgraphminpath,*hierarchygraphminpath;
	int numphysicalpartitions;
	int numhierarchypartitions;

	static ManetAddr root=NODE_BROADCAST;
	manet *m=us->manet;
	int n=us->manet->numnodes;

	physicalgraph=manetGetPhysicalGraph(m);
#ifdef AMROUTE_HACK
	int *AMRouteGetHierarchyGraph(manet *m);
	hierarchygraph=AMRouteGetHierarchyGraph(m);
#else
	hierarchygraph=manetGetHierarchyGraph(m);
#endif

	physicalgraphminpath=(int*)malloc(n*n*sizeof(int));
	memcpy(physicalgraphminpath,physicalgraph,n*n*sizeof(int));
	graphMinPath(m,physicalgraphminpath);

	hierarchygraphminpath=(int*)malloc(n*n*sizeof(int));
	memcpy(hierarchygraphminpath,hierarchygraph,n*n*sizeof(int));

	graphMakeBidirectional(m,hierarchygraphminpath);
	graphMinPath(m,hierarchygraphminpath);

#if 0
	for(i=0;i<n*n;i++)
		if (hierarchygraphminpath[i])
			hierarchygraphminpath[i]=physicalgraphminpath[i];
#endif

//	graphDump(m,"physical",physicalgraph,stderr);
//	graphDump(m,"hierarchy",hierarchygraph,stderr);
//	graphDump(m,"hierarchyminpath",hierarchygraphminpath,stderr);

	if (schedflag)
		timerSet(us,nodeMetricsCallback,1000,NULL);

//	manetAvailability(m);

	if (nodedegree==NULL)
		nodedegree=metricCreate();

	metricClear(nodedegree);

	{
		int *nodedegreetable;
		nodedegreetable=graphNodeDegree(physicalgraph,n);
		for(i=0;i<n;i++)
		{
			metricWrite(nodedegree,nodedegreetable[i]);
//			printf("node %d has %d neighbors\n",i,nodedegreetable[i]);
		}
		free(nodedegreetable);
	}
	
	printf("distribution of 1 hop neighbors: %f %f %f %f\n",metricMin(nodedegree),metricMean(nodedegree),metricStdDev(nodedegree),metricMax(nodedegree));

/* Record timestamp when root first formed  */
	for(i=0;i<n;i++)
		if ((m->nlist[i].rootflag) && (m->nlist[i].addr!=root))
		{
			if (root==NODE_BROADCAST)
				printf("root formed at %lld\n",m->curtime);
			root=m->nlist[i].addr;
		}


/* Init a pile of metrics...
*/
	for(i=0;i<MAXLEVEL;i++)
	{
		levelhistogram[i]=0;
		if (clusterheadDistance[i]==NULL)
			clusterheadDistance[i]=metricCreate();
		else
			metricClear(clusterheadDistance[i]);
		if (childdegree[i]==NULL)
			childdegree[i]=metricCreate();
		else
			metricClear(childdegree[i]);
	}

/* compute a histogram of levels  
*/

	{
		int *levels;
		int tmpchilddegree;

		levels=graphComputeLevels(m,hierarchygraph);

		for(i=0;i<n;i++)
		{
			if (levels[i]>=MAXLEVEL)
			{
				fprintf(stderr,"node %d: illegal level\n",m->nlist[i].addr & 0xFF);
				continue;
			}
			levelhistogram[levels[i]]++;

			for(j=0;j<n;j++)                                   /* is node j node i's clusterhead?   if yes, add its minpath to the metric, thus computing the distribution of distance to CH as a function of level */
				if ((i!=j) && hierarchygraph[i*n+j]>0)
				{
					metricWrite(clusterheadDistance[levels[i]], hierarchygraphminpath[i*n+j]);
					j=n;
				}

			tmpchilddegree=0;
			for(j=0;j<n;j++)                                   /* is  node j node i's child?*/
				if ((i!=j) && hierarchygraph[j*n+i]>0)
				{
					tmpchilddegree++;
				}
			metricWrite(childdegree[levels[i]], tmpchilddegree);
		}
		maxlevel=0;
		for(j=0;j<MAXLEVEL;j++)
		{
			if (levelhistogram[j]>0)
				maxlevel=j;
		}
		for(j=0;j<=maxlevel;j++)
		{
			printf("level histogram: level %d %d   dist of distance to CH: %f %f %f %f  dist of child degree: %f %f %f %f\n",j,levelhistogram[j],
				metricMin(clusterheadDistance[j]),metricMean(clusterheadDistance[j]),metricStdDev(clusterheadDistance[j]),metricMax(clusterheadDistance[j]),
				metricMin(childdegree[j]),metricMean(childdegree[j]),metricStdDev(childdegree[j]),metricMax(childdegree[j]) 
				);
		}

		free(levels);
	}

	{
		int *hierarchygraphminpathint;
		int marked[m->numnodes];

		hierarchygraphminpathint=(int*)malloc(n*n*sizeof(int));
		memcpy(hierarchygraphminpathint,hierarchygraphminpath,n*n*sizeof(int));

		for(i=0;i<n*n;i++)
			if (physicalgraphminpath[i]==0)
				hierarchygraphminpathint[i]=0;

//		graphDump(m,"physical",physicalgraphminpath,stderr);
//		graphDump(m,"hierarchyintphysical",hierarchygraphminpathint,stderr);

		memset(marked,0,sizeof(marked));

		totalcoverage=0;
		for(i=0;i<n;i++)
		{
			if (m->nlist[i].rootflag)
			{
				numroots++;

/* new from-the-graph and much-simpler version:
*/
				totalkids=0;
				if (!marked[i])
				{
					totalkids++;
					marked[i]=1;
				}
				for(j=0;j<n;j++)
					if ((i!=j) && (hierarchygraphminpathint[j*n+i]>0) && (!marked[j]))     /* if i'th node points to j   */
					{
						totalkids++;
						marked[j]=1;
					}

				printf("root %d has %d recursive kids\n",m->nlist[i].addr & 0xFF,totalkids);
				printf("fraction of nodes connected to root %d : %f\n",m->nlist[i].addr & 0xFF,totalkids/(float)n);
				totalcoverage+=totalkids;
			}
		}
		free(hierarchygraphminpathint);
	
		printf("numroots: %d\n",numroots);

		{
			static destime starttime=0;
			static destime lasttick=0;
			static double totavail=0.0;

			if (starttime==0)
				starttime=m->curtime;

			totavail+=totalcoverage*(double)(m->curtime-lasttick);
			
			printf("availability: %d %lld %d %lf  %lf\n",n,(m->curtime-lasttick),totalcoverage, totavail,(totavail / (n * (m->curtime-starttime))));
			lasttick=m->curtime;
		}
	}

#if 1
	if (pathlen==NULL)
		pathlen=metricCreate();

	metricClear(pathlen);

	for(i=0;i<n;i++)
		for(j=0;j<n;j++)
			if ((i!=j) && (physicalgraphminpath[i*n+j]>0) && (physicalgraphminpath[i*n+j]<MAXHOP))
				metricWrite(pathlen,physicalgraphminpath[i*n+j]);

	printf("physical all path length dist: %f %f %f %f\n",metricMin(pathlen),metricMean(pathlen),metricStdDev(pathlen),metricMax(pathlen));

	metricClear(pathlen);

	for(i=0;i<n;i++)
		if (m->nlist[i].rootflag)
		{
			for(j=0;j<n;j++)
				if ((i!=j) && (physicalgraphminpath[j*n+i]>0) && (physicalgraphminpath[j*n+i]<MAXHOP))
					metricWrite(pathlen,physicalgraphminpath[j*n+i]);
		}

	printf("physical root path length dist: %f %f %f %f\n",metricMin(pathlen),metricMean(pathlen),metricStdDev(pathlen),metricMax(pathlen));

	numphysicalpartitions=graphNumPartitions(m,physicalgraphminpath);
	printf("mobility: num partitions: %d  time: %lld\n",numphysicalpartitions,m->curtime);



	metricClear(pathlen);

	for(i=0;i<m->numnodes;i++)
		for(j=0;j<m->numnodes;j++)
			if ((i!=j) && (hierarchygraphminpath[i*n + j]>0) && (hierarchygraphminpath[i*n + j]<MAXHOP))
				metricWrite(pathlen,hierarchygraphminpath[i*n + j]);

	if (metricCount(pathlen)>0)
		printf("hierarchy all path length dist: %f %f %f %f\n",metricMin(pathlen),metricMean(pathlen),metricStdDev(pathlen),metricMax(pathlen));

	metricClear(pathlen);
	for(i=0;i<n;i++)
		if (m->nlist[i].rootflag)
		{
			for(j=0;j<n;j++)
				if ((i!=j) && (hierarchygraphminpath[j*m->numnodes+i]>0) && (hierarchygraphminpath[j*m->numnodes+i]<MAXHOP))
					metricWrite(pathlen,hierarchygraphminpath[j*m->numnodes+i]);
		}

	/* This is the distribution of the distance of every recursive child node To the root (which they are recursive children of...)
	*/
	if (metricCount(pathlen)>0)
		printf("hierarchy root path length dist: %f %f %f %f\n",metricMin(pathlen),metricMean(pathlen),metricStdDev(pathlen),metricMax(pathlen));


	metricClear(pathlen);
	for(i=0;i<n;i++)
		{
			int CHflag=0;
			for(j=0;j<m->numnodes;j++)
				if (hierarchygraph[j*n+i]>0)                 /* if node j points to node i, then node i is a CH  */
					CHflag=1;
			if (CHflag)
				for(j=0;j<n;j++)
					if (hierarchygraph[i*n+j]>0)             /* if node i points to node j, then add that distance to the metric  */
						metricWrite(pathlen,hierarchygraphminpath[i*m->numnodes+j]);
		}


	if (metricCount(pathlen)>0)
		printf("hierarchy CH path length dist: %f %f %f %f\n",metricMin(pathlen),metricMean(pathlen),metricStdDev(pathlen),metricMax(pathlen));

	metricClear(pathlen);

	{
		int leafflag;
	
		for(i=0;i<n;i++)
		{
			/* if node i has no children, add its distance to its CH to the metric  */
			leafflag=1;
			for(j=0;j<n;j++)
				if (hierarchygraph[j*n+i]>0)
					leafflag=0;
			if (leafflag)
				for(j=0;j<n;j++)      /* walk graph looking for CH link from i to its CH (j)  */
					if (hierarchygraph[i*n+j]>0)
						metricWrite(pathlen,hierarchygraphminpath[i*n+j]);
		}
	}

	if (metricCount(pathlen)>0)
		printf("hierarchy leaf-CH path length dist: %f %f %f %f\n",metricMin(pathlen),metricMean(pathlen),metricStdDev(pathlen),metricMax(pathlen));

	{
		int *nodedegreetable;

		metricClear(nodedegree);
		nodedegreetable=graphNodeDegree(hierarchygraph,m->numnodes);
		for(i=0;i<m->numnodes;i++)
			if (nodedegreetable[i]>0)
				metricWrite(nodedegree,nodedegreetable[i]);
		free(nodedegreetable);
		if (metricCount(nodedegree)>0)
			printf("hierarchy: node child degree: %f %f %f %f\n",metricMin(nodedegree),metricMean(nodedegree),metricStdDev(nodedegree),metricMax(nodedegree));
	}


	{
		/* for a degree of n children, how much aggregation can occur?  1.0 means no reduction, 0.0 means total reduction
		 */
		double aggregation[]={1.0,1.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
		double agg;

		agg=graphAggregationCompute(us->manet, hierarchygraph,physicalgraphminpath,numphysicalpartitions,aggregation);
		printf("aggregation: %lf\n",agg);
	}


	graphMakeBidirectional(m,hierarchygraph);
	numhierarchypartitions=graphNumPartitions(m,hierarchygraph);
	printf("hierarchy: num partitions: %d  time %lld\n",graphNumPartitions(m,hierarchygraph),m->curtime);

	free(physicalgraph);
	free(hierarchygraph);
	free(physicalgraphminpath);
	free(hierarchygraphminpath);
#endif

/* display packet origin and repeat counts,
*/
	{
	int ni,p;
	long long int totorigin=0, totrepeat=0, totrec=0;
	long long int historigin[PACKET_MAX], histrepeat[PACKET_MAX];

	memset(historigin,0,sizeof(historigin));
	memset(histrepeat,0,sizeof(histrepeat));

	for(ni=0;ni<m->numnodes;ni++)
		for(p=0;p<m->nlist[ni].status.numtypes;p++)
		{
			totrec+=m->nlist[ni].status.packetList[p].unicastRecCount;
			totrec+=m->nlist[ni].status.packetList[p].bcastRecCount;
			if ((m->nlist[ni].status.packetList[p].unicastRecCount>0) || (m->nlist[ni].status.packetList[p].bcastRecCount>0))
				printf("node %d type 0x%x rec %lld %lld\n", m->nlist[ni].addr & 0xFF, p, m->nlist[ni].status.packetList[p].unicastRecCount, m->nlist[ni].status.packetList[p].bcastRecCount);

			totorigin+=m->nlist[ni].status.packetList[p].origUnicastXmitCount;
			totorigin+=m->nlist[ni].status.packetList[p].origBcastXmitCount;
			historigin[m->nlist[ni].status.packetList[p].type]+=m->nlist[ni].status.packetList[p].origUnicastXmitCount+m->nlist[ni].status.packetList[p].origBcastXmitCount;
			totrepeat+=m->nlist[ni].status.packetList[p].repUnicastXmitCount;
			totrepeat+=m->nlist[ni].status.packetList[p].repBcastXmitCount;
			histrepeat[m->nlist[ni].status.packetList[p].type]+=m->nlist[ni].status.packetList[p].repUnicastXmitCount+m->nlist[ni].status.packetList[p].repBcastXmitCount;
		}
	printf("packet origin: %lld\npacket repeat: %lld\npacket receive: %lld\ntime: %lld\npackets/tick: %f\n",totorigin,totrepeat,totrec, (m->curtime-m->starttime),(totorigin+totrepeat)/(float)(m->curtime-m->starttime)*1000.0);

	for(i=0;i<PACKET_MAX;i++)
		if ((historigin[i]>0) || (histrepeat[i]>0))
			printf("packet histogram type %x %lld %lld\n",i,historigin[i], histrepeat[i]);
	}
}


#if 0
/* Recursive metrics function.
**  
** This walks all the clusterhead variables on all the nodes, to determine how many
** children clusterheads have, and how much of the MANET we have coverage on.
*/
static int nodeCoverage(manetNode *us, int depth, int *levelhist,metric *levelkidhist[],metric *clusterheadDistance[])
{
	int i;
	int nkids=1;  /* =1 because we count ourselves  */
	int ndirectkids=0;

	if (us->drawflag)
		return 0;
	if (depth>MAXLEVEL)
		return 0;

	us->drawflag=1;
	levelhist[depth]++;

	for(i=0;i<us->manet->numnodes;i++)
		if ((us->manet->nlist[i].clusterhead) && (us->manet->nlist[i].clusterhead->addr==us->addr))
		{
			nkids+=nodeCoverage(&us->manet->nlist[i],depth+1,levelhist,levelkidhist,clusterheadDistance);
			ndirectkids++;
		}
	metricWrite(levelkidhist[depth],ndirectkids);

	if (us->clusterhead)   /* if we have a CH, record distance to it...  */
		metricWrite(clusterheadDistance[depth],us->clusterhead->hopcount);

	return nkids;
}
#endif

/* observes the availability of the hierarchy.
** It is measured in node-seconds.  A perfect hierarchy would have a measurement of
** number of nodes * runtime over the experiment, and number of nodes * elapsed during the experiment
*/
double manetAvailability(manet *m)
{
	static int lasttick=0;
	static double totavail=0,avail;
	int i;
	int nkids=0;
	int *flagarray;


	flagarray=(int*)calloc(m->numnodes,sizeof(int));

	for(i=0;i<m->numnodes;i++)
		if (m->nlist[i].rootflag)
			nkids+=nodeAvailability(&m->nlist[i],flagarray);

	free(flagarray);

	avail= (totavail / (m->numnodes * m->curtime))  / (double)(m->numnodes * m->curtime);
	if (m->curtime-lasttick>0)
	{
		totavail+=nkids*(double)(m->curtime-lasttick);
		printf("availability: %d %lld %lf  %lf\n",nkids,(m->curtime-lasttick),totavail,avail);
		lasttick=m->curtime;
	}
	return avail;
}

/* Internal function for manetAvailability. 
** returns the number of child nodes under a node
*/
int nodeAvailability(manetNode *us,int *flagarray)
{
	int nkids=0;
	int i;
	int freeflag=0;

	if (!flagarray)
	{
		flagarray=(int*)calloc(us->manet->numnodes,sizeof(int));
		freeflag=1;
	}

	for(i=0;i<us->manet->numnodes;i++)
	{
		if ((us->manet->nlist[i].addr==us->addr) && (flagarray[i]==0))
		{
			nkids++;
			flagarray[i]=1;
		}
		if ((us->manet->nlist[i].clusterhead) && (us->manet->nlist[i].clusterhead->addr==us->addr) && (flagarray[i]==0))
		{
			nkids+=nodeAvailability(&us->manet->nlist[i],flagarray);
			flagarray[i]=1;
		}
	}

	if (freeflag)
		free(flagarray);

	return nkids;
}


/* path length function.
**
** Given a manet with n nodes, it returns an n by n array containing the
** length of the shortest path between every pair of nodes.
**
** The geometry data is from the 1 hop neighbor data gathered by the nodes.
**
*/

int *nodePathLength(manet *m)
{
	neighbor *ne;
	int i,j;
	int n=m->numnodes;
	int *distance;

	distance=(int*)malloc(n*n*sizeof(int));

	for(i=0;i<n;i++)
		for(j=0;j<n;j++)
		{
			if (i==j)
				distance[i*n+j]=0;
			else
			{
				ne=neighborSearch(&m->nlist[i],j,0);
				if ((ne) && ((ne->flags & (NEIGHBOR_HEARD|NEIGHBOR_HEARS)) == (NEIGHBOR_HEARD|NEIGHBOR_HEARS)))
					distance[i*n+j]=1;
				else
					distance[i*n+j]=0;
			}
		}

	graphMinPath(m,distance);
	return distance;
}


/* multiply a graph by another graph...  a=a*b
**
**  The point is to take a link state graph, and multiply it by a path length graph,
**  to get the path lengths.
*/

void graphMultiply(manet *m, int *a, int *b)
{
	int i;

	for(i=0;i<m->numnodes*m->numnodes;i++)
		a[i]=a[i]*b[i];
}

/*
**  For an edge to be in A, it must also exist in B
*/
void graphAnd(manet *m, int *a, int *b)
{
	int i;

	for(i=0;i<m->numnodes*m->numnodes;i++)
		if (b[i]==0)
			a[i]=0;
}

/* given a graph of edges, computs the shortest path beetn every node
** It uses the FloydAPSP algorithm.   O( n^3)   (ick)
**  distance==0 means infinity.
*/
void graphMinPath(manet *m, int *distance)
{
	int i,j,k;
	int n=m->numnodes;

	for(k=0;k<n;k++)
		for(i=0;i<n;i++)
			for(j=0;j<n;j++)
				if ((distance[i*n+k]>0) && (distance[k*n+j]>0)
				  && ((distance[i*n+j]==0) || ((distance[i*n+k]+distance[k*n+j]) < distance[i*n+j])))
				{
					distance[i*n+j]=distance[i*n+k] + distance[k*n+j];
				}
}

/* Given a graph, computes the transitive closure, and overwrites graph with it
*/
void graphTransitiveClosure(manet *m, int *distance)
{
	int i,j,k;
	int n=m->numnodes;

	for(k=0;k<n;k++)
		for(i=0;i<n;i++)
			for(j=0;j<n;j++)
				if ((distance[i*n+k]>0) && (distance[k*n+j]>0))
				{
					distance[i*n+j]=1;
				}
}

/* returns intersection of edges of graphs a and b, in the block of memory occupied by a
*/
#if 0
void graphIntersection(manet *m, int *a, int *b)
{
	fprintf(stderr,"graphIntersection: unimplemented!\n");
	abort();
}
#endif

/* Given a graph, returns a 1D array of the number of nodes which connect /to/ that node
** The intention is to compute the child degree of a hierarchy graph
** note that a node can point to itself...  
*/
int *graphNodeDegree(int *a, int num)
{
	int *ndeg;
	int i,j;

	ndeg=(int*)malloc(sizeof(*ndeg)*num);
	for(i=0;i<num;i++)
		ndeg[i]=0;
	for(i=0;i<num;i++)
		for(j=0;j<num;j++)
			if (a[j*num+i]>0)
				ndeg[i]++;

	return ndeg;
}

/* returns the one hop graph of the manet.
** this is in the same format as nodePathLength, only it is the physical model,
** not the linklayer data
*/
int *manetGetPhysicalGraph(manet *m)
{
	int *graph;
	int n=m->numnodes;

	graph=(int*)malloc(n*n*sizeof(int));
	memcpy(graph,m->linklayergraph,n*n*sizeof(int));
	return graph;
}

int manetGetNodeNum(manet *m, ManetAddr addr)
{
	int no;

	for(no=0;no<m->numnodes;no++)
		if (m->nlist[no].addr==addr)
			return no;
	return -1;
}

manetNode *manetNodeSearchAddress(manet *m, ManetAddr addr)
{
	int index=manetGetNodeNum(m,addr);

	if (index>=0)
		return &(m->nlist[index]);
	else
		return NULL;
}
/* returns the one hop graph of the manet.
** this is in the same format as nodePathLength, only it is the physical model,
** not the linklayer data
*/
int *manetGetNeighborGraph(manet *m)
{
	int *graph;
	int no;
	neighbor *ne;
	int n=m->numnodes;
	
	graph=(int*)calloc(n*n,sizeof(int));

	for(no=0;no<m->numnodes;no++)
	{
		ne=m->nlist[no].neighborlist;

		while(ne)
		{
			graph[no*n+manetGetNodeNum(m,ne->addr)]=1;
			ne=ne->next;
		}
	}
	return graph;
}


/* returns the clusterhead graph.
**
** indicates path length, as recorded by the hierarchies data
**
** NOTE: This is a pretty inefficient form to store and access
** the hierarchy in.  Could use a vector with an element for 
** each node, containing that node's clusterhead nodenum.
*/
int *manetGetHierarchyGraph(manet *m)
{
	int *graph;
	int i,j;
	int n=m->numnodes;

	graph=(int*)malloc(n*n*sizeof(int));

	if (m->hierarchygraph)
	{
		memcpy(graph,m->hierarchygraph,n*n*sizeof(int));
	}
	else
	{
		memset(graph,0,n*n*sizeof(int));

		for(i=0;i<n;i++)
			if (m->nlist[i].clusterhead!=NULL)
			{
				j=manetGetNodeNum(m,m->nlist[i].clusterhead->addr);
				graph[i*n+j]=1;

				if (graph[i*n+j]>=MAXHOP)
					graph[i*n+j]=0;
			}
	}
	return graph;
}

int *manetGetHierarchyLevels(manet *m)
{
	int i;
	int *levels;

	levels=(int*)malloc(m->numnodes*sizeof(int));

	for(i=0;i<m->numnodes;i++)
		levels[i]=m->nlist[i].level;

	return levels;
}

void graphMakeBidirectional(manet *m, int *graph)
{
	int n=m->numnodes;
	int i,j;

	for(i=0;i<n;i++)
		for(j=0;j<i;j++)	
			graph[i*n+j]=graph[j*n+i]=MAX(graph[i*n+j] , graph[j*n+i]);
}

void graphDump(manet *m, char *label,int *graph, FILE *fd)
{
	char line[10240];
	int i,j;
	int n=m->numnodes;

	for(i=0;i<n;i++)
	{
		line[0]=0;
		for(j=0;j<n;j++)
			if (graph[i*n+j])
				sprintf(line+j*3,"%2d ",graph[i*n+j]);
			else
				sprintf(line+j*3,"-- ");
		fprintf(fd,"%s  %s\n",label,line);
	}
}

int graphNumPartitions(manet *m,int *g)
{
	int n=m->numnodes;
	int i,j;
	int covered[n];
	int nump=0;

	graphTransitiveClosure(m,g);

	for(i=0;i<n;i++)
		covered[i]=0;

	for(i=0;i<n;i++)
	{
		for(i=0;i<n;i++)      /* find next uncovered node */
			if (covered[i]==0)
				break;
		if (i>=n)            /* is there no next?  if yes, exit */
			break;

		nump++;

		covered[i]=1;        /* node, and all its transitive closure neighbors are now covered.  */
		for(j=0;j<n;j++)
			if (g[i*n+j])
				covered[j]=1;
	}
	return nump;
}

static void nodeHierarchyCountEdgesCallback(manetNode *us, void *)
{
	nodeHierarchyCountEdges(us,1);
}

void nodeHierarchyCountEdges(manetNode *us, int schedflag)
{
        manet *m=us->manet;
        int i,j;
        static int *lastgraph=NULL;
        int *graph;
        int n=m->numnodes;
        int event=0;

	if (schedflag)
		tickSet(us,nodeHierarchyCountEdgesCallback,NULL);

        graph=manetGetHierarchyGraph(us->manet);

	for(i=0;i<n;i++)
		for(j=0;j<n;j++)
			if (graph[i*n+j] != ((lastgraph!=NULL)?lastgraph[i*n+j]:0))
			{
#if 0
				printf("hierarchy: %s %d to %d \n",graph[i*n+j]?"make":"break",i,j);
#endif
				event++;
			}
        if (lastgraph)
		free(lastgraph);
        lastgraph=graph;

	if (event>0)
		printf("hierarchy events: %d  time: %lld\n",event,us->manet->curtime);
}



/*
If a node has no incoming edges, that node's outgoing BW is now known.

repeat

        All nodes which are now known are then marked with their outgoing BW.
 
        All other nodes are now checked, If all of a node's incoming edges are known,
        that node is now known, and is marked with its outgoing BW (using the incoming
        BW as input).
 
        Are all nodes now known?  if yes we're done.
 
 
A node's output is 1 report + number of incoming reports multiplied by aggregation factor
for that degree.  Thus the aggregation factor is a function of the degree.
*/

double graphAggregationCompute(manet *m, int *hierarchygraph,int *physicalgraphminpath,int numphysicalpartitions,double *aggregation)
{
typedef struct
{
	double size;
	int distance;
	int doneflag;
	int rootflag;
} NodeMeta;

	NodeMeta *nodeTotal;

	int numnodes=m->numnodes;
	int alldoneflag=0;
	int nodedoneflag=0;
	double childtotal;
	int childcount=0;
	int i,j;
	int count=0;
	int baddata;

	nodeTotal=(NodeMeta*)malloc(sizeof(nodeTotal[0])*numnodes);
	for(i=0;i<numnodes;i++)
	{
		nodeTotal[i].size=0.0;
		nodeTotal[i].distance=0;
		nodeTotal[i].doneflag=0;
		nodeTotal[i].rootflag=0;
	}

	while(!alldoneflag)
	{
		alldoneflag=1;
		for(i=0;i<numnodes;i++)
			if (!nodeTotal[i].doneflag)
			{
				nodedoneflag=1;
				childtotal=0.0;
				childcount=0;
				for(j=0;j<numnodes;j++)
				{
					if (hierarchygraph[j*m->numnodes+i])   /* if j is a child of i */
					{
						if (!nodeTotal[j].doneflag)   /* j not done,  i isn't done.  */
							nodedoneflag=0;
						childtotal+=nodeTotal[j].size;
						nodeTotal[j].distance=physicalgraphminpath[j*numnodes+i];
						childcount++;
					}
				}
				if (nodedoneflag)
				{
					nodeTotal[i].doneflag=1;
					nodeTotal[i].rootflag=1;
					nodeTotal[i].size=(childtotal+1.0)*aggregation[childcount+1];

					for(j=0;j<numnodes;j++)
					{
						if (hierarchygraph[j*m->numnodes+i])   /* if j is a child of i */
							nodeTotal[j].rootflag=0;
					}
				}
				alldoneflag=0;
			}
		count++;
		if (count>numnodes)
			alldoneflag=1;
#if 0
		for(j=0;j<numnodes;j++)
			printf("node %d: %lf\n",m->nlist[j].addr & 0xFF,nodeTotal[j].total);
		printf("\n");
#endif
	}
	childtotal=0.0;
	count=0;
	baddata=0;
	for(i=0;i<numnodes;i++)
	{
		if (nodeTotal[i].rootflag)
			count++;
		else
		{
			if (nodeTotal[i].distance==0)
				baddata=1;
			childtotal+=nodeTotal[i].size * nodeTotal[i].distance;
		}
//		printf("node %d: done: %d root: %d total: %lf\n",m->nlist[i].addr & 0xFF,nodeTotal[i].doneflag,nodeTotal[i].rootflag,nodeTotal[i].total);
	}

	free(nodeTotal);
	if ((count!=numphysicalpartitions) || (baddata))
		return 0.0;
	return childtotal;
}

NodeLabel *nodeLabelApply(manetNode *us, NodeLabel *lab)
{
	NodeLabel *l;

	l=(NodeLabel*)malloc(sizeof(*l)+strlen(lab->text)+1);
	l->text=((char*)l)+sizeof(*l);
	l->priority=lab->priority;
	l->family=lab->family;
	l->tag=lab->tag;
	l->node=lab->node;
	strcpy(l->text,lab->text);
	memcpy(l->bgcolor,lab->bgcolor,4);
	memcpy(l->fgcolor,lab->fgcolor,4);

	if (lab->expiration)
		l->expiration=us->manet->curtime+lab->expiration;
	else
		l->expiration=0;

	l->next=us->labelList;
	us->labelList=l;

	return l;
}

/* Remove the single label pointed to by l
 */
void nodeLabelRemovePtr(manetNode *us, NodeLabel *l)
{
	NodeLabel *p,*q;

	p=us->labelList;
	q=NULL;

	while((p) && (p!=l))
	{
		q=p;
		p=p->next;
	}

	if (p==l)
	{
		if (q)
			q->next=p->next;
		else
			us->labelList=p->next;
		free(l);
	}
}

void nodeLabelRemove(manetNode *us, int bitmap, NodeLabel *n)
{
	NodeLabel *p,*q,*d;

	p=us->labelList;
	q=NULL;

	while(p)
	{
		if ( ((bitmap & COMMUNICATIONS_LABEL_REMOVE_TAG)?(p->tag==n->tag):1) &&
			((bitmap & COMMUNICATIONS_LABEL_REMOVE_FAMILY)?(p->family==n->family):1) &&
			((bitmap & COMMUNICATIONS_LABEL_REMOVE_PRIORITY)?(p->priority==n->priority):1) &&
			((bitmap & COMMUNICATIONS_LABEL_REMOVE_NODE)?(p->node==n->node):1)
			)
		{
			d=p;
			p=p->next;

			if (q)
				q->next=d->next;
			else
				us->labelList=d->next;
			free(d);
		}
		else
		{
			q=p;
			p=p->next;
		}
	}
}


/* Remove all the nodes which have the tag tag
 */
void nodeLabelRemoveTag(manetNode *us,NodeLabel *n)
{
	NodeLabel key;

	key.tag=n->tag;
	nodeLabelRemove(us,COMMUNICATIONS_LABEL_REMOVE_TAG,&key);
}

/* remove all the labels...
 */
void nodeLabelRemoveAll(manetNode *us)
{
	NodeLabel *p,*d;

	p=us->labelList;
	while(p)
	{
		d=p;
		p=p->next;
		free(d);
	}
	us->labelList=NULL;
}

/* remove any labels which are now timed out
 */
void nodeLabelTimeout(manetNode *us)
{
	NodeLabel *l,*d;

	l=us->labelList;

	while(l)
	{
		if ((l->expiration) && (l->expiration < us->manet->curtime))
		{
			d=l;
			l=l->next;
			nodeLabelRemovePtr(us,d);
		}
		else
			l=l->next;
	}
}

void nodeColor(manetNode *us, unsigned char *color)
{
	us->color=color;
}
