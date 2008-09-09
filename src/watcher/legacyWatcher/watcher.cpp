/*
 * Copyright (c) 1991, 1992, 1993 Silicon Graphics, Inc.
 * Modifications Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF
 * ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <GL/glut.h>

#include "node.h"
#include "idsCommunications.h"
#include "apisupport.h"      /* This client is being evil, and using the debugging calls, which are only defined in apisupport.h */
#include "graphics.h"
#include "watchermovement.h"
#include "watcherGPS.h"
#include "marshal.h"
#include "watcherGraph.h"
#include "floatinglabel.h"

#include "mobility.h"
#include "watcher.h"
#include "legacyWatcher.h"

/* Copyright (C) 2004  Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This code is intended to provide a graphical display of the nodes running on
 * a live network, using the API.
 *
 * It uses the same "location" in the architecture as a clustering algorithm and the
 * simulation support code.  It then calls the detector API, as if it was a detector,
 * fills in simulation datastructures from that data, and calls simulation graphics
 * routines to display it.
 *
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: watcher.cpp,v 1.127 2007/08/14 20:17:03 dkindred Exp $";

static void update(manet *m);
static void detectorNeighborUpdate(void *data, CommunicationsNeighbor *cn);
static void detectorStatusUpdate(void *data, ApiStatus *as);
static void detectorPositionUpdate(void *data, IDSPositionType pos, IDSPositionStatus stat);
static void drawManet(void);
static void drawHierarchy(void);
static void watcherDrawNodes(NodeDisplayType dispType, manet *m);
long long int getMilliTime(void);
void gotMessageLabel(void *data, const struct MessageInfo *mi);
void gotMessageLabelRemove(void *data, const struct MessageInfo *mi);
void gotMessageColor(void *data, const struct MessageInfo *mi);
void gotMessageEdge(void *data, const struct MessageInfo *mi);
void gotMessageEdgeRemove(void *data, const struct MessageInfo *mi);
void gotMessageGPS(void *data, const struct MessageInfo *mi);
void gotMessageGraph(void *data, const struct MessageInfo *mi);
void gotMessageGraphEdge(void *data, const struct MessageInfo *mi);
void gotMessageFloatingLabel(void *data, const struct MessageInfo *mi);
void gotMessageFloatingLabelRemove(void *data, const struct MessageInfo *mi);

//
// Used when auto-centering drawing
//
typedef enum ScaleAndShiftUpdate
{
    ScaleAndShiftUpdateOnChange,
    ScaleAndShiftUpdateAlways,
};


static manet *globalManet;
static int *globalGraphHierarchy;
static float *globalGraphManet = NULL;
static float globalGraphScaleManet;
static NodeDisplayStatus globalDispStat;
static int globalWindowManet;
static int globalWindowHierarchy;
static destime globalLastMetricsTick;
static int globalWatcherMovementEnableFlag = 0;
static void *globalWatcherMovementState;
static NodeEdge *userGraph = NULL;
static FloatingLabel *floatingLabelList = NULL;
static destime curtime = 0, begintime = 0;
static manetNode *globalSelectedNode = 0;
static int globalSelectedNodeScreenX;
static int globalSelectedNodeScreenY;
static double globalSelectedNodeDeltaX;
static double globalSelectedNodeDeltaY;

static CommunicationsLogStatePtr globalGoodwin;

static struct
{
    int step;     /* distance to go...  */
    int runFlag;    /* Or, proceed in real time...  */
    destime runstartfile;
    destime runstartwall;
    destime stoptime;
    int speed;
} globalReplay = { 0, 0, 0, 0, 0, 0 };

static double GPSScale = 80000.0;
static int globalDoMetricsFlag;

#define MAXNODE 1024

static int globalAutoCenterNodesFlag = 0; 
static int globalGpsValidFlag[MAXNODE];

static int globalPopupMenuHandle;

static unsigned char globalNeighborColors[][4] = 
{
    {   0,   0, 255, 255 },
    {   0, 255,   0, 255 },
    {   0, 255, 255, 255 },
    { 255,   0,   0, 255 },
    { 255,   0, 255, 255 },
    { 255, 255,   0, 255 },
    {   0,   0,   0, 255 },
};

static struct
{
    float angleX;
    float angleY;
    float angleZ;
    float scaleX;
    float scaleY;
    float scaleZ;
    float shiftX;
    float shiftY;
    float shiftZ;
} globalManetAdj, 
    globalManetAdjInit = { 0.0, 0.0, 0.0, .035, .035, .1, 0.0, 0.0, 0.0 }, 
    globalHierarchyAdj, 
    globalHierarchyAdjInit = { 0.0, 0.0, .035, .035, .1, 1.0, 0.0, 0.0, 0.0 };

int globalShowPositionFlag = 0;
int globalExitAtEofFlag = 0;

#define TOGGLE 0x80000000
#define SPEED  0x10000000
#define MENU_SPEED1_4 (SPEED|1)
#define MENU_SPEED1_2 (SPEED|2)
#define MENU_SPEED1 (SPEED|4)
#define MENU_SPEED2 (SPEED|8)
#define MENU_SPEED4 (SPEED|16)
#define MENU_SPEED8 (SPEED|32)
#define MENU_SPEED16 (SPEED|64)
#define MENU_SPEED32 (SPEED|128)

#define MENU_QUIT 1
#define MENU_VIEWPOINTRESET 2
#define MENU_ZOOMIN 4
#define MENU_ZOOMOUT 5
#define MENU_ALLOFF 6
#define MENU_ALLON 7

#define MENU_LABELCRITICAL 8
#define MENU_LABELWARN 9
#define MENU_LABELINFO 10
#define MENU_CLEARLABELS 11
#define MENU_CLEAREDGES 12

typedef struct
{
    char *stringSet;
    char *stringClear;
    int bit;
    int entry;
} menuEntry;

menuEntry *menuList[32];

/* Add a layer toggling menu entry.
 * 
 * layerLabel is what the layer is called.
 * num is the array index of this menu Entry
 * bit is the bit to set/clear in the dispStat bitmap field
 */
static menuEntry *menuLayerTogglingEntryAdd(char *layerLabel, int num, int bit)
{
    menuEntry *me;

    me = (menuEntry*)malloc(sizeof(*me) + strlen(layerLabel) *2 + 6);
    me->stringSet = (char *)me + sizeof(*me);
    me->stringClear = me->stringSet + strlen(layerLabel) + 3;

    sprintf(me->stringSet,   "X %s", layerLabel);
    sprintf(me->stringClear, "  %s", layerLabel);
    me->bit = 1 << bit;
    glutAddMenuEntry(
            (globalDispStat.familyBitmap & me->bit) ?  me->stringSet : me->stringClear, 
            TOGGLE | num);
    me->entry = glutGet(GLUT_MENU_NUM_ITEMS);

    return me;
}

static void menuToggle(menuEntry **meArray, int val)
{
    menuEntry *me = meArray[val & 0xFFFF];

    globalDispStat.familyBitmap ^= me->bit;
    glutSetMenu(globalPopupMenuHandle);
    glutChangeToMenuEntry(me->entry, (globalDispStat.familyBitmap&me->bit) ? me->stringSet : me->stringClear, TOGGLE | val);
}

static void menuUpdate(menuEntry **meArray)
{
    menuEntry *me;
    int i;

    for(i = 0; meArray[i]; i++)
    {
        me = meArray[i];
        glutSetMenu(globalPopupMenuHandle);
        glutChangeToMenuEntry(me->entry, (globalDispStat.familyBitmap&me->bit) ? me->stringSet : me->stringClear, TOGGLE | i);
    }
}

static void labelsClear(manet *m);

//
// Quadrangle points form a convex hull
//
typedef struct QuadranglePoint
{
    double x;
    double y;
} QuadranglePoint;

typedef struct Quadrangle
{
    QuadranglePoint p[4];
} Quadrangle;

static void maxRectangle(
        Quadrangle const *q,
        double maxRectAspectRatio,
        double *xMinRet,
        double *yMinRet,
        double *xMaxRet,
        double *yMaxRet)
{
    size_t i;
    QuadranglePoint const *p = q->p;
    double a01 = atan((p[1].y - p[0].y)/(p[1].x - p[0].x));
    double a12 = atan((p[2].y - p[1].y)/(p[2].x - p[1].x));
    double a23 = atan((p[3].y - p[2].y)/(p[3].x - p[2].x));
    double a30 = atan((p[0].y - p[3].y)/(p[0].x - p[3].x));
    // short circuit rectangles that are aligned with the x and y axis.
    // (if all lines are within 10 degrees of horizontal or vertical,
    // the rectanglish shape is assumed to be aligned enough).
    if((a01 < (M_PI/18) && a01 > (-M_PI/18)) &&
       (a23 < (M_PI/18) && a23 > (-M_PI/18)) &&
       (a12 > ((M_PI/2) - (M_PI/18)) || a12 < ((-M_PI/2) + (M_PI/18))) &&
       (a30 > ((M_PI/2) - (M_PI/18)) || a30 < ((-M_PI/2) + (M_PI/18))))

    {
        // got an aligned rectanglish shape
        fprintf(stderr, "aligned (%g,%g,%g,%g) (%g,%g)-(%g,%g)-(%g,%g)-(%g,%g)\n",
                round(a01*180/M_PI), round(a12*180/M_PI), round(a23*180/M_PI), round(a30*180/M_PI),
                p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);
        if(p[0].x < p[1].x)
        {
            *xMinRet = p[0].x;
            *xMaxRet = p[1].x;
        }
        else
        {
            *xMinRet = p[1].x;
            *xMaxRet = p[0].x;
        }
        if(p[0].y < p[2].y)
        {
            *yMinRet = p[0].y;
            *yMaxRet = p[2].y;
        }
        else
        {
            *yMinRet = p[2].y;
            *yMaxRet = p[0].y;
        }
    }
    else if((a12 < (M_PI/18) && a12 > (-M_PI/18)) &&
       (a30 < (M_PI/18) && a30 > (-M_PI/18)) &&
       (a01 > ((M_PI/2) - (M_PI/18)) || a01 < ((-M_PI/2) + (M_PI/18))) &&
       (a23 > ((M_PI/2) - (M_PI/18)) || a23 < ((-M_PI/2) + (M_PI/18))))
    {
        // got an aligned rectanglish shape rotated 90 degrees from the
        // one above
        fprintf(stderr, "90deg aligned (%g,%g,%g,%g) (%g,%g)-(%g,%g)-(%g,%g)-(%g,%g)\n",
                round(a01*180/M_PI), round(a12*180/M_PI), round(a23*180/M_PI), round(a30*180/M_PI),
                p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);
        if(p[0].x < p[2].x)
        {
            *xMinRet = p[0].x;
            *xMaxRet = p[2].x;
        }
        else
        {
            *xMinRet = p[2].x;
            *xMaxRet = p[0].x;
        }
        if(p[0].y < p[1].y)
        {
            *yMinRet = p[0].y;
            *yMaxRet = p[1].y;
        }
        else
        {
            *yMinRet = p[1].y;
            *yMaxRet = p[0].y;
        }
    }
    else
    {
        // dang, got something lopsided.
#if 0
        //
        // I think there are thirty-two cases to check of which four
        // would produce valid solutions - these solutions would then
        // have to be simply compared to find the true maximum.
        //
        // Just figuring out the cases has already taken me too long so
        // I'm punting.
        //
        static size_t nextClockwise[] = { 1, 2, 3, 0 };
        static size_t nextCounterClockwise[] = { 3, 0, 1, 2 };
        size_t *next;

        // find leftmost point
        size_t leftmost_i = 0;
        for(i = 1; i < 4; ++i)
        {
            if(p[i].x < p[leftmost_i].x)
            {
                leftmost_i = i;
            }
        }
        // find direction
        next = (p[nextClockwise[leftmost_i]].y > p[nextCounterClockwise[leftmost_i]].y) ? nextClockwise : nextCounterClockwise;
#else
        // dumb but serviceable way - just make the returned rectangle
        // half size of the enclosing rectangle.
        double xMin = p[0].x;
        double yMin = p[0].y;
        double xMax = p[0].x;
        double yMax = p[0].y;
        fprintf(stderr, "oddball (%g,%g,%g,%g) (%g,%g)-(%g,%g)-(%g,%g)-(%g,%g)\n",
                round(a01*180/M_PI), round(a12*180/M_PI), round(a23*180/M_PI), round(a30*180/M_PI),
                p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y);
        for(i = 1; i < 4; ++i)
        {
            if(p[i].x < xMin)
            {
                xMin = p[i].x;
            }
            if(p[i].y < yMin)
            {
                yMin = p[i].y;
            }
            if(p[i].x > xMax)
            {
                xMax = p[i].x;
            }
            if(p[i].y > yMax)
            {
                yMax = p[i].y;
            }
        }
        *xMinRet = xMin + ((xMax - xMin)/4);
        *yMinRet = yMin + ((yMax - yMin)/4);
        *xMaxRet = xMax - ((xMax - xMin)/4);
        *yMaxRet = yMax - ((yMax - yMin)/4);
#endif
    }
    return;
} // maxRectangle


