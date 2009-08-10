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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#include "mobility.h"
#include "simulation.h"
#ifdef USE_RNG
#include "rng.h"
#endif
#include "node.h"
#include "watcherGPS.h"

static void nodeMobilityInit(manet *m);
static void mobilityTick(manetNode *us);     /* timer callback to update x,y with delta, and check constraints */

/* This is used only for mobility events.  
** any init must be done with a separate RNG, to make sure
** mobility is deterministic
*/
#ifdef USE_RNG
static RNG *nodeMobilityRandom;
#define RAND_U01() (nodeMobilityRandom->rand_u01())
#else
#define RAND_U01() (drand48())
#endif

#define NO_CONSTRAINT 		0	// mobility does not hit constraint
#define REVERSE_CONSTRAINT 	1	// mobility does hit constraint - reverse course to old position
#define CONTINUE_CONSTRAINT 	2	// mobility does hit constraint - continue to move to new position

static MobilityList moblist[]={
	{"mobility_randomwalk",nodeMobilityRandomWalk},
	{"mobility_randomwaypoint",nodeMobilityRandomWaypoint},
	{"mobility_randomheading",nodeMobilityRandomHeading},
	{"mobility_gpsdata",nodeMobilityGPSData},
	{"mobility_none",NULL},
	{NULL,NULL}
	};

/* load a list of non-negative integers from a config variable, and return a pointer to
 * an array of those integers, with the last entry being -1.
 */
static int *intListLoad(Config *conf, char *var)
{
	int *list=NULL;
	int count=0,size=0;
	const char *p;
	int i;

	p=configSearchStr(conf,var);
	if (p==NULL)
		return NULL;

	while(*p)
	{
		while((*p) && (isspace(*p)))      /* skip preceeding whitespace */
			p++;

		if (sscanf(p,"%d",&i)==1)
		{
			if (count<size)
				list[count++]=i;
			else
			{
				size=size?(size*2):10;
				list=(int*)realloc(list,size*sizeof(list[0]));   /* MAGIC: realloc of a NULL ptr==malloc, unless your library is broken.  */
				list[count++]=i;
			}
			while((*p) && isdigit(*p))   /* walk over number.   */
				p++;
			while((*p) && (isspace(*p) || (*p==',')))   /* and then over delimiter  */
				p++;
		}
	}
	if (count<size)
		list[count++]=-1;
	else
	{
		size=size+1;
		list=(int*)realloc(list,size*sizeof(list[0]));   /* MAGIC: realloc of a NULL ptr==malloc, unless your library is broken.  */
		list[count++]=-1;
	}

	return list;
}

/* search list of ints for a value.  If it is in the list, return its
 * index.  Otherwise return -1;
 */
static int intListSearch(int *list, int key)
{
	int i=0;
	for(i=0;list[i]>=0;i++)
		if (list[i]==key)
			return i;
	return -1;
}


/* constraint callback functions
 * They return REVERSE_CONSTRAINT on violation of the constraint, and adjust the motion vector fields in the node
 */
static int mobilityConstraintNofly(manetNode *us, MobilityConstraint *con)
{
	if ((us->x >= con->data.nofly.x) && (us->x <= (con->data.nofly.x + con->data.nofly.width)) &&
	    (us->y >= con->data.nofly.y) && (us->y <= (con->data.nofly.y + con->data.nofly.height)))
	{
		if ((us->mobility->ox < con->data.nofly.x) && (us->x > con->data.nofly.x))
		{
//			us->x=nf->x;
			us->mobility->dx=-us->mobility->dx;
			return REVERSE_CONSTRAINT;
		}

		if ((us->mobility->ox > (con->data.nofly.x+con->data.nofly.width)) && (us->x < (con->data.nofly.x+con->data.nofly.width)))
		{
//			us->x=nf->x+nf->width;
			us->mobility->dx=-us->mobility->dx;
			return REVERSE_CONSTRAINT;
		}
		if ((us->mobility->oy < con->data.nofly.y) && (us->y > con->data.nofly.y))
		{
//			us->y=nf->y;
			us->mobility->dy=-us->mobility->dy;
			return REVERSE_CONSTRAINT;
		}
		if ((us->mobility->oy > (con->data.nofly.y+con->data.nofly.height)) && (us->y < (con->data.nofly.y+con->data.nofly.height)))
		{
//			us->y=nf->y+nf->height;
			us->mobility->dy=-us->mobility->dy;
			return REVERSE_CONSTRAINT;
		}
	}
	return NO_CONSTRAINT;
}

