#include <stdlib.h>
#include <GL/glut.h>
#include "graphics.h"
#include "watcherGraph.h"

static const char *rcsid __attribute ((unused)) = "$Id: watcherGraph.cpp,v 1.11 2006/09/25 17:26:18 tjohnson Exp $";

/*
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

void watcherGraphEdgeInsert(NodeEdge **g, NodeEdge *nw, destime curtime)
{
	watcherGraphEdgeRemove(g,COMMUNICATIONS_EDGE_REMOVE_ALL,nw);

	nw->next=*g;
	*g=nw;

	/* check the expire time, and convert it to an absolute timestamp if its non-zero  */
	if (nw->expiration>0)
		nw->expiration+=curtime;
}

void watcherGraphEdgeRemove(NodeEdge **g, int bitmap, NodeEdge *nw)
{
	NodeEdge *p, *q,*d;

	p=*g;
	q=NULL;

	while(p)
	{
		if (((bitmap&COMMUNICATIONS_EDGE_REMOVE_HEAD)?(p->head==nw->head):1) &&
			((bitmap&COMMUNICATIONS_EDGE_REMOVE_TAIL)?(p->tail==nw->tail):1) &&
			((bitmap&COMMUNICATIONS_EDGE_REMOVE_FAMILY)?(p->family==nw->family):1) &&
			((bitmap&COMMUNICATIONS_EDGE_REMOVE_PRIORITY)?(p->priority==nw->priority):1) &&
			((bitmap&COMMUNICATIONS_EDGE_REMOVE_TAG)?(p->tag==nw->tag):1)
			  )
			{
				d=p;
				p=p->next;

				if (q)
					q->next=d->next;
				else
					*g=d->next;
				free(d);
			}
			else
			{
				q=p;
				p=p->next;
			}
	}
}

void watcherGraphEdgeRemoveFamily(NodeEdge **g,int family)
{
	NodeEdge key;

	key.family=family;
	watcherGraphEdgeRemove(g,COMMUNICATIONS_EDGE_REMOVE_FAMILY,&key);
}

void watcherGraphEdgeNuke(NodeEdge **g)      /* remove all edges  */
{
	watcherGraphEdgeRemove(g,0,NULL);
}

static void watcherLabelDraw(GLdouble x, GLdouble y, GLdouble z, GLdouble scale, NodeLabel *l, int monochromeMode)
{
	GLdouble h,w;
	GLfloat tmp[4];
	static const GLfloat black[]={0.0,0.0,0.0,1.0};

	h=drawTextHeight(l->text)*scale+4.0;
	w=drawTextWidth(l->text)*scale+4.0;

	if (monochromeMode)
	{
		tmp[0]=1.0;
		tmp[1]=1.0;
		tmp[2]=1.0;
		tmp[3]=1.0;
	}
	else
	{
		tmp[0]=l->bgcolor[0]/255.0;
		tmp[1]=l->bgcolor[1]/255.0;
		tmp[2]=l->bgcolor[2]/255.0;
		tmp[3]=l->bgcolor[3]/255.0;
	}
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, tmp);
	glBegin(GL_POLYGON);
	glVertex3f(x  ,y,z+0.6);
	glVertex3f(x+w,y,z+0.6);
	glVertex3f(x+w,y-h,z+0.6);
	glVertex3f(x  ,y-h,z+0.6);
	glEnd();

	if (monochromeMode)
	{
		tmp[0]=0.0;
		tmp[1]=0.0;
		tmp[2]=0.0;
		tmp[3]=1.0;
	}
	else
	{
		tmp[0]=l->fgcolor[0]/255.0;
		tmp[1]=l->fgcolor[1]/255.0;
		tmp[2]=l->fgcolor[2]/255.0;
		tmp[3]=l->fgcolor[3]/255.0;
	}

	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, tmp);
	drawText(x+2.0,y-h+2.0,z+0.7,scale, l->text);

	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, black);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x,y,z+0.7);
	glVertex3f(x+w,y,z+0.7);
	glVertex3f(x+w,y-h,z+0.7);
	glVertex3f(x,y-h,z+0.7);
	glEnd();
}

void watcherGraphDraw(NodeEdge **g, NodeDisplayType dispType, NodeDisplayStatus *dispStat, destime curtime)
{
	NodeEdge *e,*d,*last;
	int i;
	GLfloat tmp[4];
	GLfloat zoffset=0.0;

	e=*g;
	last=NULL;

	while(e)
	{
		if ((e->nodeHead) && (e->nodeTail) && (e->priority <=dispStat->minPriority) && (dispStat->familyBitmap & (1<<e->family)))
		{
			switch(e->family)
			{
				case COMMUNICATIONS_LABEL_FAMILY_ROUTING:
					zoffset=-0.5;
				break;
				case COMMUNICATIONS_LABEL_FAMILY_CORRELATION_3HOP:
					zoffset=0.3;
				break;
				case COMMUNICATIONS_LABEL_FAMILY_ALERT:
					zoffset=0.5;
				break;
				default:
					zoffset=0;
			}
			if (dispStat->monochromeMode)
			{
				tmp[0]=0.0;
				tmp[1]=0.0;
				tmp[2]=0.0;
				tmp[3]=1.0;
			}
			else
				for(i=0;i<4;i++)
					tmp[i]=e->color[i]/255.0;

#warning special case to draw edges with head equal to tail
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tmp);
			drawHeavyArrow(e->nodeHead->x,e->nodeHead->y,e->nodeHead->z + zoffset,e->nodeTail->x,e->nodeTail->y,e->nodeTail->z + zoffset,e->width/10.0*dispStat->scaleLine[dispType]);

			if (e->labelHead.text)
				watcherLabelDraw(e->nodeHead->x*0.05+e->nodeTail->x*0.95,
						e->nodeHead->y*0.05+e->nodeTail->y*0.95,
						e->nodeHead->z*0.05+e->nodeTail->z*0.95,dispStat->scaleText[dispType], &(e->labelHead), dispStat->monochromeMode);
			if (e->labelMiddle.text)
				watcherLabelDraw(e->nodeHead->x*0.5+e->nodeTail->x*0.5,
						e->nodeHead->y*0.5+e->nodeTail->y*0.5,
						e->nodeHead->z*0.5+e->nodeTail->z*0.5,dispStat->scaleText[dispType], &(e->labelMiddle), dispStat->monochromeMode);
			if (e->labelHead.text)
				watcherLabelDraw(e->nodeHead->x*0.95+e->nodeTail->x*0.05,
						e->nodeHead->y*0.95+e->nodeTail->y*0.05,
						e->nodeHead->z*0.95+e->nodeTail->z*0.05,dispStat->scaleText[dispType], &(e->labelTail), dispStat->monochromeMode);
		}

		if ((e->expiration>0) && (e->expiration < curtime))   /* if it has an expire time, and it is expired...  */
		{
			d=e;
			e=e->next;
			if (last)
				last->next=e;
			else
				*g=e;

			free(d);
		}
		else
		{
			last=e;
			e=e->next;
		}
	}
}