//
// Get a box at z that can be seen though the given viewport
//
// (assumes viewport[0] = viewport[1] = 0)
//
// returns non-zero on success.
//
static int visibleDrawBoxAtZ(
        GLint *viewport,
        GLdouble z,
        GLdouble modelmatrix[16],
        GLdouble projmatrix[16],
        double maxRectAspectRatio,
        double *xMinRet,
        double *yMinRet,
        double *xMaxRet,
        double *yMaxRet)
{
    int ret;
    // Set up a border in the screen between the edges and the
    // drawing area for prettiness sake. The border goes away
    // for width or height less then 80. It then grows as 10% of
    // the width greater than 80 until it reaches a maximum
    // width of 50. These values were determined via rectal
    // extraction but the results seems pleasing to the eye.
    int borderX = viewport[2] < 80 ? 0 : viewport[2] < 580 ? (viewport[2] - 80) / 20 : 50;
    int borderY = viewport[3] < 80 ? 0 : viewport[3] < 580 ? (viewport[3] - 80) / 20 : 50;
    XYWorldZToWorldXWorldY xyz[4] =
    {
        // corners ll, lr, ul, ur
        { borderX, borderY, z, 0, 0 },
        { viewport[2] - borderX, borderY, z, 0, 0 },
        { viewport[2] - borderX, viewport[3] - borderY, z, 0, 0 },
        { borderX, viewport[3] - borderY, z, 0, 0 },
    };
    // get the drawing area corners with no shifting or scaling
    if(xyAtZForModelProjViewXY(xyz, sizeof(xyz) / sizeof(xyz[0]),
                modelmatrix, projmatrix, viewport) ==  0)
    {
        Quadrangle q = 
        { {
            { xyz[0].worldX_ret, xyz[0].worldY_ret },
            { xyz[1].worldX_ret, xyz[1].worldY_ret },
            { xyz[2].worldX_ret, xyz[2].worldY_ret },
            { xyz[3].worldX_ret, xyz[3].worldY_ret }
        } };
        maxRectangle(&q, maxRectAspectRatio, xMinRet, yMinRet, xMaxRet, yMaxRet);
        ret = !0;
    }
    else
    {
        ret = 0;
    }
    return ret;
} // visibleDrawBoxAtZ



//
// Make the (xMin,yMin)-(xMax,yMax) rectangle on the xy-plane at
// z visible on the Manet screen
//
// onChangeOrAlways - ScaleAndShiftUpdateOnChange to only update scale
//                    and shift values if things have changed enough,
//                    ScaleAndShiftUpdateAlways to update even if things
//                    haven't changed at all Usually will not update
//                    scaling or shifting if nothing has changed
//
static void scaleAndShiftToSeeOnManet(
        double xMin, 
        double yMin, 
        double xMax, 
        double yMax, 
        double z,
        ScaleAndShiftUpdate onChangeOrAlways)
{
    static int upfcnt = 0;
    static int nccnt = 0;
    static double prevXMin = DBL_MAX;
    static double prevXMax = -DBL_MAX;
    static double prevYMin = DBL_MAX;
    static double prevYMax = -DBL_MAX;
    static double prevZ = DBL_MAX;
    static GLint prevWidth;
    static GLint prevHeight;
    GLint viewport[4];
    // adjust min and max values so the change less often. Do this
    // by reducing the precision to only three digits of precision 
    // by rounding the lower values down and higher values up.
    double dx = xMax - xMin;
    double dy = yMax - yMin;
    double mul = exp10(floor(log10(dx < dy ? dx : dy)) - 2);
    xMin = mul * floor(xMin / mul);
    yMin = mul * floor(yMin / mul);
    xMax = mul * ceil(xMax / mul);
    yMax = mul * ceil(yMax / mul);
    // get the viewport because the scale and centering might change
    // based on a screen resize
    glGetIntegerv(GL_VIEWPORT, viewport);
    if(onChangeOrAlways == ScaleAndShiftUpdateAlways ||
            prevXMin != xMin ||
            prevYMin != yMin ||
            prevXMax != xMax ||
            prevYMax != yMax ||
            prevZ != z ||
            prevWidth != viewport[2] ||
            prevHeight != viewport[3])
    {
        double wXMin;
        double wYMin;
        double wXMax;
        double wYMax;
        double  nodesWidth = xMax == xMin ? 1 : xMax - xMin;
        double nodesHeight = yMax == yMin ? 1 : yMax - yMin;
        GLdouble modelmatrix[16];
        GLdouble projmatrix[16];
        // Matrices and viewport to convert from world to screen
        // coordinates. 
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -20.0);
        glScalef(1.0, 1.0, globalManetAdj.scaleZ); // getting scale x and y so start with unity
        glRotatef(globalManetAdj.angleX, 1.0, 0.0, 0.0);
        glRotatef(globalManetAdj.angleY, 0.0, 1.0, 0.0);
        glRotatef(globalManetAdj.angleZ, 0.0, 0.0, 1.0);
        glTranslatef(0.0, 0.0, globalManetAdj.shiftZ + 3); // getting shift x and y so start with zero
        glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glPopMatrix();
        if(visibleDrawBoxAtZ(
                    viewport,
                    z,
                    modelmatrix,
                    projmatrix,
                    nodesWidth/nodesHeight,
                    &wXMin,
                    &wYMin,
                    &wXMax,
                    &wYMax))
        {
            static time_t tick = 0;
            time_t now = time(0);
            // get shift and scale
            globalManetAdj.shiftX = ((wXMin + wXMax) / 2) - ((xMin + xMax) / 2);
            globalManetAdj.shiftY = ((wYMin + wYMax) / 2) - ((yMin + yMax) / 2);
            globalManetAdj.scaleX = (wXMax - wXMin) / nodesWidth;
            globalManetAdj.scaleY = (wYMax - wYMin) / nodesHeight;
            if(globalManetAdj.scaleX > globalManetAdj.scaleY)
            {
                globalManetAdj.scaleX = globalManetAdj.scaleY;
            }
            else
            {
                globalManetAdj.scaleY = globalManetAdj.scaleX;
            }
            //if(now > tick)
            {
                fprintf(stderr, "********************************************************************\n");
                fprintf(stderr, "****************viewport (%g,%g)-(%g,%g)********************\n",
                        wXMin, wYMin, wXMax, wYMax);
                fprintf(stderr, "****************extents  (%g,%g)-(%g,%g)********************\n",
                        xMin, yMin, xMax, yMax);
                fprintf(stderr, "****************scaleXManet=%g****************\n", globalManetAdj.scaleX);
                fprintf(stderr, "****************shift(%g,%g)****************\n", globalManetAdj.shiftX, globalManetAdj.shiftY);
                if(nccnt)
                {
                    fprintf(stderr, "****************no scale/shift change %d times****************\n", nccnt);
                    nccnt = 0;
                }
                if(upfcnt)
                {
                    fprintf(stderr, "****************failed unproject %d times****************\n", upfcnt);
                    upfcnt = 0;
                }
                tick = now + 1;
            }
        }
        prevXMin = xMin;
        prevYMin = yMin;
        prevXMax = xMax;
        prevYMax = yMax;
        prevZ = z;
        prevWidth = viewport[2];
        prevHeight = viewport[3];
    }
    else
    {
        ++nccnt;
    }
    {
        static time_t tick = 0;
        time_t now = time(0);
        if(now > tick)
        {
            if(nccnt)
            {
                fprintf(stderr, "****************no scale/shift change %d times****************\n", nccnt);
                nccnt = 0;
            }
            if(upfcnt)
            {
                fprintf(stderr, "****************failed unproject %d times****************\n", upfcnt);
                upfcnt = 0;
            }
            tick = now + 5;
        }
    }
} // scaleAndShiftToSee

//
// Scale and shift the Manet to the center of the Manet screen
//
// onChangeOrAlways - ScaleAndShiftUpdateOnChange to only update scale
//                    and shift values if things have changed enough,
//                    ScaleAndShiftUpdateAlways to update even if things
//                    haven't changed at all Usually will not update
//                    scaling or shifting if nothing has changed
//
static void scaleAndShiftToCenter(manet *m, ScaleAndShiftUpdate onChangeOrAlways)
{
    if(m->numnodes)
    {
        double xMin = DBL_MAX;
        double xMax = -DBL_MAX;
        double yMin = DBL_MAX;
        double yMax = -DBL_MAX;
        double zMin = DBL_MAX;
        int includeAntenna = (globalDispStat.familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS)) ? !0 : 0;
        int includeHierarchy = (globalDispStat.familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_HIERARCHY)) ? !0 : 0;
        int i;
        // find drawing extents
        for(i = 0; i < m->numnodes; ++i)
        {
            double r = 0;
            if(includeAntenna)
            {
                r = m->nlist[i].aradius;
            }
            if(includeHierarchy)
            {
                double hr = HIERARCHY_RADIUS(m->nlist[i].level);
                if(r < hr)
                {
                    r = hr;
                }
            }
            {
                double nodeXMin = m->nlist[i].x - r;
                double nodeXMax = m->nlist[i].x + r;
                double nodeYMin = m->nlist[i].y - r;
                double nodeYMax = m->nlist[i].y + r;
                if(nodeXMin < xMin)
                {
                    xMin = nodeXMin;
                }
                if(nodeXMax > xMax)
                {
                    xMax = nodeXMax;
                }
                if(nodeYMin < yMin)
                {
                    yMin = nodeYMin;
                }
                if(nodeYMax > yMax)
                {
                    yMax = nodeYMax;
                }
            }
            if(m->nlist[i].z < zMin)
            {
                zMin = m->nlist[i].z;
            }
        }
        scaleAndShiftToSeeOnManet(xMin, yMin, xMax, yMax, zMin, onChangeOrAlways);
    }
} // scaleAndShiftToCenter


static void ReshapeManet(int awidth, int aheight)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    fprintf(stderr, "Reshape cur (%d, %d)\n", viewport[2], viewport[3]);
    fprintf(stderr, "Reshape given (%d, %d)\n", awidth, aheight);
    glViewport(0, 0, awidth, aheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, GLfloat(awidth) / GLfloat(aheight), 1.0, 50.0);
    if(globalAutoCenterNodesFlag && globalManet)
    {
        scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateOnChange);
    }
}

static void ReshapeHierarchy(int awidth, int aheight)
{
    glViewport(0, 0, awidth, aheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, GLfloat(awidth) / GLfloat(aheight), 1.0, 50.0);
}

static void getShiftAmount(GLdouble &x_ret, GLdouble &y_ret)
{
    GLdouble z;
    int i;
    GLdouble modelmatrix[16];
    GLdouble projmatrix[16];
    GLint viewport[4];
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);
    glScalef(globalManetAdj.scaleX, globalManetAdj.scaleY, globalManetAdj.scaleZ);
    glRotatef(globalManetAdj.angleX, 1.0, 0.0, 0.0);
    glRotatef(globalManetAdj.angleY, 0.0, 1.0, 0.0);
    glRotatef(globalManetAdj.angleZ, 0.0, 0.0, 1.0);
    glTranslatef(globalManetAdj.shiftX, globalManetAdj.shiftY, globalManetAdj.shiftZ + 3);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glPopMatrix();

    if(globalManet)
    {
        for(i = 0; ; ++i)
        {
            if(i == globalManet->numnodes)
            {
                z = -20;
                break;
            }
            if(globalGpsValidFlag[i])
            {
                z = globalManet->nlist[i].z;
                break;
            }
        }
    }
    else
    {
        z = -20;
    }
    {
        int xmid = viewport[2] / 2;
        int ymid = viewport[3] / 2;
        int dx = viewport[2] / 20; // shift by 1/20th the screen
        int dy = viewport[3] / 20; // shift by 1/20th the screen
        XYWorldZToWorldXWorldY xyz[2] =
        {
            { xmid, ymid, z, 0, 0 },
            { xmid + dx, ymid - dy, z, 0, 0 },
        };
        if(xyAtZForModelProjViewXY(
                    xyz, sizeof(xyz) / sizeof(xyz[0]),
                    modelmatrix, projmatrix, viewport) ==  0)
        {
            x_ret = xyz[0].worldX_ret - xyz[1].worldX_ret;
            y_ret = xyz[1].worldY_ret - xyz[2].worldY_ret;
        }
        else
        {
            x_ret = 20.;
            y_ret = 20.;
        }
    }
    return;
}