static int mobilityConstraintFlyonly(manetNode *us, MobilityConstraint *con)
{
	if (us->x > con->data.flyonly.maxx)
	{
		us->mobility->dx=-us->mobility->dx;
		us->mobility->ox=con->data.flyonly.maxx-1;	// be very bad - force node into the flyonly zone
		return REVERSE_CONSTRAINT;
	}
	if (us->x < con->data.flyonly.minx)
	{
		us->mobility->dx=-us->mobility->dx;
		us->mobility->ox=con->data.flyonly.minx+1;	// be very bad - force node into the flyonly zone
		return REVERSE_CONSTRAINT;
	}
	if (us->y > con->data.flyonly.maxy)
	{
		us->mobility->dy=-us->mobility->dy;
		us->mobility->oy=con->data.flyonly.maxy-1;	// be very bad - force node into the flyonly zone
		return REVERSE_CONSTRAINT;
	}
	if (us->y < con->data.flyonly.miny)
	{
		us->mobility->dy=-us->mobility->dy;
		us->mobility->oy=con->data.flyonly.miny+1;	// be very bad - force node into the flyonly zone
		return REVERSE_CONSTRAINT;
	}
	return NO_CONSTRAINT;
}

static int mobilityConstraintRepel(manetNode *us, MobilityConstraint *con)
{
	int i;
	int repelflag=0;
	double repelx=0.0, repely=0.0;

	for(i=0;i<us->manet->numnodes;i++)
	{
		if (i==us->index)
			continue;

		if ((fabs(us->x - us->manet->nlist[i].x) < con->data.repel.repelRadius) && (fabs(us->y - us->manet->nlist[i].y)<con->data.repel.repelRadius))
		{
			repelflag=1;
			repelx=us->manet->nlist[i].x;
			repely=us->manet->nlist[i].y;
		}
	}
	if (repelflag)
	{
		double heading=atan2(us->x - repelx, (us->y - repely));
		us->mobility->dx=sin(heading)*us->mobility->speed;
		us->mobility->dy=cos(heading)*us->mobility->speed;
		return CONTINUE_CONSTRAINT;
	}
	return NO_CONSTRAINT;
}


/* load a list of constraints from a config file
 */
static MobilityConstraint *mobilityConstraintLoad(Config *conf)
{
	MobilityConstraint *list=NULL, *mc;
	int i;
	char const *con;
	char conKey[32],nodeListKey[32];
	
	i=0;
	while(1)
	{
		i++;
		snprintf(conKey,sizeof(conKey),"mobility_nofly%d",i);
		snprintf(nodeListKey,sizeof(nodeListKey),"mobility_nofly%d_nodes",i);
		con=configSearchStr(conf,conKey);
		if (con)
		{
			mc=(MobilityConstraint*)malloc(sizeof(*mc));
			mc->handler=mobilityConstraintNofly;
			mc->type=MOBILITYCONSTRAINT_NOFLY;
			mc->applyTo=intListLoad(conf,nodeListKey);
			sscanf(con,"%lf,%lf,%lf,%lf",&mc->data.nofly.x,&mc->data.nofly.y,&mc->data.nofly.width,&mc->data.nofly.height);

			mc->next=list;
			list=mc;
		}
		else
			break;
	}
	i=0;
	while(1)
	{
		i++;
		snprintf(conKey,sizeof(conKey),"mobility_flyonly%d",i);
		snprintf(nodeListKey,sizeof(nodeListKey),"mobility_flyonly%d_nodes",i);
		con=configSearchStr(conf,conKey);
		if (con)
		{
			double x,y,wid,heit;
			mc=(MobilityConstraint*)malloc(sizeof(*mc));
			mc->handler=mobilityConstraintFlyonly;
			mc->type=MOBILITYCONSTRAINT_FLYONLY;
			mc->applyTo=intListLoad(conf,nodeListKey);
			sscanf(con,"%lf,%lf,%lf,%lf",&x,&y,&wid,&heit);

			mc->data.flyonly.minx=x;
			mc->data.flyonly.miny=y;
			mc->data.flyonly.maxx=x+wid;
			mc->data.flyonly.maxy=y+heit;

			mc->next=list;
			list=mc;
		}
		else
			break;
	}
	i=0;
	while(1)
	{
		i++;
		snprintf(conKey,sizeof(conKey),"mobility_repel%d",i);
		snprintf(nodeListKey,sizeof(nodeListKey),"mobility_repel%d_nodes",i);
		con=configSearchStr(conf,conKey);
		if (con)
		{
			mc=(MobilityConstraint*)malloc(sizeof(*mc));
			mc->handler=mobilityConstraintRepel;
			mc->type=MOBILITYCONSTRAINT_REPEL;
			mc->applyTo=intListLoad(conf,nodeListKey);
			sscanf(con,"%lf",&mc->data.repel.repelRadius);

			mc->next=list;
			list=mc;
		}
		else
			break;
	}
	return list;
}

