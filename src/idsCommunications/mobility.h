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

#ifndef MOBILITY_H
#define MOBILITY_H

#ifdef MODULE_MOBILITY

#include"des.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is a couple of mobility models
 *
 * to use, one just calls one on a node, and then it schedules itself from there
 *
 * Note that they depend on a number of variables in the node structure.
 *
 * The DES code needs a good way to add variables to a node structure on demand.
 */

typedef enum
{
	MOBILITYCONSTRAINT_NOFLY,
	MOBILITYCONSTRAINT_FLYONLY,
	MOBILITYCONSTRAINT_REPEL
} MobilityConstraintType;

struct MobilityConstraint;
typedef struct MobilityConstraint MobilityConstraint;
struct MobilityMove;
typedef struct MobilityMove MobilityMove;

typedef int MobilityConstraintCallback(manetNode *us, MobilityConstraint *con);
typedef int MobilityMoveCallback(manetNode *us, MobilityMove *mov);

/* we have 3 types of constraints right now.  They are implented with three 
 * different functions, the handler function pointer is effectively the type variable.
 */

typedef struct NoflyRect
{
	double x,y,width,height;
} NoflyRect;

typedef struct FlyonlyRect
{
	double minx,miny,maxx,maxy;
} FlyonlyRect;

typedef struct Repel
{
	double repelRadius;
} Repel;

struct MobilityConstraint
{
	MobilityConstraintCallback *handler;
	MobilityConstraintType type;
	int *applyTo;     /* if NULL, apply to all nodes.  otherwise, list of node indexes, terminated with a -1 */
	union
	{
		struct NoflyRect nofly;
		struct FlyonlyRect flyonly;
		struct Repel repel;
	} data;
	struct MobilityConstraint *next;
};

struct MobilityMove
{
	MobilityMoveCallback *handler;
	int rwx, rwy;
	double speed;
};

#define MOBILITY_MAXCONSTRAINT 10

/* per-node mobility state
 */
typedef struct mobilityState
{
	int rwx, rwy;     /* private vars for mobilityMove.  Need to be elsewhere */
	double speed;

	double ox, oy, oz;    /* previous location   */
	double dx, dy, dz;    /* current vector  */
	MobilityConstraint *constraint[MOBILITY_MAXCONSTRAINT];   /* end of list indicated by NULL ptr  */

	FILE *gpsDataFD;  		/* If you're not GPS mobility, you probably don't want to mess with this. */ 
	double xOffset, yOffset; 	/* Or this. */

} mobilityState;


typedef void MobilityCallback(manetNode *us);

typedef struct
{
	char *name;
	MobilityCallback *func;
} MobilityList;

/* per-manet mobility state
 */
typedef struct mobilityManetState
{
	double maxx, maxy;
	MobilityConstraint *constraintList;
	MobilityList *defaultMove;
	int period;
	int numInited;     /* number of nodes inited, used to know when to position the nodes */
} mobilityManetState;


void mobilityInit(manetNode *us);
/* sets up random number sequence for mobility.
** sets initial positions based on configuration info in manet.
*/

void nodeMobilityInitRandom(manet *m);
/* sets all the nodes to random locations.  Gets random number seed from config info
*/

void nodeMobilityInitRandomMinDist(manet *m);

void nodeMobilityInitFile(manet *m,char const *filname);
/* sets all the nodes to locations read from a file.  Gets filename from config info
*/

/* This one just goes to the right, in a straight line, forever
*/
void nodeMobilityLinear(manetNode *us);

/* This one does a random walk
*/
void nodeMobilityRandomWalk(manetNode *us);

/* random waypoint, with pause time of 0
*/
void nodeMobilityRandomWaypoint(manetNode *us);

/* random heading...  
** pick a random direction and speed.  
** go in it for n steps.  
** bounce off the walls if we hit them.
**
**  So really, its a variation of RandomWalk.
**
** recycles variables evily...
**   heading is in the varilable us->speed
**   number of ticks left before we choose a new heading is in us->rwx
**   speed is in us->rwy
*/
void nodeMobilityRandomHeading(manetNode *us);

/*
** A mobilty model that reads GPS coordinates from mane-gps
** files. 
*/
void nodeMobilityGPSData(manetNode *us);

/* Call this to count the number of edges in the graph, and 
** print the result on stdout.   (FIXME)
*/
void nodeMobilityCountEdges(manetNode *us, int schedflag);

/* Call this to dump the coordinates of the nodes every tick
*/
void nodeMobilityDumpCoords(manetNode *us, void *data);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_MOBILITY */
#endif /* !MOBILITY_H */