static void shiftCenterLeft()
{
    if(glutGetWindow() == globalWindowManet)
    {
        GLdouble xshift, dummy;
        getShiftAmount(xshift, dummy);
        globalManetAdj.shiftX += xshift;
        globalAutoCenterNodesFlag = 0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.shiftX += 20.0;
    }
} // shiftCenterLeft

static void shiftCenterRight()
{
    if(glutGetWindow() == globalWindowManet)
    {
        GLdouble xshift, dummy;
        getShiftAmount(xshift, dummy);
        globalManetAdj.shiftX -= xshift;
        globalAutoCenterNodesFlag = 0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.shiftX -= 20.0;
    }
} // shiftCenterRight

static void shiftCenterUp()
{
    if(glutGetWindow() == globalWindowManet)
    {
        GLdouble yshift, dummy;
        getShiftAmount(dummy, yshift);
        globalManetAdj.shiftY -= yshift;
        globalAutoCenterNodesFlag = 0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.shiftY -= 20.0;
    }
} // shiftCenterUp

static void shiftCenterDown()
{
    if(glutGetWindow() == globalWindowManet)
    {
        GLdouble yshift, dummy;
        getShiftAmount(dummy, yshift);
        globalManetAdj.shiftY += yshift;
        globalAutoCenterNodesFlag = 0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.shiftY += 20.0;
    }
} // shiftCenterDown

static void shiftCenterIn()
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.shiftZ -= 20.0;
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.shiftZ -= 20.0;
    }
} // shiftCenterUp

static void shiftCenterOut()
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.shiftZ += 20.0;
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.shiftZ += 20.0;
    }
} // shiftCenterDown

static void Key2(int key, int x, int y)
{

    switch (key) {
        case GLUT_KEY_LEFT:
            shiftCenterRight();
            break;
        case GLUT_KEY_RIGHT:
            shiftCenterLeft();
            break;
        case GLUT_KEY_UP:
            shiftCenterDown();
            break;
        case GLUT_KEY_DOWN:
            shiftCenterUp();
            break;
        default:
            return;
    }
    glutPostRedisplay();
}

static void viewpointReset(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj = globalManetAdjInit;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj = globalHierarchyAdjInit;
    }
}

static void zoomOut(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.scaleX /= 1.05;
        if (globalManetAdj.scaleX < 0.001) {
            globalManetAdj.scaleX = 0.001;
        }
        globalManetAdj.scaleY = globalManetAdj.scaleX;
        globalAutoCenterNodesFlag = 0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.scaleX -= 0.01;
        if (globalHierarchyAdj.scaleX < 0.01) {
            globalHierarchyAdj.scaleX = 0.01;
        }
        globalHierarchyAdj.scaleY = globalHierarchyAdj.scaleX;
    }
}

static void zoomIn(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.scaleX *= 1.05;
        globalManetAdj.scaleY = globalManetAdj.scaleX;
        globalAutoCenterNodesFlag = 0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.scaleX += 0.01;
        globalHierarchyAdj.scaleY = globalHierarchyAdj.scaleX;
    }
}

static void compressDistance()
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.scaleZ -= 0.1;
        if (globalManetAdj.scaleZ < 0.1)
        {
            globalManetAdj.scaleZ = 0.1;
        }
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.scaleZ -= 0.1;
        if (globalHierarchyAdj.scaleZ < 0.1)
        {
            globalHierarchyAdj.scaleZ = 0.1;
        }
    }
} // compressDistance

static void expandDistance()
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.scaleZ += 0.1;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.scaleZ += 0.1;
    }
} // compressDistance

#define TEXT_SCALE 0.08
#define TEXT_SCALE_ZOOM_FACTOR 1.05
#define ARROW_SCALE_ZOOM_FACTOR 1.05

static void textZoomReset(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalDispStat.scaleText[NODE_DISPLAY_MANET] = 0.08;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] = 0.08;
    }
}

static void textZoomIn(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalDispStat.scaleText[NODE_DISPLAY_MANET] *= TEXT_SCALE_ZOOM_FACTOR;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] *= TEXT_SCALE_ZOOM_FACTOR;
    }
}

static void textZoomOut(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalDispStat.scaleText[NODE_DISPLAY_MANET] /= TEXT_SCALE_ZOOM_FACTOR;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] /= TEXT_SCALE_ZOOM_FACTOR;
    }
}

static void arrowZoomReset(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalDispStat.scaleLine[NODE_DISPLAY_MANET] = 1.0;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] = 1.0;
    }
}

static void arrowZoomIn(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalDispStat.scaleLine[NODE_DISPLAY_MANET] *= ARROW_SCALE_ZOOM_FACTOR;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] *= ARROW_SCALE_ZOOM_FACTOR;
    }
}

static void arrowZoomOut(void)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalDispStat.scaleLine[NODE_DISPLAY_MANET] /= ARROW_SCALE_ZOOM_FACTOR;
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] /= ARROW_SCALE_ZOOM_FACTOR;
    }
}

static void rotateX(float deg)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.angleX += deg;
        while(globalManetAdj.angleX >= 360.0)
        {
            globalManetAdj.angleX -= 360.0;
        }
        while(globalManetAdj.angleX < 0)
        {
            globalManetAdj.angleX += 360.0;
        }
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.angleX += deg;
        while(globalHierarchyAdj.angleX >= 360.0)
        {
            globalHierarchyAdj.angleX -= 360.0;
        }
        while(globalHierarchyAdj.angleX < 0)
        {
            globalHierarchyAdj.angleX += 360.0;
        }
    }
} // rotateX

static void rotateY(float deg)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.angleY += deg;
        while(globalManetAdj.angleY >= 360.0)
        {
            globalManetAdj.angleY -= 360.0;
        }
        while(globalManetAdj.angleY < 0)
        {
            globalManetAdj.angleY += 360.0;
        }
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.angleY += deg;
        while(globalHierarchyAdj.angleY >= 360.0)
        {
            globalHierarchyAdj.angleY -= 360.0;
        }
        while(globalHierarchyAdj.angleY < 0)
        {
            globalHierarchyAdj.angleY += 360.0;
        }
    }
} // rotateY

static void rotateZ(float deg)
{
    if(glutGetWindow() == globalWindowManet)
    {
        globalManetAdj.angleZ += deg;
        while(globalManetAdj.angleZ >= 360.0)
        {
            globalManetAdj.angleZ -= 360.0;
        }
        while(globalManetAdj.angleZ < 0)
        {
            globalManetAdj.angleZ += 360.0;
        }
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else if(glutGetWindow() == globalWindowHierarchy)
    {
        globalHierarchyAdj.angleZ += deg;
        while(globalHierarchyAdj.angleZ >= 360.0)
        {
            globalHierarchyAdj.angleZ -= 360.0;
        }
        while(globalHierarchyAdj.angleZ < 0)
        {
            globalHierarchyAdj.angleZ += 360.0;
        }
    }
} // rotateZ

static void Key(unsigned char key, int x, int y)
{
    int mod = glutGetModifiers();
    char keystr[2] = { 0, 0 };
    fprintf(stderr, "%s: %s%s%s%s (0x%x)\n", __func__,
            mod & GLUT_ACTIVE_SHIFT ? "<shift> " : "", 
            mod & GLUT_ACTIVE_CTRL ? "<ctrl> " : "",
            mod & GLUT_ACTIVE_ALT ? "<alt> " : "",
            isprint(key) ? (keystr[0] = key, keystr) : keystr, 
            key);

    switch (key) {
        case 27:
            exit(1);

        case 'n':
            shiftCenterIn();
            break;
        case 'm':
            shiftCenterOut();
            break;

        case 'q':
            zoomOut();
            break;
        case 'w':
            zoomIn();
            break;

        case 'a':
            textZoomOut();
            break;
        case 's':
            textZoomIn();
            break;

        case 'a' - 'a' + 1:
            arrowZoomOut();
            break;
        case 's' - 'a' + 1:
            arrowZoomIn();
            break;

        case 'z':
            compressDistance();
            break;
        case 'x':
            expandDistance();
            break;

        case 'e':
            rotateX(-5.0);
            break;
        case 'r':
            rotateX(5.0);
            break;
        case 'd':
            rotateY(-5.0);
            break;
        case 'f':
            rotateY(5.0);
            break;
        case 'c':
            rotateZ(-5.0);
            break;
        case 'v':
            rotateZ(5.0);
            break;

        case 'b':
        case 'B':
            menuToggle(menuList, 0);
            break;

        case 'r' - 'a' + 1:
            textZoomReset();
            arrowZoomReset();
            viewpointReset();
            break;

        case '=':
        case '+':
            globalAutoCenterNodesFlag = 1; 
            if(globalManet)
            {
                scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
            }
            break;

        case ' ':
            globalReplay.runFlag = !globalReplay.runFlag;
            if (globalReplay.runFlag)
            {
                globalReplay.runstartfile = curtime;
                globalReplay.runstartwall = getMilliTime();
            }
            break;
        case 't':
            globalReplay.step = 1000;
            break;

        default:
            fprintf(stderr, "%s:       <esc>: exit\n", __func__);
            fprintf(stderr, "%s:         <n>: closer\n", __func__);
            fprintf(stderr, "%s:         <m>: further\n", __func__);
            fprintf(stderr, "%s:         <q>: zoom out\n", __func__);
            fprintf(stderr, "%s:         <w>: zoom in\n", __func__);
            fprintf(stderr, "%s:         <a>: text zoom out\n", __func__);
            fprintf(stderr, "%s:         <s>: text zoom in\n", __func__);
            fprintf(stderr, "%s:  <ctrl> <a>: arrow zoom out\n", __func__);
            fprintf(stderr, "%s:  <ctrl> <s>: arrow zoom in\n", __func__);
            fprintf(stderr, "%s:         <x>: compress Z scale\n", __func__);
            fprintf(stderr, "%s:         <z>: expand Z scale\n", __func__);
            fprintf(stderr, "%s:         <e>: tilt up\n", __func__);
            fprintf(stderr, "%s:         <r>: tilt down\n", __func__);
            fprintf(stderr, "%s:         <d>: rotate left\n", __func__);
            fprintf(stderr, "%s:         <f>: rotate right\n", __func__);
            fprintf(stderr, "%s:         <c>: spin clockwise\n", __func__);
            fprintf(stderr, "%s:         <v>: spin counterclockwise\n", __func__);
            fprintf(stderr, "%s:         <b>: bandwidth toggle\n", __func__);
            fprintf(stderr, "%s:  <ctrl> <r>: reset viewpoint\n", __func__);
            fprintf(stderr, "%s: < > (space): start/stop\n", __func__);
            fprintf(stderr, "%s:         <t>: step on second\n", __func__);
            return;
    }

    glutPostRedisplay();
}

static void Mouse(int button, int updown, int mousex, int mousey)
{
    if(globalManet && 
            globalManet->numnodes && 
            button == GLUT_LEFT_BUTTON && 
            updown == GLUT_DOWN)
    {
        unsigned int dist;
        manetNode *n = closestNode(
                globalManet, mousex, mousey, (unsigned int)(4.0 * globalManetAdj.scaleX / 0.02), &dist);
        if(n)
        {
            if(dist < 5)
            {
                globalSelectedNode = n;
                globalSelectedNodeScreenX = mousex;
                globalSelectedNodeScreenY = mousey;
                globalSelectedNodeDeltaX = 0;
                globalSelectedNodeDeltaY = 0;
            }
        }
    }
    else if(updown == GLUT_UP)
    {
        globalSelectedNode = 0;
    }
    return;
}

static void Motion(int mousex, int mousey)
{
    if(globalSelectedNode)
    {
        GLdouble modelmatrix[16];
        GLdouble projmatrix[16];
        GLint viewport[4];
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -20.0);
        glScalef(globalManetAdj.scaleX, globalManetAdj.scaleY, globalManetAdj.scaleZ);
        glRotatef(globalManetAdj.angleX, 1.0, 0.0, 0.0);
        glRotatef(globalManetAdj.angleY, 0.0, 1.0, 0.0);
        glRotatef(globalManetAdj.angleZ, 0.0, 0.0, 1.0);
        glTranslatef(globalManetAdj.shiftX, globalManetAdj.shiftY, globalManetAdj.shiftZ + 3);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glPopMatrix();
        {
            XYWorldZToWorldXWorldY xyz =
            {
                mousex, viewport[3] - mousey, globalSelectedNode->z, 0, 0
            };
            if(xyAtZForModelProjViewXY(
                        &xyz, 1, modelmatrix, projmatrix, viewport) == 0)
            {
                globalSelectedNodeDeltaX = 
                    globalSelectedNode->x - xyz.worldX_ret;
                globalSelectedNodeDeltaY = 
                    globalSelectedNode->y - xyz.worldY_ret;
            }
        }
    }
    return;
}