static void mobilityConstraintInit(manetNode *us)
{
	MobilityConstraint *con;
	int numcon=0;

	memset(us->mobility->constraint, 0, sizeof(us->mobility->constraint));
	for(con=us->manet->mobility->constraintList;con!=NULL;con=con->next)
	{
		if ((con->applyTo==NULL) || (intListSearch(con->applyTo,us->index)>=0))
		{
			if (numcon<(MOBILITY_MAXCONSTRAINT-1))
				us->mobility->constraint[numcon++]=con;
		}
	}
	timerSet(us,(eventCallback*)mobilityTick,us->manet->mobility->period,NULL);
}

/* called per node 
 */
void mobilityInit(manetNode *us)
{
	MobilityList *mob,*ml;
	char addr[32];
	char const *val;

	if (us->manet->mobility==NULL)                    /* need per-manet config?   */
		nodeMobilityInit(us->manet);

	/* determine our mobilityMove   */
	mob=us->manet->mobility->defaultMove;

	sprintf(addr,"%d",us->addr & 0xFF);
	for(ml=moblist;ml->name;ml++)
	{
		val=configSearchStr(us->manet->conf,ml->name);
		if ((val!=NULL) && (strstr(val,addr)))
			mob=ml;
	}

	if ((mob) && (mob->func))
	{
		(mob->func)(us);
	}

	us->manet->mobility->numInited++;

	if (us->manet->mobility->numInited==us->manet->numnodes)
	{
		char const *initpos;
		char const *locfile;
		int flag=0;

		initpos=configSearchStr(us->manet->conf,"mobilityinitposition");
		if (initpos==NULL)
		{
			fprintf(stderr,"mobility: need to specify mobilityinitposition!\n");
			exit(1);
		}

		locfile=configSearchStr(us->manet->conf,"mobilityinitfile");
		if (locfile)
		{
			char fullpath[PATH_MAX];
			if (0 == configGetPathname(us->manet->conf,
						   locfile,
						   fullpath,
						   sizeof(fullpath)))
			{
				nodeMobilityInitFile(us->manet,fullpath);
				flag=1;
			}
			else
			{
				fprintf(stderr,"mobility: configGetPathname(%s) failed!\n",
					locfile);
				exit (1);
			}
		}

		if (strcasecmp(initpos,"random")==0)
		{
			nodeMobilityInitRandom(us->manet);
			flag=1;
		}
		if (strcasecmp(initpos,"randommindist")==0)
		{
			nodeMobilityInitRandomMinDist(us->manet);
			flag=1;
		}

		if (!flag)
		{
			fprintf(stderr,"mobility: no initial locations!\n");
			exit(1);
		}
	}
}

/* called per manet, on the first node.  (which may not be node index 0)
 */
static void
nodeMobilityInit(manet *m)
{
	MobilityConstraint *arena;

#ifdef USE_RNG
	nodeMobilityRandom=new RNG(configSearchInt(m->conf,"mobilityseed"));
#endif
	char const *val;
	MobilityList *ml;

	m->mobility=(mobilityManetState*)malloc(sizeof(*m->mobility));

	m->mobility->maxx=configSearchInt(m->conf,"areaX");
	m->mobility->maxy=configSearchInt(m->conf,"areaY");
	m->mobility->period=configSetInt(m->conf,"mobilityperiod",1000);
	m->mobility->constraintList=mobilityConstraintLoad(m->conf);
	m->mobility->numInited=0;

	/* build flyonly constraint for the arena  */
	arena=(MobilityConstraint*)malloc(sizeof(*arena));
	arena->handler=mobilityConstraintFlyonly;
	arena->type=MOBILITYCONSTRAINT_FLYONLY;
	arena->applyTo=NULL;   /* IE: all nodes  */
	arena->data.flyonly.minx=0;
	arena->data.flyonly.miny=0;
	arena->data.flyonly.maxx=m->mobility->maxx;
	arena->data.flyonly.maxy=m->mobility->maxy;

	arena->next=m->mobility->constraintList;
	m->mobility->constraintList=arena;

	/* determine default mobility model
	 */
	for (ml=moblist;ml->name;ml++)
	{
		val=configSearchStr(m->conf,ml->name);
		if ((val!=NULL) && (strstr(val,"default")))
			m->mobility->defaultMove=ml;
	}

}

