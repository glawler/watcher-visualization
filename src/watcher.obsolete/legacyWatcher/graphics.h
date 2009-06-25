#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef GRAPHICS
#include <GL/glut.h>
#endif

#include "des.h"

#include "watcherPropertyData.h"
using namespace watcher; 

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

/* $Id: graphics.h,v 1.30 2007/04/25 14:20:05 dkindred Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	double x;
	double y;
} points;

#ifdef GRAPHICS
manetNode *closestNode(manet *m, int x, int y, unsigned int r, unsigned int *dist_ret);
typedef struct XYWorldZToWorldXWorldY
{
    int x;
    int y;
    GLdouble worldZ;
    GLdouble worldX_ret;
    GLdouble worldY_ret;
} XYWorldZToWorldXWorldY;

/*
 * Get the world coordinate (x,y,z) for the projected coordinates (x, y)
 * at the world coordinate "z" using the given transformation matrices.
 */
int xyAtZForModelProjViewXY(
        XYWorldZToWorldXWorldY *xyz,
        size_t xyx_count,
        GLdouble modelmatrix[16], 
        GLdouble projmatrix[16], 
        GLint viewport[4]);

int xyAtZForScreenXY(int x, int y, GLdouble z, GLdouble *x_ret, GLdouble *y_ret);
void drawCircle( GLdouble x, GLdouble y, GLdouble z, GLdouble radius);

void drawText( GLdouble x, GLdouble y, GLdouble z, GLdouble scale, char *text, GLdouble lineWidth=2.0);
GLfloat drawTextHeight(char *text);
GLfloat drawTextWidth(char *text);

void drawArrow(GLdouble x1, GLdouble y1, GLdouble x2,GLdouble y2, GLdouble width);
void drawHeavyArrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2,GLdouble width);
int drawWrap(points p[], int N);

void nodeDraw(manetNode *us, NodeDisplayType dispType, NodeDisplayStatus const *dispStat, WatcherPropertyData *);
void nodeDrawFrowny(manetNode *us, NodeDisplayType dispType, NodeDisplayStatus const *dispStat, WatcherPropertyData *);
void drawNeighbors(manet *m);
void drawGraph(manet *m, int *graph,GLdouble scale, int drawlens);
void drawNodes(NodeDisplayType dispType, manet *m);
void nodeDrawLabel(manetNode *us, NodeDisplayStatus const *dispStat, GLfloat nodex, GLfloat nodey, GLfloat nodez);

void drawHierarchy(manet *m, NodeDisplayStatus const *dispStat);
void manetDraw(manet *m);
#define HIERARCHY_RADIUS(level)	(8 + (4*(level)))
#else
#define closestNode(m, x, y, r, dist_ret) (0)
#define xyAtZForScreenXY(x, y, z, x_ret, y_ret) (0)
#define drawCircle(a,b,c,d) do { } while(0)
#define drawText(a,b,c,d,e) do { } while(0)
#define drawArrow(a,b,c,d) do { } while(0)
#define drawHeavyArrow(a,b,c,d,e,f,g) do { } while(0)
#define drawWrap(a,b) do { } while(0)

#define drawNeighbors(a) do { } while(0)
#define drawGraph(a,b) do { } while(0)
#define drawNodes(a) do { } while(0)
#define nodeDrawLabel(n, disp, x, y, z) do { } while(0)

#define drawHierarchy(a) do { } while(0)
#define manetDraw(a) do { } while(0)
#define HIERARCHY_RADIUS(level)	(0)
#endif

#ifdef __cplusplus
}
#endif

#endif