static GLenum Args(int argc, char **argv)
{
    return GL_TRUE;
}


static void crossProduct(double *c, double a[3], double b[3])
{  
    double len;
    c[0] = (a[1] * b[2]) - (b[1] * a[2]);
    c[1] = (a[2] * b[0]) - (b[2] * a[0]);
    c[2] = (a[0] * b[1]) - (b[0] * a[1]);

    len = sqrt((c[0] * c[0]) + (c[1] * c[1]) + (c[2] * c[2]));
    c[0] /= len;
    c[1] /= len;
    c[2] /= len;
}



static void drawManet(void)
{
    static const GLfloat light_pos[] = { 0.0, 0.0, 0.0, 1.0 };
    static const GLfloat blue[] = { 0.2f, 0.2f, 1.0f, 1.0f };
    static const GLfloat blue2[] = { 0.8f, 0.8f, 1.0f, 1.0f };
    static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
#if 0
    static const GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
    static const GLfloat polished[] = { 50.0 };
    static const GLfloat arena[] = { 0.0, 1.0, 1.0, 1.0 };
    static const GLfloat histogram[] = { 0.0, 0.0, 1.0, 1.0 };
#endif
    static const GLfloat hierarchy[] = { 0.0, 0.0, 1.0, 0.5 };
    static const GLfloat physical[] = { 1.0, 0.0, 0.0, 1.0 };

    manet *m = globalManet;
    int *phy;
    char buff[128];

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    glTranslatef(0.0, 0.0, -20.0);

    glPushMatrix();
    glTranslatef(0.0, 0.0, -50.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glPopMatrix();

    glPushMatrix();
    struct tm tm;
    time_t curtime_secs = curtime / 1000;
    if (localtime_r(&curtime_secs, &tm))
    {
        snprintf(buff, sizeof(buff),
                "Time: %04d-%02d-%02d %02d:%02d:%02d.%03d",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                (int)(curtime % 1000));
    }
    else
        snprintf(buff, sizeof(buff), "Time: %lld", curtime);

    glScalef(0.02, 0.02, 0.02);
    if (globalDispStat.monochromeMode)
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    else
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    drawText(-400, -360, 0, globalDispStat.scaleText[NODE_DISPLAY_MANET], buff);

    if (globalShowPositionFlag)
    {
        snprintf(buff, sizeof(buff),
                "Location: %3.1f, %3.1f, %3.1f scale %f layers: 0x%x  time: %ld @%ld",
                globalManetAdj.shiftX, globalManetAdj.shiftY, globalManetAdj.shiftZ, 
                globalManetAdj.scaleX,
                globalDispStat.familyBitmap,
                (long)( curtime / 1000), (long)((curtime - begintime)) / 1000);
        drawText(-200, -360, 0, globalDispStat.scaleText[NODE_DISPLAY_MANET], buff);
    }

    {
        static destime lasttick = 0;
        double stepangle, cx = m->numnodes / 2.0;
        double a[3], b[3], c[3];
        int x, y;

        if (lasttick == 0)
            lasttick = getMilliTime();
        stepangle = (5.0 / 1000.0) * (getMilliTime() - lasttick);

        if ((globalGraphManet != NULL) && (globalDispStat.familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_FLOATINGGRAPH)))
        {
            glTranslatef(+300.0, -300.0, 0.0);
            glRotatef(stepangle, 0.0, 1.0, 0.0);
            glRotatef(-90.0, 1.0, 0.0, 0.0);

            glScaled(10.0, 10.0, 10.0);
            for(y = 0; y < (m->numnodes - 1); y++)
            {
                glBegin(GL_TRIANGLE_STRIP);
                glVertex3f(0.0 - cx, y - cx, globalGraphManet[0 + y * m->numnodes]);
                glVertex3f(0.0 - cx, y + 1 - cx, globalGraphManet[0 + (y + 1) * m->numnodes]);
                for(x = 0; x < (m->numnodes - 1); x++)
                {
                    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ((y & 1) ^ (x&1)) ? blue : blue2);
                    a[0] = 1.0;
                    a[1] = 0.0;
                    a[2] = globalGraphManet[(x + 1) + y * m->numnodes] - globalGraphManet[(x + 0) + y * m->numnodes];
                    b[0] = 1.0;
                    b[1] = -1.0;
                    b[2] = globalGraphManet[(x + 1) + y * m->numnodes] - globalGraphManet[(x + 0) + (y + 1) * m->numnodes];
                    crossProduct(c, a, b);
                    glVertex3f(x + 1 - cx, y - cx, globalGraphManet[(x + 1) + y * m->numnodes]);
                    glNormal3d(c[0], c[1], c[2]);

                    a[0] = 0.0;
                    a[1] = 1.0;
                    a[2] = globalGraphManet[(x + 1) + (y + 1) * m->numnodes] - globalGraphManet[(x + 1) + (y + 0) * m->numnodes];
                    b[0] = 1.0;
                    b[1] = 0.0;
                    b[2] = globalGraphManet[(x + 1) + (y + 1) * m->numnodes] - globalGraphManet[(x + 0) + (y + 1) * m->numnodes];
                    crossProduct(c, a, b);
                    glVertex3f(x + 1 - cx, y + 1 - cx, globalGraphManet[(x + 1) + (y + 1) * m->numnodes]);
                }
                glEnd();
            }
        }
    }

    glPopMatrix();

    glScalef(globalManetAdj.scaleX, globalManetAdj.scaleY, globalManetAdj.scaleZ);
    glRotatef(globalManetAdj.angleX, 1.0, 0.0, 0.0);
    glRotatef(globalManetAdj.angleY, 0.0, 1.0, 0.0);
    glRotatef(globalManetAdj.angleZ, 0.0, 0.0, 1.0);
    glTranslatef(globalManetAdj.shiftX, globalManetAdj.shiftY, globalManetAdj.shiftZ);


    /* This is cut-and-pasted from manetDraw(), for now
    */

    /* Physical links are on plane Z = 0  */
#if 1
    if (globalDispStat.familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_PHYSICAL))
    {
        phy = manetGetPhysicalGraph(m);
        if (!globalDispStat.monochromeMode)
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, physical);
        glColor4f(1.0, 0.0, 0.0, 0.5);
        drawGraph(m, phy, globalDispStat.scaleText[NODE_DISPLAY_MANET], 1);
        free(phy);
    }
#endif

    glPushMatrix();
    glTranslatef(0.0, 0.0, + 0.3);
    watcherDrawNodes(NODE_DISPLAY_MANET, m);
    glPopMatrix();

    if (globalDispStat.familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_HIERARCHY))
    {
        if (!globalDispStat.monochromeMode)
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, hierarchy);
        glPushMatrix();
        glTranslatef(0.0, 0.0, + 0.1);
        drawGraph(globalManet, globalGraphHierarchy, globalDispStat.scaleText[NODE_DISPLAY_MANET], 1);
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(0.0, 0.0, + 0.6);
    watcherGraphDraw(&userGraph, NODE_DISPLAY_MANET, &globalDispStat, m->curtime);
    floatingLabelDraw(&floatingLabelList, NODE_DISPLAY_MANET, &globalDispStat, m->curtime);
    glPopMatrix();

    glFlush();

    glutSwapBuffers();
}

static void drawHierarchy(void)
{
    static const GLfloat light_pos[] = { 0.0, 0.0, 0.0, 1.0 };
    static const GLfloat hierarchy[] = { 0.0, 0.0, 1.0, 0.5 };
    static const GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();

    glTranslatef(0.0, 0.0, -20.0);

    glPushMatrix();
    glTranslatef(0.0, 0.0, -50.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glPopMatrix();

    glScalef(0.02, 0.02, 0.10);

    glTranslatef(-400.0, 350.0, 0.0);

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, hierarchy);

    glScalef(globalHierarchyAdj.scaleX, globalHierarchyAdj.scaleY, globalHierarchyAdj.scaleZ);
    glRotatef(globalHierarchyAdj.angleX, 1.0, 0.0, 0.0);
    glRotatef(globalHierarchyAdj.angleY, 0.0, 1.0, 0.0);
    glRotatef(globalHierarchyAdj.angleZ, 0.0, 0.0, 1.0);
    glTranslatef(globalHierarchyAdj.shiftX, globalHierarchyAdj.shiftY, globalHierarchyAdj.shiftZ);

#if 1
    glPushMatrix();
    drawHierarchy(globalManet, &globalDispStat);
    glPopMatrix();
#endif

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);

    glFlush();

    glutSwapBuffers();
}

/**************************************************************************************
 *
 * These functions are the watcher's implementation of a clustering algorithm.
 *
 */


static void menuCallback(int val)
{
    if ((val & 0xF0000000) == TOGGLE)
    {
        menuToggle(menuList, val);
        if(val == COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS || 
           val == COMMUNICATIONS_LABEL_FAMILY_HIERARCHY)
        {
            if(globalAutoCenterNodesFlag && globalManet)
            {
                scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
            }
        }
        return;
    }
    if ((val & 0xF0000000) == SPEED)
    {
        globalReplay.speed = val&0x0FFFF;
        if (globalReplay.runFlag)			/* If speed is changed, reset starttime, so we don't jump  */
        {
            globalReplay.runstartfile = curtime;
            globalReplay.runstartwall = getMilliTime();
        }
        return;
    }

    switch(val)
    {
        case MENU_LABELCRITICAL:
            globalDispStat.minPriority = COMMUNICATIONS_LABEL_PRIORITY_CRITICAL;
            break;
        case MENU_LABELWARN:
            globalDispStat.minPriority = COMMUNICATIONS_LABEL_PRIORITY_WARN;
            break;
        case MENU_LABELINFO:
            globalDispStat.minPriority = COMMUNICATIONS_LABEL_PRIORITY_INFO;
            break;
        case MENU_CLEARLABELS:
            labelsClear(globalManet);
            floatingLabelNuke(&floatingLabelList);
            break;
        case MENU_CLEAREDGES:
            watcherGraphEdgeNuke(&userGraph);
            break;
        case MENU_ZOOMIN:
            zoomIn();
            break;
        case MENU_ZOOMOUT:
            zoomOut();
            break;
        case MENU_ALLON:
            globalDispStat.familyBitmap = COMMUNICATIONS_LABEL_FAMILY_ALL;
            menuUpdate(menuList);
            break;
        case MENU_ALLOFF:
            globalDispStat.familyBitmap = 0;
            menuUpdate(menuList);
            break;
        case MENU_QUIT:
            exit(1);
            break;
        case MENU_VIEWPOINTRESET:
            textZoomReset();
            arrowZoomReset();
            viewpointReset();
            break;
    }
}

/* walk the arrays of label pointers, and remove all of them
*/

static void labelsClear(manet *m)
{
    int nod;
    manetNode *us;

    for(nod = 0; nod < m->numnodes; nod++)
    {
        us = &(m->nlist[nod]);
        nodeLabelRemoveAll(us);
        us->cluster->needupdate = 1;
    }
}

/* This is called in either goodwin or live mode, to init our
 * data structures
 */
void nodeOpenSucceed(manetNode *us)
{
    int i;

    us->addr = communicationsNodeAddress(us->cluster->cs);
#ifdef DEBUG_WATCHER
    fprintf(stderr, "nodeOpenSucceed: addr is now %d\n", us->addr & 0xFF);
#endif

    us->cluster->totpackets.unicastRecCount = 0;
    us->cluster->totpackets.repUnicastXmitCount = 0;
    us->cluster->totpackets.origUnicastXmitCount = 0;
    us->cluster->totpackets.unicastRecByte = 0;
    us->cluster->totpackets.repUnicastXmitByte = 0;
    us->cluster->totpackets.origUnicastXmitByte = 0;
    us->cluster->totpackets.bcastRecCount = 0;
    us->cluster->totpackets.repBcastXmitCount = 0;
    us->cluster->totpackets.origBcastXmitCount = 0;
    us->cluster->totpackets.bcastRecByte = 0;
    us->cluster->totpackets.repBcastXmitByte = 0;
    us->cluster->totpackets.origBcastXmitByte = 0;
    us->rootflag = 0;
    us->level = 0;
    us->clusterhead = NULL;

    us->cluster->needupdate = 0;
    us->cluster->datavalid = 0;
    for(i = 0; i < (int)(sizeof(us->cluster->positionStatus) / sizeof(us->cluster->positionStatus[0])); i++)
        us->cluster->positionStatus[i] = IDSPOSITION_INACTIVE;

    communicationsNameSet(us->cluster->cs, "watcher", "");

    communicationsNeighborRegister(us->cluster->cs, detectorNeighborUpdate, us);
    communicationsStatusRegister(us->cluster->cs, 5 * 1000, detectorStatusUpdate, us);
    /* we want to know about position updates, so we need to claim eligibility */
    idsPositionRegister(us->cluster->cs, COORDINATOR_NEIGHBORHOOD, IDSPOSITION_INFORM, detectorPositionUpdate, us);
    idsPositionRegister(us->cluster->cs, COORDINATOR_REGIONAL, IDSPOSITION_INFORM, detectorPositionUpdate, us);
    idsPositionRegister(us->cluster->cs, COORDINATOR_ROOT, IDSPOSITION_INFORM, detectorPositionUpdate, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL, gotMessageFloatingLabel, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_FLOATINGLABEL_REMOVE, gotMessageFloatingLabelRemove, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL, gotMessageLabel, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_LABEL_REMOVE, gotMessageLabelRemove, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_COLOR, gotMessageColor, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE, gotMessageEdge, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_EDGE_REMOVE, gotMessageEdgeRemove, us);
    /* Only use GPS locations from clients if we're not getting them from the watchermovement stuff */
    if (!globalWatcherMovementEnableFlag)
        messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_GPS, gotMessageGPS, us);

    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH, gotMessageGraph, us);
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_GRAPH_EDGE, gotMessageGraphEdge, us);
}