/* nominal callback called once per period to update coordinates and check
 * constraints.
 */
static void mobilityTick(manetNode *us)
{
	int i;

	timerSet(us,(eventCallback*)mobilityTick,us->manet->mobility->period,NULL);

	us->mobility->ox=us->x;
	us->mobility->oy=us->y;
	us->mobility->oz=us->z;
	us->x+=us->mobility->dx;
	us->y+=us->mobility->dy;
	us->z+=us->mobility->dz;

	for(i=0;us->mobility->constraint[i]!=NULL;i++)
	{
		int constraintReturn = us->mobility->constraint[i]->handler(us,us->mobility->constraint[i]);
		if (constraintReturn)
		{
			us->x=us->mobility->ox;
			us->y=us->mobility->oy;
			us->z=us->mobility->oz;

			if(constraintReturn==CONTINUE_CONSTRAINT) // stuck inside constraint - attempt to break out
			{
				us->x+=us->mobility->dx;
				us->y+=us->mobility->dy;
				us->z+=us->mobility->dz;
			}

			return;
		}
	}
}

static int constraintCheck(manetNode *us)
{
	int i;

	if (us->mobility==NULL)
		return NO_CONSTRAINT;

	for(i=0;us->mobility->constraint[i]!=NULL;i++)
	{
		if (us->mobility->constraint[i]->handler(us,us->mobility->constraint[i]))
			return REVERSE_CONSTRAINT;
	}
	return NO_CONSTRAINT;
}

#if 0
/* If the given coords are inside a nofly zone, return that nofly zone.  Otherwise return NULL
 * 
 *  We may eventually have a couple of different shapes...
 */
static mobilityNofly *mobilityNoflyInside(manetNode *us)
{
	mobilityNofly *nf;

	for(nf=us->manet->mobility->nofly;nf;nf=nf->next)
	{
		if ((us->x >= nf->x) && (us->x <= (nf->x + nf->width)) &&
		    (us->y >= nf->y) && (us->y <= (nf->y + nf->height)))
			return nf;
	}
	return NULL;
}
#endif
/* Place all the nodes randomly...  
 */
void
nodeMobilityInitRandom(manet *m)
{
	int i;
#ifdef USE_RNG
	RNG *rnd;     /* this is a private PRNG, just for laying out the nodes  */

	rnd=new RNG(configSearchInt(m->conf,"mobilityinitseed"));
#endif

        for(i=0;i<m->numnodes;i++)
	{
		if (m->nlist[i].x>0.0)
			continue;
		fprintf(stderr,"positioning node %d\n",i);

		do
		{
			m->nlist[i].x=RAND_U01()*m->mobility->maxx;
			m->nlist[i].y=RAND_U01()*m->mobility->maxy;
			m->nlist[i].z=0;
		} while (constraintCheck(&(m->nlist[i]))!=0);
	}

#ifdef USE_RNG
	delete rnd;
#endif
}

/* place all the nodes randomly, unless a node is too close to
 * an already existing node.  Then re-roll and try again
 */
