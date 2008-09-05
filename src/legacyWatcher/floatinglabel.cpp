#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>

#include "graphics.h"
#include "floatinglabel.h"
#include "watcherGraph.h"

static const char *rcsid __attribute ((unused)) = "$Id: floatinglabel.cpp,v 1.2 2007/03/09 22:59:58 tjohnson Exp $";

/*
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 */

#if 0
        unsigned char bgcolor[4],fgcolor[4];    /* background color, forground color   */
        char *text;

        int family;
        int priority;
        int tag;                        /* client assigned grouping value.  */
        destime expiration;    /* set to 0 to never expire   (Milliseconds)  */

        struct FloatingLabel *next;
#endif

void floatingLabelAdd(FloatingLabel **list, FloatingLabel *lab, destime curtime)
{
	FloatingLabel *n;

	n=(FloatingLabel*)malloc(sizeof(*n)+strlen(lab->text)+1);
	memcpy(n,lab,sizeof(*n));
	n->text=(char*)(n+1);
	strcpy(n->text,lab->text);

/* check the expire time, and convert it to an absolute timestamp if its non-zero  */
	if (n->expiration>0)
		n->expiration+=curtime;

	n->next=*list;
	*list=n;
}

void floatingLabelRemove(FloatingLabel **g, int bitmap, FloatingLabel *nw)
{
	FloatingLabel *p, *q,*d;

	p=*g;
	q=NULL;

	while(p)
	{
		if (
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

void floatingLabelRemoveFamily(FloatingLabel **g,int family)
{
	FloatingLabel key;

	key.family=family;
	floatingLabelRemove(g,COMMUNICATIONS_EDGE_REMOVE_FAMILY,&key);
}

void floatingLabelNuke(FloatingLabel **g)      /* remove all edges  */
{
	floatingLabelRemove(g,0,NULL);
}

void floatingLabelDraw(FloatingLabel **list, NodeDisplayType dispType, NodeDisplayStatus *dispStat, destime curtime)
{
	GLdouble h,w;
	GLfloat tmp[4];
	static const GLfloat black[]={0.0,0.0,0.0,1.0};
	FloatingLabel *l,*last=NULL,*d;

	l=*list;
	while(l!=NULL)
	{

		GLdouble x=l->x;
		GLdouble y=l->y;
		GLdouble z=l->z;

		h=drawTextHeight(l->text)*dispStat->scaleText[dispType]+4.0;
		w=drawTextWidth(l->text)*dispStat->scaleText[dispType]+4.0;

		if (dispStat->monochromeMode)
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

		if (dispStat->monochromeMode)
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
		drawText(x+2.0,y-h+2.0,z+0.7,dispStat->scaleText[dispType], l->text);

		glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, black);
		glBegin(GL_LINE_LOOP);
		glVertex3f(x,y,z+0.7);
		glVertex3f(x+w,y,z+0.7);
		glVertex3f(x+w,y-h,z+0.7);
		glVertex3f(x,y-h,z+0.7);
		glEnd();

		if ((l->expiration>0) && (l->expiration < curtime))   /* if it has an expire time, and it is expired...  */
		{
			d=l;
			l=l->next;
			if (last)
				last->next=l;
			else
				*list=l;

			free(d);
		}
		else
		{
			last=l;
			l=l->next;
		}
	}
}