/* This is called in LIVE mode to attempt to connect to a node.
*/
void nodeAttemptOpen(manetNode *us)
{
    destime tick;

    if (us->cluster->cs)     /* if we're already open, NOP  */
        return;

    tick = getMilliTime();
    if ((tick - us->cluster->lastopen) < 5000)
        return;

    us->cluster->lastopen = tick;

    fprintf(stderr, "node %d: attempting API open ", us->addr & 0xFF);
    if (us->addr)
    {
        us->cluster->cs = communicationsInit(us->cluster->controladdr);
    }
    else
        us->cluster->cs = NULL;

    if (us->cluster->cs)
        fprintf(stderr, "succeeded\n");
    else
    {
        fprintf(stderr, "failed errno = %d\n", errno);
        return;
    }

    nodeOpenSucceed(us);
}

/* This is called by DES code, to allocate our data structures, when the manet
 * is created.  Node that the manet creation does not address physical locations,
 * or connections to the hosts.
 */
void nodeInit(manetNode *us)
{
    us->cluster = (clusteringState*)malloc(sizeof(*(us->cluster)));


    us->cluster->totpackets.unicastRecCount = 0;
    us->cluster->totpackets.repUnicastXmitCount = 0;
    us->cluster->totpackets.origUnicastXmitCount = 0;
    us->cluster->totpackets.unicastRecByte = 0;
    us->cluster->totpackets.repUnicastXmitByte = 0;
    us->cluster->totpackets.origUnicastXmitByte = 0;
    us->cluster->totpackets.bcastRecCount = 0;
    us->cluster->totpackets.repBcastXmitCount = 0;
    us->cluster->totpackets.origBcastXmitCount = 0;
    us->cluster->totpackets.bcastRecByte = 0;
    us->cluster->totpackets.repBcastXmitByte = 0;
    us->cluster->totpackets.origBcastXmitByte = 0;

    /*10 is the number of families to support */
    us->cluster->labelClockError = NULL;

    if (us->index == 0)
    {
        memset(us->manet->linklayergraph, 0, us->manet->numnodes * us->manet->numnodes * sizeof(us->manet->linklayergraph[0]));
        us->manet->hierarchygraph = globalGraphHierarchy;
        globalLastMetricsTick = 0;
    }

    us->cluster->cs = NULL;
    us->cluster->lastopen = 0;
    us->cluster->controladdr = us->addr;

    //	nodeAttemptOpen(us);
}

void nodeFree(manetNode *us)
{
    communicationsClose(us->cluster->cs);
    free(us->cluster);
    free(us);
}

void nodeGotPacket(manetNode *us, packet *p)
{
    fprintf(stderr, "we should never get a packet!\n");
    exit(1);
}



/**************************************************************************************
 *
 *  These functions are the watcher's interface to the idsCommunications API.
 *
 */

void status(void *messageStatusHandlerData, MessageStatus stat)
{
    int msgnum = (int)messageStatusHandlerData;
    switch(stat)
    {
        case MESSAGE_SUCCESSFUL:
            fprintf(stderr, "message %d transmitted successfully\n", msgnum);
            break;
        default:
            fprintf(stderr, "message %d was not acked\n", msgnum);
            break;
    }
}


void gotMessageColor(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    unsigned char *pos;
    ManetAddr node;
    int colorflag;
    int index;
    int i;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);
    UNMARSHALLONG(pos, node);

    index = manetGetNodeNum(us->manet, node);
    if (index >= 0)
    {
        UNMARSHALBYTE(pos, colorflag);
        if (colorflag)
        {
            for(i = 0; i < 4; i++)
                UNMARSHALBYTE(pos, us->manet->nlist[index].cluster->color[i]);
            nodeColor(&(us->manet->nlist[index]), us->manet->nlist[index].cluster->color);

#ifdef DEBUG_WATCHER
            fprintf(stderr, "got color msg, node %d %d.%d.%d.%d\n", node & 0xFF, us->manet->nlist[index].cluster->color[0], us->manet->nlist[index].cluster->color[1], us->manet->nlist[index].cluster->color[2], us->manet->nlist[index].cluster->color[3]);
#endif

        }
        else
            nodeColor(&(us->manet->nlist[index]), NULL);
    }
    us->cluster->needupdate = 1;
}

void gotMessageFloatingLabel(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    FloatingLabel lab;
    char string[260];
    unsigned char *pos;

    lab.text = string;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);
    pos = communicationsWatcherFloatingLabelUnmarshal(pos, &lab);

    floatingLabelAdd(&floatingLabelList, &lab, curtime);

    us->cluster->needupdate = 1;
}

void gotMessageFloatingLabelRemove(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    FloatingLabel lab;
    char string[260];
    unsigned char *pos;
    int bitmap;

    lab.text = string;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);
    UNMARSHALBYTE(pos, bitmap);
    pos = communicationsWatcherFloatingLabelUnmarshal(pos, &lab);

    floatingLabelRemove(&floatingLabelList, bitmap, &lab);

    us->cluster->needupdate = 1;
}


/* The marshal function which goes with this unmarshal is in idsCommunications.cpp
*/
void gotMessageLabel(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    unsigned char *pos;
    int index;
    char string[260];
    NodeLabel lab;
    lab.text = string;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);
    pos = communicationsWatcherLabelUnmarshal(pos, &lab);

    index = manetGetNodeNum(us->manet, lab.node);    /* lookup node index from its address.  */
    if (index >= 0)    /* if we've found it... */
    {
        /*
         * if node applying label already has a label of this tag on node node, then replace it
         */
        nodeLabelRemove(&(us->manet->nlist[index]), COMMUNICATIONS_LABEL_REMOVE_TAG|COMMUNICATIONS_LABEL_REMOVE_FAMILY|COMMUNICATIONS_LABEL_REMOVE_NODE|COMMUNICATIONS_LABEL_REMOVE_PRIORITY, &lab);
        if (lab.text[0] != 0)
            nodeLabelApply(&(us->manet->nlist[index]), &lab);
    }

    us->cluster->needupdate = 1;
}

void gotMessageLabelRemove(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    int i;
    int bitmap;
    char string[260];
    NodeLabel lab;
    lab.text = string;
    unsigned char *pos;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);
    UNMARSHALBYTE(pos, bitmap);
    pos = communicationsWatcherLabelUnmarshal(pos, &lab);

    for(i = 0; i < us->manet->numnodes; i++)
        nodeLabelRemove(&(us->manet->nlist[i]), bitmap, &lab);

    us->cluster->needupdate = 1;
}


void gotMessageEdge(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    unsigned char *pos;
    NodeEdge *e;
    int msgflag = 0;
    int i;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);

    e = (NodeEdge *)malloc(sizeof(*e) + 256 * 3);
    e->labelHead.text = ((char*)e) + sizeof(*e);
    e->labelMiddle.text = (e->labelHead.text) + 256;
    e->labelTail.text = (e->labelMiddle.text) + 256;

    e->next = NULL;
    UNMARSHALLONG(pos, e->head);
    UNMARSHALLONG(pos, e->tail);
    UNMARSHALLONG(pos, e->tag);
    UNMARSHALBYTE(pos, e->family);
    UNMARSHALBYTE(pos, e->priority);
    for(i = 0; i < 4; i++)
        UNMARSHALBYTE(pos, e->color[i]);

    UNMARSHALBYTE(pos, msgflag);
    if (msgflag & 1)
        pos = communicationsWatcherLabelUnmarshal(pos, &(e->labelHead));
    else
        e->labelHead.text = NULL; 		/* not memory leak because we malloced a big chunk for e, and left pieces for the text blocks... */
    if (msgflag & 2)
        pos = communicationsWatcherLabelUnmarshal(pos, &(e->labelMiddle));
    else
        e->labelMiddle.text = NULL; 		/* not memory leak because we malloced a big chunk for e, and left pieces for the text blocks... */
    if (msgflag & 4)
        pos = communicationsWatcherLabelUnmarshal(pos, &(e->labelTail));
    else
        e->labelTail.text = NULL; 		/* not memory leak because we malloced a big chunk for e, and left pieces for the text blocks... */


    /* file version probiem...  earlier goodwin files will not have these fields  */

    if ((size_t)(pos - (unsigned char*)messageInfoRawPayloadGet(mi)) < messageInfoRawPayloadLenGet(mi))
    {
        UNMARSHALBYTE(pos, e->width);
    }
    else
        e->width = 15;

    if ((size_t)(pos - (unsigned char*)messageInfoRawPayloadGet(mi)) < messageInfoRawPayloadLenGet(mi))
    {
        UNMARSHALLONG(pos, e->expiration);
    }
    else
        e->expiration = 0;

    e->nodeHead = manetNodeSearchAddress(us->manet, e->head);
    e->nodeTail = manetNodeSearchAddress(us->manet, e->tail);
    watcherGraphEdgeInsert(&userGraph, e, us->manet->curtime);
    us->cluster->needupdate = 1;
}

void gotMessageEdgeRemove(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    unsigned char *pos;
    NodeEdge buff, *e = &buff;
    int bitmap;

    pos = (unsigned char *)messageInfoRawPayloadGet(mi);

    UNMARSHALBYTE(pos, bitmap);
    UNMARSHALLONG(pos, e->head);
    UNMARSHALLONG(pos, e->tail);
    UNMARSHALBYTE(pos, e->family);
    UNMARSHALBYTE(pos, e->priority);
    UNMARSHALLONG(pos, e->tag);

    watcherGraphEdgeRemove(&userGraph, bitmap, e);
    us->cluster->needupdate = 1;
}

void gotMessageGPS(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    WatcherGPS *location;
    destime tim;

    if (messageInfoOriginatorGet(mi) != us->addr)
        return;

    location = watcherGPSUnmarshal(messageInfoRawPayloadGet(mi), messageInfoRawPayloadLenGet(mi));
    if (location == NULL)
    {
        fprintf(stderr, "GPS corrupt\n");
        return;
    }

    us->x = location->lon * GPSScale;
    us->y = location->lat * GPSScale;
    us->z = location->alt - 20;
    globalGpsValidFlag[us->index] = 1;

    tim = location->time;      /* GPS time in milliseconds (idsCommunication's canonical unit)  */

    //	fprintf(stderr, "node %d at %f %f %f\n", us->index, lat[us->index], lon[us->index], alt[us->index]);

    if(globalAutoCenterNodesFlag)
    {
        scaleAndShiftToCenter(us->manet, ScaleAndShiftUpdateOnChange);
    }
    else
    {
        static time_t tick = 0;
        static int repcnt = 0;
        time_t now = time(0);
        if(now > tick)
        {
            if(repcnt)
            {
                fprintf(stderr, "******************don't center nodes (%d times)******************\n", repcnt);
                repcnt = 0;
            }
            else
            {
                fprintf(stderr, "******************don't center nodes******************\n");
            }
            tick = now + 5;
        }
        else
        {
            ++repcnt;
        }
    }

    free(location);
} // gotMessageGPS

void gotMessageGraph(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;

    unsigned char *payload = (unsigned char*)messageInfoRawPayloadGet(mi);

    graphUnmarshal(globalGraphManet, us->manet->numnodes, payload);
}

