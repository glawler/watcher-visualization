#ifndef METRIC_H
#define METRIC_H

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: metric.h,v 1.8 2006/06/14 15:04:34 tjohnson Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int numsamples;
	int maxsamples;
	double *sample;
} metric;

/* A basic data structure to gather a series of observations
** and then compute some simple statistics
*/

metric *metricCreate(void);
void metricFree(metric *m);
void metricClear(metric *);      /* delete all the data points, but do not free the memory.  */

void metricWrite(metric *m, double val);   /* Add a data point  */


double metricMean(metric *);     /* return the mean  */
double metricMin(metric *);      /* return the minimum data point */
double metricMax(metric *);      /* return the maximum data point */
double metricStdDev(metric *);   /* return the standard deviation of the points */
double metricTotal(metric *m);   /* return the sum of all the points */
int metricCount(metric *m);   /* return the number of points */

void metricDump(metric *m, char *label, FILE *fil);    /* print all the values in the metric, labeled with *label, to file *fil */

#ifdef __cplusplus
};
#endif

#endif
