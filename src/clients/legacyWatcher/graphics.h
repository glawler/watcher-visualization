#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef GRAPHICS
#include <GL/glut.h>
#endif

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  Copyright (C) 2009  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: graphics.h,v 1.30 2007/04/25 14:20:05 dkindred Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

// WatcherGraphNodePtr closestNode(WatcherGraphPtr graph, int x, int y, unsigned int r, unsigned int *dist_ret);

void drawCircle( GLdouble x, GLdouble y, GLdouble z, GLdouble radius);
void drawText( GLdouble x, GLdouble y, GLdouble z, GLdouble scale, char *text, GLdouble lineWidth=2.0);
GLfloat drawTextHeight(char *text);
GLfloat drawTextWidth(char *text);

void drawArrow(GLdouble x1, GLdouble y1, GLdouble x2,GLdouble y2, GLdouble width);
void drawHeavyArrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2,GLdouble width);

void nodeDraw(WatcherGraphNodePtr us);
void nodeDrawFrowny(WatcherGraphNodePtr us);
// void drawNodes(WatcherGraphPtr graph);
// void nodeDrawLabel(LabelDisplayInfoPtr labelDisplayInfo, GLfloat x, GLfloat y, GLfloat z);
// void manetDraw(WatcherGraphPtr graph);

#ifdef __cplusplus
}
#endif

#endif