void gotMessageGraphEdge(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;
    CommunicationsGraphEdge edge;
    int dat;
    unsigned char *hp;
    int a, b;

    unsigned char *payload = (unsigned char*)messageInfoRawPayloadGet(mi);
    int payloadlen = messageInfoRawPayloadLenGet(mi);

#ifdef DEBUG_WATCHER
    fprintf(stderr, "edge, payloadlen %d\n", payloadlen);
#endif

    for(a = 0; a < (us->manet->numnodes * us->manet->numnodes); a++)
        globalGraphManet[a] = 0;

    for(hp = payload; hp < payload + payloadlen;)
    {
        UNMARSHALLONG(hp, edge.a);
        UNMARSHALLONG(hp, edge.b);
        UNMARSHALSHORT(hp, dat);
        edge.value = dat / 100.0;

        a = manetGetNodeNum(us->manet, edge.a);
        b = manetGetNodeNum(us->manet, edge.b);

        fprintf(stderr, "got an edge, %d.%d.%d.%d (%d)  to %d.%d.%d.%d (%d)  length %f\n", PRINTADDR(edge.a), a, PRINTADDR(edge.b), b, edge.value);

        if ((a >= 0) && (b >= 0))
            globalGraphManet[a + b * us->manet->numnodes] = edge.value * globalGraphScaleManet;
    }
}


/* This is called when a neighbor arrives or departs.  
 * I'm using it as a signal to redraw the display.
 */
static void detectorNeighborUpdate(void *data, CommunicationsNeighbor *cn)
{
    manetNode *us = (manetNode*)data;

#ifdef DEBUG_WATCHER
    fprintf(stderr, "node %d: neighbor %d distance %d %s\n", us->addr & 0xff, cn->addr & 0xFF, cn->distance, communicationsNeighborState2Str(cn->state));
#endif
    us->cluster->needupdate = 1;
}

/* This callback is called when the API receives an ApiStatus message
 * That message is a debugging call, and carries some information that is
 * useful for debugging the clustering algorithm.
 */
static void detectorStatusUpdate(void *data, ApiStatus *as)
{
    manetNode *us = (manetNode*)data;
    int i;
    double duration;
    ApiPacketCount total;

    us->level = as->level;
    //	us->rootflag = as->rootflag;   /* Now read from postion update callback.  */

    us->cluster->lasttimewatcher = curtime;

    duration = as->timestamp - us->cluster->lasttime;
    us->cluster->lasttime = as->timestamp;

    total.unicastRecCount = 0;
    total.repUnicastXmitCount = 0;
    total.origUnicastXmitCount = 0;
    total.unicastRecByte = 0;
    total.repUnicastXmitByte = 0;
    total.origUnicastXmitByte = 0;
    total.bcastRecCount = 0;
    total.repBcastXmitCount = 0;
    total.origBcastXmitCount = 0;
    total.bcastRecByte = 0;
    total.repBcastXmitByte = 0;
    total.origBcastXmitByte = 0;

    //	fprintf(stderr, "node %d: got status update\n", us->addr & 0xFF);

    for(i = 0; i < as->numtypes; i++)
    {
#if 1
        fprintf(stderr, "bandwidth: node %d time %lld type 0x%x %lld %lld %lld %lld %lld %lld %lld %lld \n", us->addr & 0xff, as->timestamp,  as->packetList[i].type,
                as->packetList[i].unicastRecCount,
                as->packetList[i].repUnicastXmitCount + as->packetList[i].origUnicastXmitCount,
                as->packetList[i].unicastRecByte,
                as->packetList[i].repUnicastXmitByte + as->packetList[i].origUnicastXmitByte,
                as->packetList[i].bcastRecCount,
                as->packetList[i].repBcastXmitCount + as->packetList[i].origBcastXmitCount,
                as->packetList[i].bcastRecByte,
                as->packetList[i].repBcastXmitByte + as->packetList[i].origBcastXmitByte);
#endif

#define DOVAR(x) total.x += as->packetList[i].x

        DOVAR(unicastRecCount);
        DOVAR(repUnicastXmitCount);
        DOVAR(origUnicastXmitCount);
        DOVAR(unicastRecByte);
        DOVAR(repUnicastXmitByte);
        DOVAR(origUnicastXmitByte);
        DOVAR(bcastRecCount);
        DOVAR(repBcastXmitCount);
        DOVAR(origBcastXmitCount);
        DOVAR(bcastRecByte);
        DOVAR(repBcastXmitByte);
        DOVAR(origBcastXmitByte);
#undef DOVAR
    }

#define DOVAR(x) \
    us->cluster->curpersec.x = (us->cluster->totpackets.x > 0.0) ? (long long int)((( total.x - us->cluster->totpackets.x ) / duration) * 1000.0) : 0;\
    us->cluster->totpackets.x = total.x

    DOVAR(unicastRecCount);
    DOVAR(repUnicastXmitCount);
    DOVAR(origUnicastXmitCount);
    DOVAR(unicastRecByte);
    DOVAR(repUnicastXmitByte);
    DOVAR(origUnicastXmitByte);
    DOVAR(bcastRecCount);
    DOVAR(repBcastXmitCount);
    DOVAR(origBcastXmitCount);
    DOVAR(bcastRecByte);
    DOVAR(repBcastXmitByte);
    DOVAR(origBcastXmitByte);

#undef DOVAR

#if 0
    fprintf(stderr, "node %d: type 0x%x %lld %lld %lld %lld %lld %lld %lld %lld \n", us->addr & 0xff,  0,
            us->cluster->curpersec.unicastRecCount,
            us->cluster->curpersec.unicastXmitCount,
            us->cluster->curpersec.unicastRecByte,
            us->cluster->curpersec.unicastXmitByte,
            us->cluster->curpersec.bcastRecCount,
            us->cluster->curpersec.bcastXmitCount,
            us->cluster->curpersec.bcastRecByte,
            us->cluster->curpersec.bcastXmitByte);
#endif
    us->cluster->needupdate = 1;
    us->cluster->datavalid = 1;
}

static void detectorPositionUpdate(void *data, IDSPositionType pos, IDSPositionStatus stat)
{
    manetNode *us = (manetNode*)data;

#ifdef DEBUG_WATCHER
    fprintf(stderr, "node %d: got position update  %s is %s\n", us->addr & 0xFF, idsPosition2Str(pos), idsPositionStatus2Str(stat));
#endif
    us->cluster->positionStatus[pos] = stat;
    if (pos == COORDINATOR_ROOT)
        us->rootflag = (stat == IDSPOSITION_ACTIVE);

    us->cluster->needupdate = 1;
}


#define GETMAXFD(mfd, nfd) do { (mfd) = ((nfd) > (mfd)) ? (nfd) : (mfd); } while(0)

/* When we are in playback mode, this will call comunicationsLogStep,
 * Otherwise it calls communicationsReadReady.
 */

void checkIOGoodwin(int arg)
{
    manet *m = globalManet;

    glutTimerFunc(100, checkIOGoodwin, 0);

    if (curtime == 0)
    {
        int i, j;
        CommunicationsStatePtr const *cs;

        cs = communicationsLogNodesGet(globalGoodwin);
        for(i = 0; cs[i]; i++)
        {
            j = manetGetNodeNum(m, communicationsNodeAddress(cs[i]));
            if (j < 0)
                continue;

            fprintf(stderr, "initing node %d.%d.%d.%d\n", PRINTADDR(communicationsNodeAddress(cs[i])));
            m->nlist[j].cluster->cs = cs[i];
            nodeOpenSucceed(&m->nlist[j]);
        }
    }

    if ((globalReplay.step > 0) || (globalReplay.runFlag))
    {
        int i;
        destime starttime;
        if (globalReplay.runFlag)     /* if we're in run mode, look at wallclock timestamps, and then step that far in sim time  */
        {
            destime tmp;
            /* we want to find the difference between when we are, and when it is.  And then step forward that far */

            tmp = (destime)(double(getMilliTime() - globalReplay.runstartwall) * (globalReplay.speed / 4.0)) + globalReplay.runstartfile;    /* this is wall time, converted to file time  */
            globalReplay.step = tmp - curtime;
        }

        if (globalReplay.step > 0)
        {
            starttime = m->curtime;
            while(m->curtime < starttime + globalReplay.step)
            {
                long nevents;
                if (((nevents = communicationsLogStep(globalGoodwin, 1, &m->curtime)) < 0) && (globalExitAtEofFlag))
                    exit(0);
                if (nevents > 0 && globalDoMetricsFlag)
                {
                    nodeMobilityCountEdges(&m->nlist[0], 0);
                    nodeHierarchyCountEdges(&m->nlist[0], 0);

                    if ((m->curtime - globalLastMetricsTick) > 1000)
                    {
                        update(m);
                        nodeMetrics(&(m->nlist[0]), 0);
                        globalLastMetricsTick = m->curtime;
                    }
                }
            }
            update(m);
            if ((globalReplay.stoptime > 0) && (globalManet->curtime > globalReplay.stoptime))
                exit(0);
        }
#if 0
        glutPostWindowRedisplay(globalWindowManet);
        glutPostWindowRedisplay(globalWindowHierarchy);
        glutPostRedisplay();
#endif
        for(i = 0; i < m->numnodes; i++)
            m->nlist[i].cluster->needupdate = 0;
    }
    curtime = communicationsLogTimeGet(globalGoodwin);
    globalReplay.step = - 1;
    glutPostWindowRedisplay(globalWindowManet);
    glutPostWindowRedisplay(globalWindowHierarchy);
    glutPostRedisplay();
}

void checkIOLive(int arg)
{
    struct timeval timeout = { 0, 0 };
    int needupdate = 0;
    manet *m = globalManet;
    int movementFd = - 1; 
    curtime = getMilliTime();

    while(1)
    {
        int maxfd = - 1;
        int i;
        fd_set readfds;
        FD_ZERO(&readfds);

        for(i = 0; i < m->numnodes; i++)
        {
            if (m->nlist[i].cluster->cs == NULL)     /* If never managed to open  */
            {
                // try every 5 seconds
                if ((curtime - m->nlist[i].cluster->lastopen) > 5000)
                {
                    nodeAttemptOpen(&(m->nlist[i]));
                }
            }
            else      /* we did successfully open, but its not currently open, AND time for retry.   */
            {
                if ((communicationsReturnFD(m->nlist[i].cluster->cs) < 0) && ((curtime - m->nlist[i].cluster->lastopen) > 5000))
                {
                    if(communicationsReadReady(m->nlist[i].cluster->cs) == 0)
                    {
                        m->nlist[i].addr = communicationsNodeAddress(m->nlist[i].cluster->cs);
                    }
                    m->nlist[i].cluster->lastopen = curtime;
                }
            }

            if ((m->nlist[i].cluster->cs) && (communicationsReturnFD(m->nlist[i].cluster->cs) >= 0))
            {
                FD_SET(communicationsReturnFD(m->nlist[i].cluster->cs), &readfds);
                GETMAXFD(maxfd, communicationsReturnFD(m->nlist[i].cluster->cs));
            }
        }

        if (globalWatcherMovementEnableFlag)
            movementFd = watcherMovementFD(globalWatcherMovementState);
        if (movementFd >= 0)
        {
            FD_SET(movementFd, &readfds);
            GETMAXFD(maxfd, movementFd);
        }

        if (maxfd >= 0)
        {
            int rc = select(maxfd + 1, &readfds, NULL, NULL, &timeout);

            curtime = getMilliTime();

            if (rc == 0)
                break;

            if (rc < 1)
            {
                perror("checkIO");
                break;
            }

            for(i = 0; i < m->numnodes; i++)
            {
                if ((m->nlist[i].cluster->cs) && (communicationsReturnFD(m->nlist[i].cluster->cs) >= 0))
                    if (FD_ISSET(communicationsReturnFD(m->nlist[i].cluster->cs), &readfds))
                        if (communicationsReadReady(m->nlist[i].cluster->cs) < 0)    /* This actually means that the API lost its connection, and we're going to do the reconnect thing  */
                        {
                            m->nlist[i].cluster->lastopen = curtime;
                            needupdate = 1;
                        }
                if (m->nlist[i].cluster->needupdate)
                    needupdate = 1;
            }

            if (movementFd >= 0)
            {
                if (FD_ISSET(movementFd, &readfds))
                {
                    if (watcherMovementRead(globalWatcherMovementState, globalManet))
                    {
                        needupdate = 1;
                        /* And if we're recording, stuff a bunch of GPS messages here */
                    }
                }
            }
            else
                if (globalWatcherMovementEnableFlag)
                {
                    if (watcherMovementUpdate(globalWatcherMovementState, globalManet))
                        needupdate = 1;
                }

            if (needupdate)
            {
                update(m);

                for(i = 0; i < m->numnodes; i++)
                    m->nlist[i].cluster->needupdate = 0;
                m->curtime = getMilliTime();
                if ((m->curtime - globalLastMetricsTick) > 1000)
                {
                    nodeMetrics(&(m->nlist[0]), 0);
                    globalLastMetricsTick = m->curtime;
                }
            }
        }
        else
            break;
    }

    glutPostWindowRedisplay(globalWindowManet);
    glutPostWindowRedisplay(globalWindowHierarchy);
    glutPostRedisplay();

    glutTimerFunc(100, checkIOLive, 0);

    return;
} // checkIOLive


