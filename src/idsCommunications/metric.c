#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "metric.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: metric.c,v 1.11 2006/06/14 15:04:34 tjohnson Exp $";

metric *metricCreate(void)
{
	metric *m;
	int maxsamples=100;

	m=(metric*)malloc(sizeof(*m));
	m->sample=(double*)malloc(sizeof(m->sample[0])*maxsamples);
	m->maxsamples=maxsamples;
	m->numsamples=0;

	return m;
}

void metricFree(metric *m)
{
	free(m->sample);
	free(m);
}

void metricClear(metric *m)
{
	m->numsamples=0;
}

void metricWrite(metric *m, double val)
{
	if (m->numsamples>=(m->maxsamples-1))
	{
		m->maxsamples*=2;
		m->sample=(double*)realloc(m->sample,sizeof(m->sample[0])*m->maxsamples);
	}
	m->sample[m->numsamples++]=val;
}

int metricCount(metric *m)
{
	return m->numsamples;
}

double metricMean(metric *m)
{
	int i;
	double mean=0.0;

	for(i=0;i<m->numsamples;i++)
		mean+=m->sample[i];

	if (m->numsamples>0)
		return mean/(double)m->numsamples;
	else
		return 0.0;
}

double metricMin(metric *m)
{
	double min;
	int i;

	if (m->numsamples<1)
		return 0.0;

	min=m->sample[0];
	for(i=1;i<m->numsamples;i++)
		if (m->sample[i]<min)
			min=m->sample[i];
	return min;
}

double metricMax(metric *m)
{
	double max;
	int i;

	if (m->numsamples<1)
		return 0.0;

	max=m->sample[0];
	for(i=1;i<m->numsamples;i++)
		if (m->sample[i]>max)
			max=m->sample[i];
	return max;
}

double metricStdDev(metric *m)
{
	int i;
	double std1=0.0,std2=0.0;

	for(i=0;i<m->numsamples;i++)
	{
		std1+=(m->sample[i]*m->sample[i]);
		std2+=m->sample[i];
	}
	return sqrt((std1 - ((std2*std2)/(double)m->numsamples))/(double)(m->numsamples-1));
}

double metricTotal(metric *m)
{
	double tot=0.0;
	int i;

	if (m->numsamples<1)
		return 0.0;

	for(i=1;i<m->numsamples;i++)
		tot+=m->sample[i];
	return tot;
}

void metricDump(metric *m, char *label, FILE *fil)
{
	int i;

	for(i=0;i<m->numsamples;i++)
		fprintf(fil,"%s: %lf\n",label,m->sample[i]);
}