void
nodeMobilityInitRandomMinDist(manet *m)
{
	int i,j;
#ifdef USE_RNG
	RNG *rnd;
	rnd=new RNG(configSearchInt(m->conf,"mobilityinitseed"));
#endif
	double mindist,tmp,dist;

	mindist=configSearchDouble(m->conf,"mobilityinitmindistance");
	if (mindist <1.0)
		mindist=1.0;

	fprintf(stderr,"mindist== %lf\n", mindist);
	fprintf(stderr,"maxx== %lf\n", m->mobility->maxx);
	fprintf(stderr,"maxy== %lf\n", m->mobility->maxy);
        for(i=0;i<m->numnodes;i++)
	{
		if (m->nlist[i].x>0)
			continue;

		do
		{
			m->nlist[i].x=RAND_U01()*m->mobility->maxx;
			m->nlist[i].y=RAND_U01()*m->mobility->maxy;
			m->nlist[i].z=0;

			dist=mindist;
			for(j=0;j<i;j++)
			{
				tmp=hypot(m->nlist[i].x - m->nlist[j].x, m->nlist[i].y - m->nlist[j].y);
				if ((j==0) | (tmp < dist))
					dist=tmp;
			}
		} while((dist < mindist) || (constraintCheck(&(m->nlist[i]))!=0));
	}

#ifdef USE_RNG
	delete rnd;
#endif
}

void
nodeMobilityInitFile(manet *m, char const *filname)
{
	FILE *fil;
	char line[8192];
	char *p,*inp;
	int i=0,indx;
	unsigned int addr;
	double x,y,z;

	fil=fopen(filname,"r");

	if (fil==NULL)
	{
		fprintf(stderr,"nodeMobilityInitFile: could not open location file.  errno= %d %s\n",errno,strerror(errno));
		return;
	}

	while(fgets(line,sizeof(line),fil))
	{
		if ((line[0]=='#'))
			continue;

		inp=line;
		while(isspace(*inp))
			inp++;
		p=strsep(&inp," \t");

		addr=0;
		addr=ntohl(inet_addr(p));
		fprintf(stderr,"p= %s addr= %d.%d.%d.%d\n",p, PRINTADDR(addr));
		if (addr!=0xFFFFFFFF)
		{
			if ((indx=manetGetNodeNum(m,addr))>=0)   /* do we already have a record for this addr?  */
			{
			}
			else
			{
				m->nlist[i].addr=addr;     /* no existing record, make a new one...  */
				indx=i;
				i++;
			}
			
			while(isspace(*inp))
				inp++;
			p=strsep(&inp," \t");
			while(isspace(*inp))
				inp++;

			if (strcmp(p,"at")==0)
			{
				p=strsep(&inp," \t");
				sscanf(p,"%lf",&x);
				while(isspace(*inp))
					inp++;
				p=strsep(&inp," \t");
				sscanf(p,"%lf",&y);
				if (inp)
				{
					while(isspace(*inp))
						inp++;
					p=strsep(&inp," \t");
					sscanf(p,"%lf",&z);
				}
				else
					z=0.0;

				m->nlist[indx].x=x;
				m->nlist[indx].y=y;
				m->nlist[indx].z=z;
			}
			if (strcmp(p,"netmask")==0)
			{
				p=strsep(&inp," \t");
				m->nlist[indx].netmask=ntohl(inet_addr(p));
			}
			if (strcmp(p,"broadcast")==0)
			{
				p=strsep(&inp," \t");
				m->nlist[indx].bcastaddr=ntohl(inet_addr(p));
			}
		}
	}

	fclose(fil);
}

/* Callback functions which implement mobility
*/

/* This one just goes to the right, in a straight line, forever
*/
void
nodeMobilityLinear(manetNode *us)
{
	timerSet(us,(eventCallback*)nodeMobilityLinear,1000,NULL);

	us->x+=1;
}

/* This one does a random walk
*/
static void nodeMobilityRandomWalkInternal(manetNode *us)
{
	int noise;

	noise=(int)(RAND_U01()*100.0)-50;
	timerSet(us,(eventCallback*)nodeMobilityRandomWalkInternal,1000+noise,NULL);

	us->mobility->dx=floor(RAND_U01()*7.0)-3.0;
	us->mobility->dy=floor(RAND_U01()*7.0)-3.0;
        us->mobility->dz=0;
}

void nodeMobilityRandomWalk(manetNode *us)
{
	nodeMobilityRandomWalkInternal(us);
	mobilityConstraintInit(us);
}