/* This function calls the idsCommunications API to get each node's neighbor list.
 * That data is then converted into the simulator support code's internal link status
 * graph. 
 *
 * Relevant only to watcher
 */
static void update(manet *m)
{
    int i, j;
    CommunicationsNeighbor *n;

    memset(m->linklayergraph, 0, m->numnodes * m->numnodes * sizeof(m->linklayergraph[0]));
    memset(globalGraphHierarchy, 0, m->numnodes * m->numnodes * sizeof(int));
    for(i = 0; i < m->numnodes; i++)
    {
        if (m->nlist[i].cluster->cs == NULL)
            continue;
        n = communicationsNeighborList(m->nlist[i].cluster->cs);

        m->nlist[i].clusterhead = NULL;
        for(; n; n = n->next)
        {
            j = manetGetNodeNum(m, n->addr);
            if (j < 0)
                continue;

            if (n->distance == 1)
            {
                m->linklayergraph[i *m->numnodes + j] = 1;
            }
#if 1
            if ((n->type&COMMUNICATIONSNEIGHBOR_PARENT) &&
                    (!(m->nlist[i].cluster->positionStatus[COORDINATOR_ROOT] == IDSPOSITION_ACTIVE))
               )
            {
                if (n->distance <= MAXHOP)
                    globalGraphHierarchy[i *m->numnodes + j] = n->distance;
                m->nlist[i].clusterhead = &(m->nlist[i].cluster->fakeCH);
                m->nlist[i].cluster->fakeCH.addr = n->addr;
            }
#endif
#if 0
            if (n->type&COMMUNICATIONSNEIGHBOR_CHILD)
            {
                globalGraphHierarchy[i *m->numnodes + j] = n->distance;
            }
#endif
        }
    }
}

static void glInit()
{
    GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_AMBIENT, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

    glClearColor(1.0, 1.0, 1.0, 0.0);
}

static void manetWindowInit(void)
{
    size_t i;
    int labelmenu;
    int speedmenu = -1;
    if ((globalWindowManet = glutCreateWindow("MANET")) == GL_FALSE)
    {
        exit(1);
    }

    glInit();

    if (globalWatcherMovementEnableFlag)
        globalWatcherMovementState = watcherMovementInit(globalManet);

    viewpointReset();

    labelmenu = glutCreateMenu(menuCallback);
    glutAddMenuEntry("Critical", MENU_LABELCRITICAL);
    glutAddMenuEntry("Warning", MENU_LABELWARN);
    glutAddMenuEntry("Info", MENU_LABELINFO);

    if (globalGoodwin)
    {
        speedmenu = glutCreateMenu(menuCallback);
        glutAddMenuEntry("1/4", MENU_SPEED1_4);
        glutAddMenuEntry("1/2", MENU_SPEED1_2);
        glutAddMenuEntry("1", MENU_SPEED1);
        glutAddMenuEntry("2", MENU_SPEED2);
        glutAddMenuEntry("4", MENU_SPEED4);
        glutAddMenuEntry("8", MENU_SPEED8);
        glutAddMenuEntry("16", MENU_SPEED16);
        glutAddMenuEntry("32", MENU_SPEED32);
    }

    globalPopupMenuHandle = glutCreateMenu(menuCallback);
    glutAddMenuEntry("Zoom In (w)", MENU_ZOOMIN);
    glutAddMenuEntry("Zoom Out (q)", MENU_ZOOMOUT);
    glutAddMenuEntry("Reset Viewpoint (^r)", MENU_VIEWPOINTRESET);
    glutAddMenuEntry("Clear Labels", MENU_CLEARLABELS);
    glutAddMenuEntry("Clear Edges", MENU_CLEAREDGES);
    glutAddMenuEntry("Show all layers", MENU_ALLON);
    glutAddMenuEntry("Hide all layers", MENU_ALLOFF);

    i = 0;
    menuList[i] = menuLayerTogglingEntryAdd("Bandwidth (b)", i, COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Sanity Checks", i, COMMUNICATIONS_LABEL_FAMILY_SANITYCHECK);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Antenna Radius", i, COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Neighbors", i, COMMUNICATIONS_LABEL_FAMILY_PHYSICAL);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Hierarchy", i, COMMUNICATIONS_LABEL_FAMILY_HIERARCHY);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Routing", i, COMMUNICATIONS_LABEL_FAMILY_ROUTING);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Routing Onehop", i, COMMUNICATIONS_LABEL_FAMILY_ROUTING_ONEHOP);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("WormholeRouting", i, COMMUNICATIONS_LABEL_FAMILY_ROUTING2);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("WormholeRouting Onehop", i, COMMUNICATIONS_LABEL_FAMILY_ROUTING2_ONEHOP);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Floating Graph", i, COMMUNICATIONS_LABEL_FAMILY_FLOATINGGRAPH);
    i++;

    menuList[i] = menuLayerTogglingEntryAdd("Anom Paths", i, COMMUNICATIONS_LABEL_FAMILY_ANOMPATHS);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Normal Paths", i, COMMUNICATIONS_LABEL_FAMILY_NORMPATHS);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Correlators", i, COMMUNICATIONS_LABEL_FAMILY_CORRELATION);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Correlated Paths", i, COMMUNICATIONS_LABEL_FAMILY_CORRELATION_3HOP);
    i++;
    menuList[i] = menuLayerTogglingEntryAdd("Wormhole Alerts", i, COMMUNICATIONS_LABEL_FAMILY_ALERT);
    i++;

    menuList[i] = menuLayerTogglingEntryAdd("Undefined", i, COMMUNICATIONS_LABEL_FAMILY_UNDEFINED);
    i++;

    glutAddSubMenu("Label Min Priority", labelmenu);
    if (globalGoodwin)
        glutAddSubMenu("Playback speed", speedmenu);

    glutAddMenuEntry("Quit", MENU_QUIT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutReshapeFunc(ReshapeManet);
    glutKeyboardFunc(Key);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutSpecialFunc(Key2);
    glutDisplayFunc(drawManet);
    return;
} /* manetWindowInit */

static void hierarchyWindowInit()
{
    globalWindowHierarchy = glutCreateWindow("Hierarchy");
    glInit();
    viewpointReset();
    glutReshapeFunc(ReshapeHierarchy);
    glutKeyboardFunc(Key);
    glutSpecialFunc(Key2);
    glutDisplayFunc(drawHierarchy);
    return;
} /* hierarchyWindowInit */


