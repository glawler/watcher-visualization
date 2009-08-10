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

#ifndef METRIC_H
#define METRIC_H

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