/* random waypoint, with pause time of 0
*/
static void nodeMobilityRandomWaypointInternal(manetNode *us)
{
	int noise;

	noise=(int)(RAND_U01()*100.0)-50;

	timerSet(us,(eventCallback*)nodeMobilityRandomWaypointInternal,1000+noise,NULL);

	if (((us->mobility->rwx==0) && (us->mobility->rwy==0)) || ((fabs(us->x-us->mobility->rwx)<4.0) && (fabs(us->y-us->mobility->rwy)<4.0)))   /* we're there, pick a new destination */
	{
		us->mobility->rwx=(int)(RAND_U01()*us->manet->mobility->maxx);
		us->mobility->rwy=(int)(RAND_U01()*us->manet->mobility->maxy);
		us->mobility->speed=configSearchDouble(us->manet->conf,"mobilityspeed");
	}
	else
	{
		double heading=atan2(us->mobility->rwx-us->x,us->mobility->rwy-us->y);
		us->mobility->dx=sin(heading)*us->mobility->speed;
		us->mobility->dy=cos(heading)*us->mobility->speed;
		us->mobility->dz=0;
	}
}

/* note: untested (just fixed a couple of major bugs).  -dkindred 2007-07-23
 */
void nodeMobilityRandomWaypoint(manetNode *us)
{
	us->mobility=(mobilityState*)malloc(sizeof(*(us->mobility)));
	us->mobility->rwx=0;
	us->mobility->rwy=0;
	nodeMobilityRandomWaypointInternal(us);
	mobilityConstraintInit(us);
}

/* random heading...  
** pick a random direction and speed.  
** go in it for n steps.  
** bounce off the walls if we hit them.
**
** recycles variables evily...
**   heading is in the variable us->speed
**   number of ticks left before we choose a new heading is in us->rwx
**   speed is in us->rwy
*/
static void nodeMobilityRandomHeadingInternal(manetNode *us)
{
	double heading;
	int noise;

#ifdef USE_RNG
	if (nodeMobilityRandom==NULL)
		return;
#endif
	noise=(int)(RAND_U01()*100.0)-50;

	timerSet(us,(eventCallback*)nodeMobilityRandomHeadingInternal,1000+noise,NULL);
//	printf("mobility: time %d node %d\n",us->manet->curtime,us->addr);

	if (us->mobility->rwx==0)
	{
		us->mobility->rwx=(int)(RAND_U01()*100.0)+10;

		heading=RAND_U01()*3.14159263*2.0;
		us->mobility->dx=sin(heading)*us->mobility->speed;
		us->mobility->dy=cos(heading)*us->mobility->speed;
                us->mobility->dz=0;
	}
	us->mobility->rwx--;
}

void nodeMobilityRandomHeading(manetNode *us)
{
	us->mobility=(mobilityState*)malloc(sizeof(*(us->mobility)));
	us->mobility->rwx=0;
	us->mobility->rwy=0;
	us->mobility->speed=configSearchDouble(us->manet->conf,"mobilityspeed");
	nodeMobilityRandomHeadingInternal(us);
	mobilityConstraintInit(us);
}