int main(int argc, char **argv)
{
    Config *conf;
    char *goodwinfilename;
    int goodwinfd;
    int i, ch;
    destime starttime = 0;
    int relativestart = 0;
    int relativestop = 0;
    int startrunning = 0;
    int winx = 0, winy = 0, winwidth = 500, winheight = 500, winpos = 0;

    globalDispStat.minPriority = COMMUNICATIONS_LABEL_PRIORITY_INFO;
    globalDispStat.familyBitmap = COMMUNICATIONS_LABEL_FAMILY_ALL;
    globalDispStat.scaleText[NODE_DISPLAY_MANET] = 0.08;
    globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] = 0.08;
    globalDispStat.scaleLine[NODE_DISPLAY_MANET] = 1.0;
    globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] = 1.0;
    globalDispStat.monochromeMode = 0;

    globalReplay.speed = 4;
    int shiftSet = 0;   /* flag to indicate if the user specified an initial display location */
    char *argv0 = argv[0];


    while((ch = getopt(argc, argv, "ira:dems:t:l:p:c:?w:")) != -1)
    {
        switch(ch)
        {
            case 'i':
                globalDoMetricsFlag = 1;
                break;
            case 'r':
                startrunning = 1;
                break;
            case 'a':
                sscanf(optarg, "%d", &globalReplay.speed);
                break;
            case 'd':
                globalShowPositionFlag = 1;
                break;
            case 'e':
                globalExitAtEofFlag = 1;
                break;
            case 'm':
                globalWatcherMovementEnableFlag = 1;
                break;
            case 'l':
                sscanf(optarg, "%x", &i);
                globalDispStat.familyBitmap = i;
                break;
            case 'p':
                {
                    char *sep = ",";
                    char *pos;
                    char *num;

                    num = strtok_r(optarg, sep, &pos);
                    if (num)
                        sscanf(num, "%f", &globalManetAdjInit.shiftX);
                    num = strtok_r(NULL, sep, &pos);
                    if (num)
                        sscanf(num, "%f", &globalManetAdjInit.shiftY);
                    if (pos)
                        sscanf(pos, "%f", &globalManetAdjInit.shiftZ);
                    shiftSet = 1;
                    fprintf(stderr, "initial location %f %f %f\n",
                            globalManetAdjInit.shiftX, globalManetAdjInit.shiftY, globalManetAdjInit.shiftZ);
                }
                break;
            case 'w':
                {
                    char *sep = ",";
                    char *pos;
                    char *num;

                    num = strtok_r(optarg, sep, &pos);
                    if (num)
                        sscanf(num, "%d", &winwidth);
                    num = strtok_r(NULL, sep, &pos);
                    if (num)
                        sscanf(num, "%d", &winheight);
                    num = strtok_r(NULL, sep, &pos);
                    if (num)
                    {
                        winpos = 1;
                        sscanf(num, "%d", &winx);
                    }
                    num = strtok_r(NULL, sep, &pos);
                    if (num)
                        sscanf(num, "%d", &winy);
                }
                break;
            case 'c':
                {
                    float tmp;
                    sscanf(optarg, "%f", &tmp);
                    globalManetAdjInit.scaleX = tmp;
                    globalManetAdjInit.scaleY = tmp;
                    globalManetAdjInit.scaleZ = 0.10;
                    fprintf(stderr, "initial scale %f\n", globalManetAdjInit.scaleX);
                }
            case 's':
                {
                    if (optarg[0] == '@')
                    {
                        sscanf(optarg + 1, "%lld", &starttime);
                        relativestart = 1;
                    }
                    else
                        sscanf(optarg, "%lld", &starttime);
                    starttime *= 1000;
                    fprintf(stderr, "start at %lld\n", starttime);
                }
                break;
            case 't':
                {
                    if (optarg[0] == '@')
                    {
                        sscanf(optarg + 1, "%lld", &globalReplay.stoptime);
                        relativestop = 1;
                    }
                    else
                        sscanf(optarg, "%lld", &globalReplay.stoptime);
                    globalReplay.stoptime *= 1000;
                    fprintf(stderr, "stop at %lld\n", globalReplay.stoptime);
                }
                break;
            case '?':
            default:
                fprintf(stderr,
                        "watcher configfile goodwinfile - displays telemetry from a set of livenetwork demons\n"
                        "-m - get positions from watcherMovement module\n"
                        "-l hexnum - set layer bitmap\n"
                        "-p float,float,float - set initial position\n"
                        "-c float - set initial scale\n"
                        "-d - show the position and scale\n"
                        "-e - exit at EOF when playing a goodwin file\n"
                        "-s timestamp - skip to absolute timestamp\n"
                        "             - @timestamp means timestamp is relative to the start of the file\n"
                        "-t timestamp - exit at absolute timestamp\n"
                        "             - @timestamp means timestamp is relative to the start of the file\n"
                        "-r - Start in run mode, instead of pause\n"
                        "-a [int] - set the initial playback speed.  The units are in quarters of real time\n"
                        "           so 4 is realtime, 1 is 1/4 speed, and 8 is 2X speed\n"
                        "-w int,int[,int,int] - specify location of the window, like the --geometry option\n"
                        "             on a standard X program, the numbers are width,height,X,Y in pixels\n"
                        "-i - execute the simulator land metrics\n"
                       );
                exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    if(argc > 0)
    {
        fprintf(stderr, "loading config: %s\n", argv[0]);
        conf = configLoad(argv[0]);
    }
    else
    {
        fprintf(stderr, "loading default config\n");
        conf = configLoad(0);
    }
    if (conf == NULL)
    {
        fprintf(stderr, "could not open config file %s!\n", argc > 0 ? argv[0] : "");
        exit(1);
    }

    globalDispStat.monochromeMode = configSearchInt(conf, "watcher_monochrome");
    GPSScale = configSetDouble(conf, "watcher_gpsscale", 80000);

    goodwinfilename = argc > 1 ? argv[1] : NULL;
    if (goodwinfilename)
    {
        goodwinfd = open(goodwinfilename, O_RDONLY);
        if (goodwinfd < 0)
        {
            fprintf(stderr, "could not open goodwin file!\n");
            exit(1);
        }
        globalGoodwin = communicationsLogLoad(goodwinfd);
    }
    else
        globalGoodwin = NULL;

    /* In playback mode, do we load a config file, or a goodwin file?
     *   I think we want to load both, to allow a config file to specify a locations file in the
     *   event that location telemetry is not available.
     * How to create manet struct if its a goodwin file?  
     *  How to override numnodes?
     */

    globalManet = manetInit(conf, 0);
    globalGraphHierarchy = (int*)malloc(globalManet->numnodes * globalManet->numnodes * sizeof(int));
    memset(globalGraphHierarchy, 0, globalManet->numnodes * globalManet->numnodes * sizeof(int));

    if (globalGoodwin)
    {
        CommunicationsStatePtr const *cs;
        cs = communicationsLogNodesGet(globalGoodwin);

        for(i = 0; i < globalManet->numnodes; i++)
        {
            mobilityInit(&globalManet->nlist[i]);
        }
        for(i = 0; i < globalManet->numnodes; i++)
        {
            /* Must do this *after* all the mobilityInit() calls,
             * because the last one will reset all of these addrs
             * to the values from the locations file (which may be
             * control network addresses).   -dkindred 2007-05-17
             */
            globalManet->nlist[i].addr = communicationsNodeAddress(cs[i]);
        }
    }
    else
        for(i = 0; i < globalManet->numnodes; i++)
            mobilityInit(&globalManet->nlist[i]);

    firstStep(globalManet, 0);

    globalGraphManet = (float*)calloc(globalManet->numnodes * globalManet->numnodes * sizeof(globalGraphManet[0]), 1);
    globalGraphScaleManet = configSearchDouble(conf, "graphscale");
    if (globalGraphScaleManet <= 0.0)
        globalGraphScaleManet = 1.0;


    if (Args(argc, argv) == GL_FALSE)
    {
        exit(1);
    }
    /* glutInit requires argv[0] to contain the program name */
    --argv;
    ++argc;
    *argv = argv0;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    if (winpos)
        glutInitWindowPosition(winx, winy);
    glutInitWindowSize(winwidth, winheight);

    if (!shiftSet)
    {
        globalManetAdjInit.shiftX = -(configSetDouble(globalManet->conf, "areaX", 100.0) / 2.0);
        globalManetAdjInit.shiftY = -(configSetDouble(globalManet->conf, "areaY", 100.0) / 2.0);
        globalManetAdjInit.shiftZ = 0.0;

    }
    hierarchyWindowInit();
    manetWindowInit();

    if (globalGoodwin != NULL)
    {
        checkIOGoodwin(0);
        begintime = communicationsLogNextEventTimeGet(globalGoodwin);
    }
    else
    {
        checkIOLive(0);
        begintime = getMilliTime();
    }

    if ((globalGoodwin) && (starttime > 0))
    {
        if (relativestart)
            starttime += begintime;
        if (relativestop)
            globalReplay.stoptime += begintime;

        destime steptime = starttime - begintime;

        if (steptime > 0)
        {
            destime runtime;
            destime lastmetricstime;

#ifdef DEBUG_WATCHER
            fprintf(stderr, "stepping %lld\n", steptime);
#endif

            globalManet->curtime = communicationsLogTimeGet(globalGoodwin);

            runtime = globalManet->curtime;
            lastmetricstime = runtime;
            while(globalManet->curtime < runtime + steptime)
            {
                long nevents;
                if ((nevents = communicationsLogStep(globalGoodwin, 1, &globalManet->curtime)) < 0)
                {
                    fprintf(stderr, "hit EOF, walking forward to start time\n");
                    exit(1);
                }
                if (nevents > 0 && globalDoMetricsFlag)
                {
                    nodeMobilityCountEdges(&globalManet->nlist[0], 0);
                    nodeHierarchyCountEdges(&globalManet->nlist[0], 0);

                    if ((globalManet->curtime - lastmetricstime) > 1000)
                    {
                        update(globalManet);
                        nodeMetrics(&(globalManet->nlist[0]), 0);
                        lastmetricstime = globalManet->curtime;
                    }
                }
            }
        }
        curtime = communicationsLogTimeGet(globalGoodwin);
    }

    if (startrunning)
    {
        globalReplay.runFlag = 1;
        globalReplay.runstartfile = curtime;
        globalReplay.runstartwall = getMilliTime();
    }

    update(globalManet);

    glutMainLoop();
    return 0;
}

void packetSend(manetNode *us, packet *p, int origflag)
{
    fprintf(stderr, "This is the watcher, this shouldn't be called\n");
    abort();
}


static void watcherDrawNodes(NodeDisplayType dispType, manet *m)
{
    int i, j;
    char buff[160];
    GLfloat xmit[] = { 1.0, 0.1, 0.1, 0.6 };
    GLfloat rec[] = { 0.1, 1.0, 0.1, 0.6 };
    GLfloat frame[] = { 0.0, 0.0, 0.0, 0.6 };

    int *levels;

    levels = manetGetHierarchyLevels(m);

    GLfloat maxcount = 0.0, maxbyte = 0.0;
    GLfloat barheight = 30.0;
    GLfloat barwidth = 6.0;
    GLfloat countscale, bytescale;
    GLfloat x, y, z;
    for(i = 0; i < m->numnodes; i++)
    {
#define GETMAX(max, dat)  if ((dat) > (max)) (max) = (dat)
        GETMAX(maxcount, m->nlist[i].cluster->curpersec.unicastRecCount);
        GETMAX(maxcount, m->nlist[i].cluster->curpersec.repUnicastXmitCount);
        GETMAX(maxcount, m->nlist[i].cluster->curpersec.origUnicastXmitCount);
        GETMAX(maxbyte, m->nlist[i].cluster->curpersec.unicastRecByte);
        GETMAX(maxbyte, m->nlist[i].cluster->curpersec.repUnicastXmitByte);
        GETMAX(maxbyte, m->nlist[i].cluster->curpersec.origUnicastXmitByte);
        GETMAX(maxcount, m->nlist[i].cluster->curpersec.bcastRecCount);
        GETMAX(maxcount, m->nlist[i].cluster->curpersec.repBcastXmitCount);
        GETMAX(maxcount, m->nlist[i].cluster->curpersec.origBcastXmitCount);
        GETMAX(maxbyte, m->nlist[i].cluster->curpersec.bcastRecByte);
        GETMAX(maxbyte, m->nlist[i].cluster->curpersec.repBcastXmitByte);
        GETMAX(maxbyte, m->nlist[i].cluster->curpersec.origBcastXmitByte);
#undef GETMAX
    }
    countscale = barheight / maxcount;
    bytescale = barheight / maxbyte;

    for(i = 0; i < m->numnodes; i++)
    {
        NodeDisplayStatus const *ds;
        double ndx;
        double ndy;
        void (*nodeDrawFn)(manetNode *, NodeDisplayType, NodeDisplayStatus const *) = (!((m->nlist[i].cluster->cs) && (communicationsLinkup(m->nlist[i].cluster->cs)))) ? nodeDrawFrowny : nodeDraw;
        NodeDisplayStatus everythingStat = globalDispStat;
        everythingStat.familyBitmap = 0xFFFFFFFF;
        everythingStat.minPriority = 0;

        if(&(m->nlist[i]) == globalSelectedNode)
        {
            ndx = globalSelectedNodeDeltaX;
            ndy = globalSelectedNodeDeltaY;
            ds = &everythingStat;
        }
        else
        {
            ndx = 0;
            ndy = 0;
            ds = &globalDispStat;
        }

        if (m->nlist[i].cluster->labelClockError)
            nodeLabelRemovePtr(&(m->nlist[i]), m->nlist[i].cluster->labelClockError);
        m->nlist[i].cluster->labelClockError = NULL;

        /* Do not display clock errors when playing goodwin files.  The problem is that goodwinmerge converts to GPS time, but the status messages do not, so clock error is huge  */
        if ((globalGoodwin == NULL) && (m->nlist[i].cluster->datavalid) &&
                (fabs(m->nlist[i].cluster->lasttime -
                      m->nlist[i].cluster->lasttimewatcher) > 1000.0) &&
                (ds->familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_SANITYCHECK)) )
        {
            NodeLabel lab = { { 255, 0, 0, 255 }, { 0, 0, 0, 255 }, NULL, COMMUNICATIONS_LABEL_PRIORITY_CRITICAL, 0, 0, 0, 0, 0 };

            sprintf(buff, "CLOCK ERROR! %d", (int)fabs(m->nlist[i].cluster->lasttime - m->nlist[i].cluster->lasttimewatcher));
            lab.text = buff;
            if (m->nlist[i].cluster->labelClockError)
                nodeLabelRemovePtr(&m->nlist[i], m->nlist[i].cluster->labelClockError);
            m->nlist[i].cluster->labelClockError = nodeLabelApply(&(m->nlist[i]), &lab);
        }


        /* If color is unspecified, then color according to position  */
        if ((m->nlist[i].color == NULL) && (ds->familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_HIERARCHY)))
        {
            if (m->nlist[i].cluster->positionStatus[COORDINATOR_NEIGHBORHOOD] == IDSPOSITION_ACTIVE)
                m->nlist[i].color = (unsigned char*)&globalNeighborColors[5];
            if (m->nlist[i].cluster->positionStatus[COORDINATOR_REGIONAL] == IDSPOSITION_ACTIVE)
                m->nlist[i].color = (unsigned char*)&globalNeighborColors[2];
            if (m->nlist[i].cluster->positionStatus[COORDINATOR_ROOT] == IDSPOSITION_ACTIVE)
                m->nlist[i].color = (unsigned char*)&globalNeighborColors[1];
            m->nlist[i].x += ndx;
            m->nlist[i].y += ndy;
            nodeDrawFn(&(m->nlist[i]), dispType, ds);
            m->nlist[i].y -= ndy;
            m->nlist[i].x -= ndx;
            m->nlist[i].color = NULL;
        }
        else
        {
            m->nlist[i].x += ndx;
            m->nlist[i].y += ndy;
            nodeDrawFn(&(m->nlist[i]), dispType, ds);
            m->nlist[i].y -= ndy;
            m->nlist[i].x -= ndx;
        }

        if ((ds->familyBitmap & (1 << COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH)) && (m->nlist[i].cluster->datavalid))
        {
            x = m->nlist[i].x - 6.0 + ndx;
            y = m->nlist[i].y - 6.0 + ndy;
            z = m->nlist[i].z;

#define MAKEBAR(v, pos, scale) glBegin(GL_POLYGON);\
            glVertex3f(x - (barwidth *pos), y - ((v) * scale), z);\
            glVertex3f(x - (barwidth *(pos - 1)), y - ((v) * scale), z);\
            glVertex3f(x - (barwidth *(pos - 1)), y, z);\
            glVertex3f(x - (barwidth *pos), y, z);\
            glEnd()

            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, xmit);
            MAKEBAR(-(m->nlist[i].cluster->curpersec.origUnicastXmitByte + m->nlist[i].cluster->curpersec.repUnicastXmitByte), 1, bytescale);
            MAKEBAR(-(m->nlist[i].cluster->curpersec.origUnicastXmitCount + m->nlist[i].cluster->curpersec.repUnicastXmitCount), 2, countscale);
            MAKEBAR(-(m->nlist[i].cluster->curpersec.origBcastXmitByte + m->nlist[i].cluster->curpersec.repBcastXmitByte), 3, bytescale);
            MAKEBAR(-(m->nlist[i].cluster->curpersec.origBcastXmitCount + m->nlist[i].cluster->curpersec.repBcastXmitCount), 4, countscale);
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, rec);
            MAKEBAR(m->nlist[i].cluster->curpersec.unicastRecByte, 1, bytescale);
            MAKEBAR(m->nlist[i].cluster->curpersec.unicastRecCount, 2, countscale);
            MAKEBAR(m->nlist[i].cluster->curpersec.bcastRecByte, 3, bytescale);
            MAKEBAR(m->nlist[i].cluster->curpersec.bcastRecCount, 4, countscale);

            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, frame);
            glBegin(GL_LINE_STRIP);
            glVertex3f(x - barwidth *4, y - barheight, z);
            glVertex3f(x, y - barheight, z);
            glVertex3f(x, y + barheight, z);
            glVertex3f(x - barwidth *4, y + barheight, z);
            glVertex3f(x - barwidth *4, y - barheight, z);
            glEnd();
            glBegin(GL_LINES);
            for(j = 1; j < 4; j++)
            {
                glVertex3f(x - barwidth *j, y - barheight, z);
                glVertex3f(x - barwidth *j, y + barheight, z);
            }
            glVertex3f(x - barwidth *4, y, z);
            glVertex3f(x, y, z);
            glEnd();
        }

#undef MAKEBAR
    }

    free(levels);
} // watcherDrawNodes