static void nodeMobilityGPSDataInternal(manetNode *us, void *)
{
	if(NULL == us->mobility->gpsDataFD) /* we're hosed */
	{
		fprintf(stderr, "No open file descriptor for gps data for node %d\n", us->addr & 0xFF);
		return; 
	}

	char line[128];
	memset(line, 0, sizeof(line)); 

	if(NULL == fgets(line, sizeof(line), us->mobility->gpsDataFD))
	{
		fprintf(stderr, "Error reading a line from the GPS data file for node %d", us->addr & 0xFF);
		return; 
	}

	char *newline = strrchr(line, '\n'); 
	if(newline) *newline='\0'; 

	char *tsStr=NULL, *latStr=NULL, *lonStr=NULL, *altStr=NULL;

	tsStr=strtok(line, ",");
	lonStr=strtok(NULL, ","); 
	latStr=strtok(NULL, ","); 
	altStr=strtok(NULL, ","); 

	if(!tsStr || !latStr || !lonStr || !altStr)
	{
		fprintf(stderr, "Error parsing gps data file for node %d. (Only spec files of the format \"id,x,y,z\" supprted", us->addr & 0xFF); 
		exit(EXIT_FAILURE);
	}

	int ts=-1;
	double lat=0.0, lon=0.0, alt=0.0;

	char *endPtr=NULL;
	ts=strtol(tsStr, &endPtr, 10); 
	if(ts==LONG_MIN || ts==LONG_MAX || endPtr==tsStr) { fprintf(stderr, "Error parsing timestamp in GPS file"); return; }

	if(0 == (lat=strtod(latStr,&endPtr)) && (endPtr==latStr || errno==ERANGE)) { fprintf(stderr, "Error parsing lat value in GPS file for node %d", us->addr & 0xFF); return; }
	if(0 == (lon=strtod(lonStr,&endPtr)) && (endPtr==lonStr || errno==ERANGE)) { fprintf(stderr, "Error parsing lon value in GPS file for node %d", us->addr & 0xFF); return; }
	if(0 == (alt=strtod(altStr,&endPtr)) && (endPtr==altStr || errno==ERANGE)) { fprintf(stderr, "Error parsing alt value in GPS file for node %d", us->addr & 0xFF); return; }

	// fprintf(stderr, "lat=%f, lon=%f, alt=%f\n", lat, lon, alt); 

	us->x=(lon*80000)+us->mobility->xOffset;
	us->y=(lat*80000)+us->mobility->yOffset;
	us->z=alt;

	/* If everyone has coords, compute the offsets once (so nodes show on display) */
	if(!us->mobility->xOffset)
	{
		/* Kind of cheating in that I'm looking at the other node's structs.           */
		/* If we are the last one (everyone else has coords already), compute the offsets and set everyone's offset value */
		int i=0;
		for( ; i < us->manet->numnodes; i++)
		{
			if(!us->manet->nlist[i].x && !us->manet->nlist[i].y) break;
		}
		if(i == us->manet->numnodes)	/* Everyone has coords */
		{
			double minX=us->manet->nlist[0].x, minY=us->manet->nlist[0].y;
			for(i=0; i < us->manet->numnodes; i++)
			{
				minX = minX > us->manet->nlist[i].x ? us->manet->nlist[i].x : minX; 
				minY = minY > us->manet->nlist[i].y ? us->manet->nlist[i].y : minY; 
			}
			for(i=0; i < us->manet->numnodes; i++)
			{
				us->manet->nlist[i].mobility->xOffset=-minX;
				us->manet->nlist[i].mobility->yOffset=-minY;

				fprintf(stderr, "Computed offsets: x=%f, y=%f", minX, minY); 
			}
		}
	}

	/* read the next line to determine when to execute the callback */
	if(NULL != fgets(line, sizeof(line), us->mobility->gpsDataFD))
	{
		if(NULL != (tsStr=strtok(line, ",")))
		{
			int ts2=strtol(tsStr, &endPtr, 10);
			if(ts2==LONG_MIN || ts2==LONG_MAX || endPtr==tsStr) { fprintf(stderr, "Error parsing timestamp in GPS file"); return; }

			timerSet(us,nodeMobilityGPSDataInternal,(ts2-ts)*1000,NULL);
		}
	}
	else
	{
		int repeat=0;
		repeat=configSearchInt(us->manet->conf, "mobility_gpsRepeat");
		if(repeat)
		{
			rewind(us->mobility->gpsDataFD); 
			timerSet(us,nodeMobilityGPSDataInternal,1000,NULL);
		}
		else
		{
			fclose(us->mobility->gpsDataFD); 
		}
		
		fprintf(stderr, "Out of GPS data for node %d: %s", us->addr & 0xFF, repeat ? "starting movement over" : "stopping movement"); 
	}
}

void nodeMobilityGPSData(manetNode *us)
{
	char fullpath[PATH_MAX];
	FILE *gpsConf_fd=NULL;
	us->mobility=(mobilityState*)malloc(sizeof(*(us->mobility)));
	memset(us->mobility, 0, sizeof(us->mobility)); 

	const char *gpsConfFile=configSearchStr(us->manet->conf,"mobility_gpsConfigFile");

	if(!gpsConfFile)
	{
		fprintf(stderr, "mobility_gpsConfigFile not found");
		exit(EXIT_FAILURE); 
	}

	if (0 != configGetPathname(us->manet->conf,
				   gpsConfFile,
				   fullpath,
				   sizeof(fullpath)))
	{
		fprintf(stderr,"mobility: configGetPathname(%s) failed!\n",
			gpsConfFile);
		exit (1);
	}

	if(NULL == (gpsConf_fd = fopen(fullpath, "r")))
	{
		fprintf(stderr, "Unable to open %s for reading", fullpath);
		exit(EXIT_FAILURE);
	}

	/* conf file line example: 1,192.168.2.101.log */
	/* Format: ID,FILENAME */
	char line[128];
	while(NULL != fgets(line, sizeof(line), gpsConf_fd))
	{
		char *id=NULL, *filename=NULL;
		id=strtok(line, ",");
		filename=strtok(NULL, ","); 

		if(!id || !filename) 
		{
			fprintf(stderr, "Error parsing file %s", fullpath); 
			continue;
		}

		/* ID must be last octet of addr for now - this is different than default mane-gpse! */
		if((us->addr & 0xFF) == (unsigned int)strtol(id, NULL, 10))
		{
			char *newline = strchr(filename, '\n');
			if(newline) *newline='\0'; 
			if(NULL == (us->mobility->gpsDataFD = fopen(filename, "r")))
			{
				fprintf(stderr, "Unable to open %s for reading", filename); 
				exit(EXIT_FAILURE); 
			}
			
			break;
		}
	}
	
	if(NULL == us->mobility->gpsDataFD)
	{
		fprintf(stderr, "No GPS data given for node %d. Exiting\n", us->addr & 0xFF);
		exit(EXIT_FAILURE); 
	}
	
	/* There is a better place for this */
	for(int i=0; i < us->manet->numnodes; i++) 
	{
		us->manet->nlist[i].x=0.0;
		us->manet->nlist[i].y=0.0;
	}
	nodeMobilityGPSDataInternal(us, NULL); 
	
}

/* Callback function, for rescheduling the nodeMobilityCountEdges callback
 * when it is supposed to be rescheduled
 */
static void nodeMobilityCountEdgesCallback(manetNode *us, void *)
{
	nodeMobilityCountEdges(us,1);
}

/* This is called after every tick, and counts the number of edges
** in the MANET.  It then reports the absolute delta between the number
** of edges, and the last number of edges.  Thus our definition of a
** mobility event: the addition or removal of an edge.
**
** Then, put some code here to log the mobility events, so we can run them
** as a scenario on a tealab.
*/
void nodeMobilityCountEdges(manetNode *us, int schedflag)
{
	manet *m=us->manet;
	int i,j;
	static int *lastgraph=NULL;
	int *graph;
	int n=m->numnodes;
	int event=0;

	if (schedflag)
		tickSet(us,nodeMobilityCountEdgesCallback,NULL);

	graph=manetGetPhysicalGraph(us->manet);

		for(i=0;i<n;i++)
			for(j=0;j<i;j++)
				if (graph[i*n+j] != ((lastgraph!=NULL)?lastgraph[i*n+j]:0))
				{
#if 1
					printf("mobility: time %lld %s %d to %d \n",us->manet->curtime,graph[i*n+j]?"make":"break",m->nlist[i].addr & 0xFF ,m->nlist[j].addr & 0xFF);
#endif
					event++;
				}
			
	if (lastgraph)
		free(lastgraph);
	lastgraph=graph;

	if (event>0)
		printf("mobility events: %d  time: %lld\n",event,us->manet->curtime);
}

void nodeMobilityDumpCoords(manetNode *us, void *)
{
	int i;

	tickSet(us,nodeMobilityDumpCoords,NULL);

	for(i=0;i<us->manet->numnodes;i++)
		printf("mobility: time %lld node %d at %lf %lf\n",us->manet->curtime,us->manet->nlist[i].addr & 0xFF,us->manet->nlist[i].x,us->manet->nlist[i].y);
}


/* The problem:  how to measure the number of mobility events per time unit?

The current implementation is an individually schedule event per object which updates
its location.  possibly creating and deleting edges as it does.
These events are completely independant, and may occur at the same instant.  But,we
don't want to count an edge as created in one event, when the next event (in the
same instant) would delete it.

So, how about all the mobility being done in one single event?  
Thus, we count all the edges.  run all the mobility events, and then count all
the edges again.  That does mean that all mobility occurs in lock step.  All we
really care about is when the clock moves.  We want to count edges and look
for delta every time the clock ticks.  The current behavior with separate events
per node is very nice, in that its extremely compartmentalized...

Or, how about we just count all the edges every n ticks?  If an edge is created
then destroyed during those n ticks, neither occurance will be observed.  
Does it matter if no packets are sent?    (I think it does...)

The final implementation:  
We count edges every time the clock ticks.  Edges which are created and destroyed
in an instant (when the clock does not tick) don't matter, since a packet can 
not be sent in an instant.

*/
