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
#include "backgroundImage.h"
#include "skybox.h"
#include "watcherPropertyData.h"

#include "mobility.h"
#include "watcher.h"

#include "libconfig.h++"
#include "singletonConfig.h"
#include "watcherScrollingGraphControl.h"

#include "legacyWatcher.h"
using namespace legacyWatcher;

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
// XXX now in header. static void drawManet(void);
// XXX now in header. static void drawHierarchy(void);
static void watcherDrawNodes(NodeDisplayType dispType, manet *m);
long long int getMilliTime(void);
void gotMessageLabel(void *data, const struct MessageInfo *mi);
void gotMessageWatcherProperty(void *data, const struct MessageInfo *mi);
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
NodeDisplayStatus globalDispStat; // referenced in graphics.cpp, so don't make static
static destime globalLastMetricsTick;
static int globalWatcherMovementEnableFlag = 0;
static void *globalWatcherMovementState;
static NodeEdge *userGraph = NULL;
static FloatingLabel *floatingLabelList = NULL;
static destime curtime = 0, begintime = 0;
static manetNode *globalSelectedNode = 0;

// UNUSED NOW static int globalSelectedNodeScreenX;
// UNUSED NOW static int globalSelectedNodeScreenY;
static double globalSelectedNodeDeltaX;
static double globalSelectedNodeDeltaY;
// GTL not yet. static void scaleNodesZAxis(const float scaleVal); 

static char *globalgoodwinfilename;
static CommunicationsLogStatePtr globalGoodwin;

GPSDataFormat globalGPSDataFormat=legacyWatcher::GPS_DATA_FORMAT_DEG_WGS84;

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
static legacyWatcher::WatcherView globalActiveView = legacyWatcher::ManetView;

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

GlobalManetAdj globalManetAdj;
GlobalManetAdj globalHierarchyAdj;
GlobalManetAdj globalManetAdjInit =     { 0.0, 0.0, 0.0, .035, .035, .03, 0.0, 0.0, 0.0 }; 
GlobalManetAdj globalHierarchyAdjInit = { 0.0, 0.0, 0.0, 1.9, 1.5, 0.0, 0.0, 0.0, 0.0 };

int globalShowPositionFlag = 0;
int globalExitAtEofFlag = 0;

WatcherPropertiesList GlobalWatcherPropertiesList;

void legacyWatcher::layerToggle(const Layer layer, const bool turnOn)
{
    if (turnOn)
        globalDispStat.familyBitmap |= layer;
    else 
        globalDispStat.familyBitmap &= ~layer;
}

NodeDisplayStatus &legacyWatcher::getDisplayStatus(void) 
{
    return globalDispStat;
}

void legacyWatcher::toggleMonochrome(bool isOn)
{
    globalDispStat.monochromeMode=isOn?1:0;
}

void legacyWatcher::setGPSDataFormat(const GPSDataFormat &format)
{
    globalGPSDataFormat=format;
}

GPSDataFormat legacyWatcher::getGPSDataFormat()
{
    return globalGPSDataFormat;
}

void legacyWatcher::toggleThreeDView(bool isOn)
{
    // GTL start of support to raise non-leaf nodes off the Z axis. 
    // Doesn't yet work as edges are only drawn in an x-y plain.
    // float hiearchyZScale=2.0;
    // if (isOn)
    //     scaleNodesZAxis(hiearchyZScale);
    // else
    //     scaleNodesZAxis(1/hiearchyZScale); 

    globalDispStat.threeDView=isOn?1:0;
}

void legacyWatcher::toggleBackgroundImage(bool isOn)
{
   globalDispStat.backgroundImage=isOn?1:0;
}

// UNUSED NOW static void labelsClear(manet *m);

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
        double , // maxRectAspectRatio,
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
    int borderX = viewport[2] < 80 ? 0 : viewport[2] < 580 ? (viewport[2] - 80) / 20 : 70;
    int borderY = viewport[3] < 80 ? 0 : viewport[3] < 580 ? (viewport[3] - 80) / 20 : 70;
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
        GlobalManetAdj *manetAdj = &globalManetAdj;
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
        glScalef(1.0, 1.0, manetAdj->scaleZ); // getting scale x and y so start with unity
        glRotatef(manetAdj->angleX, 1.0, 0.0, 0.0);
        glRotatef(manetAdj->angleY, 0.0, 1.0, 0.0);
        glRotatef(manetAdj->angleZ, 0.0, 0.0, 1.0);
        glTranslatef(0.0, 0.0, manetAdj->shiftZ + 3); // getting shift x and y so start with zero
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
            manetAdj->shiftX = ((wXMin + wXMax) / 2) - ((xMin + xMax) / 2);
            manetAdj->shiftY = ((wYMin + wYMax) / 2) - ((yMin + yMax) / 2);
            manetAdj->scaleX = (wXMax - wXMin) / nodesWidth;
            manetAdj->scaleY = (wYMax - wYMin) / nodesHeight;
            if(manetAdj->scaleX > manetAdj->scaleY)
            {
                manetAdj->scaleX = manetAdj->scaleY;
            }
            else
            {
                manetAdj->scaleY = manetAdj->scaleX;
            }

            BackgroundImage &bgImage=BackgroundImage::getInstance(); 
            if (bgImage.centerImage())
            {
                GLfloat x,y,z,w,h;
                bgImage.getDrawingCoords(x,w,y,h,z);
                bgImage.setDrawingCoords(manetAdj->shiftX, w, manetAdj->shiftY, h, z); 
                bgImage.centerImage(false); 
            }

            //if(now > tick)
            {
                fprintf(stderr, "********************************************************************\n");
                fprintf(stderr, "****************viewport (%g,%g)-(%g,%g)********************\n",
                        wXMin, wYMin, wXMax, wYMax);
                fprintf(stderr, "****************extents  (%g,%g)-(%g,%g)********************\n",
                        xMin, yMin, xMax, yMax);
                fprintf(stderr, "****************scaleXManet=%g****************\n", manetAdj->scaleX);
                fprintf(stderr, "****************shift(%g,%g)****************\n", manetAdj->shiftX, manetAdj->shiftY);
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
                r = m->nlist[i].aradius;

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
                if(nodeXMin < xMin) xMin = nodeXMin;
                if(nodeXMax > xMax) xMax = nodeXMax;
                if(nodeYMin < yMin) yMin = nodeYMin;
                if(nodeYMax > yMax) yMax = nodeYMax;
            }
            if(m->nlist[i].z < zMin)
                zMin = m->nlist[i].z;
        }
        scaleAndShiftToSeeOnManet(xMin, yMin, xMax, yMax, zMin, onChangeOrAlways);
    }
} // scaleAndShiftToCenter

void legacyWatcher::ReshapeManet(int awidth, int aheight)
{
    // GLint viewport[4];
    // glGetIntegerv(GL_VIEWPORT, viewport);
    // fprintf(stderr, "Reshape cur (%d, %d)\n", viewport[2], viewport[3]);
    // fprintf(stderr, "Reshape given (%d, %d)\n", awidth, aheight);
    glViewport(0, 0, awidth, aheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, GLfloat(awidth) / GLfloat(aheight), 1.0, 50.0);
    if(globalAutoCenterNodesFlag && globalManet)
    {
        scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateOnChange);
    }
}

void legacyWatcher::ReshapeHierarchy(int awidth, int aheight)
{
    glViewport(0, 0, awidth, aheight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, GLfloat(awidth) / GLfloat(aheight), 1.0, 50.0);
}

static void getShiftAmount(GLdouble &x_ret, GLdouble &y_ret)
{
    GlobalManetAdj *manetAdj = &globalManetAdj;
    GLdouble z;
    int i;
    GLdouble modelmatrix[16];
    GLdouble projmatrix[16];
    GLint viewport[4];
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);
    glScalef(manetAdj->scaleX, manetAdj->scaleY, manetAdj->scaleZ);
    glRotatef(manetAdj->angleX, 1.0, 0.0, 0.0);
    glRotatef(manetAdj->angleY, 0.0, 1.0, 0.0);
    glRotatef(manetAdj->angleZ, 0.0, 0.0, 1.0);
    glTranslatef(manetAdj->shiftX, manetAdj->shiftY, manetAdj->shiftZ + 3);
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
        if(xyAtZForModelProjViewXY( xyz, sizeof(xyz) / sizeof(xyz[0]), modelmatrix, projmatrix, viewport) ==  0)
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

void legacyWatcher::jumpToX(int x)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftX = x;
    else
        globalHierarchyAdj.shiftX = x;
    globalAutoCenterNodesFlag = 0;
}

void legacyWatcher::jumpToY(int y)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftY = y;
    else
        globalHierarchyAdj.shiftY = y;
    globalAutoCenterNodesFlag = 0;
}

void legacyWatcher::jumpToXY(int x, int y)
{
    jumpToX(x);
    jumpToY(y);
}

void legacyWatcher::shiftBackgroundCenterLeft(double dx)
{
    if (globalActiveView==legacyWatcher::ManetView)
    {
        GLfloat x, y, w, h, z;
        BackgroundImage &bg=BackgroundImage::getInstance();
        bg.getDrawingCoords(x, w, y, h, z);
        bg.setDrawingCoords(x+dx, w, y, h, z);
    } 
}

void legacyWatcher::shiftBackgroundCenterUp(double dy)
{
    if (globalActiveView==legacyWatcher::ManetView)
    {
        GLfloat x, y, w, h, z;
        BackgroundImage &bg=BackgroundImage::getInstance();
        bg.getDrawingCoords(x, w, y, h, z);
        bg.setDrawingCoords(x, w, y+dy, h, z);
    } 
}

void legacyWatcher::shiftCenterRight()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterRight(shift);
}
void legacyWatcher::shiftCenterRight(double shift)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftX -= shift;
    else
        globalHierarchyAdj.shiftX -= shift;

    globalAutoCenterNodesFlag=0;
} 

void legacyWatcher::shiftCenterLeft()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterLeft(shift);
}
void legacyWatcher::shiftCenterLeft(double shift)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftX += shift;
    else
        globalHierarchyAdj.shiftX += shift;

    globalAutoCenterNodesFlag=0;
} 

void legacyWatcher::shiftCenterDown()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterDown(shift);
}
void legacyWatcher::shiftCenterDown(double shift)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftY += shift;
    else
        globalHierarchyAdj.shiftY += shift;

    globalAutoCenterNodesFlag=0;
} 

void legacyWatcher::shiftCenterUp()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterUp(shift);
}
void legacyWatcher::shiftCenterUp(double shift)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftY -= shift;
    else
        globalHierarchyAdj.shiftY -= shift;

    globalAutoCenterNodesFlag=0;
} 

void legacyWatcher::shiftCenterIn()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterIn(shift);
}
void legacyWatcher::shiftCenterIn(double shift)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftZ -= shift;
    else
        globalHierarchyAdj.shiftZ -= shift;

    globalAutoCenterNodesFlag=0;
} 

void legacyWatcher::shiftCenterOut()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterOut(shift);
}
void legacyWatcher::shiftCenterOut(double shift)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.shiftZ += shift;
    else
        globalHierarchyAdj.shiftZ += shift;

    globalAutoCenterNodesFlag=0;
} 

void legacyWatcher::viewpointReset(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj = globalManetAdjInit;
    else
        globalHierarchyAdj = globalHierarchyAdjInit;
}

void legacyWatcher::zoomOut()
{
    if (globalActiveView==legacyWatcher::ManetView)
    {
        globalManetAdj.scaleX /= 1.05;
        if (globalManetAdj.scaleX < 0.001) 
            globalManetAdj.scaleX = 0.001;
        globalManetAdj.scaleY = globalManetAdj.scaleX;
        globalAutoCenterNodesFlag = 0;
    }
    else
    {
        globalHierarchyAdj.scaleX -= 0.03;
        if (globalHierarchyAdj.scaleX < 0.01) 
            globalHierarchyAdj.scaleX = 0.01;
        globalHierarchyAdj.scaleY = globalHierarchyAdj.scaleX;
    }
}

void legacyWatcher::zoomIn()
{
    if (globalActiveView==legacyWatcher::ManetView)
    {
        globalManetAdj.scaleX *= 1.05;
        globalManetAdj.scaleY = globalManetAdj.scaleX;
        globalAutoCenterNodesFlag = 0;
    }
    else
    {
        globalHierarchyAdj.scaleX += 0.03;
        globalHierarchyAdj.scaleY = globalHierarchyAdj.scaleX;
    }
}

void legacyWatcher::setActiveView(const WatcherView &view)
{
    globalActiveView=view;
}

void legacyWatcher::compressDistance()
{
    if (globalActiveView==legacyWatcher::ManetView)
    {
        globalManetAdj.scaleZ -= 0.1;
        if (globalManetAdj.scaleZ < 0.02)
        {
            globalManetAdj.scaleZ = 0.02;
        }
        if(globalAutoCenterNodesFlag && globalManet)
        {
            scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
        }
    }
    else
    {
        globalHierarchyAdj.scaleZ -= 0.1;
        if (globalHierarchyAdj.scaleZ < 0.02)
        {
            globalHierarchyAdj.scaleZ = 0.02;
        }
    }
} // compressDistance

void legacyWatcher::expandDistance()
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalManetAdj.scaleZ += 0.1;
    else
        globalHierarchyAdj.scaleZ += 0.1;
} // compressDistance

#define TEXT_SCALE 0.08
#define TEXT_SCALE_ZOOM_FACTOR 1.05
#define ARROW_SCALE_ZOOM_FACTOR 1.05

void legacyWatcher::textZoomReset(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalDispStat.scaleText[NODE_DISPLAY_MANET] = 0.08;
    else
        globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] = 0.08;
}

void legacyWatcher::textZoomIn(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalDispStat.scaleText[NODE_DISPLAY_MANET] *= TEXT_SCALE_ZOOM_FACTOR;
    else
        globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] *= TEXT_SCALE_ZOOM_FACTOR;
}

void legacyWatcher::textZoomOut(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalDispStat.scaleText[NODE_DISPLAY_MANET] /= TEXT_SCALE_ZOOM_FACTOR;
    else
        globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] /= TEXT_SCALE_ZOOM_FACTOR;
}

void legacyWatcher::arrowZoomReset(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalDispStat.scaleLine[NODE_DISPLAY_MANET] = 1.0;
    else
        globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] = 1.0;
}

void legacyWatcher::arrowZoomIn(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalDispStat.scaleLine[NODE_DISPLAY_MANET] *= ARROW_SCALE_ZOOM_FACTOR;
    else
        globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] *= ARROW_SCALE_ZOOM_FACTOR;
}

void legacyWatcher::arrowZoomOut(void)
{
    if (globalActiveView==legacyWatcher::ManetView)
        globalDispStat.scaleLine[NODE_DISPLAY_MANET] /= ARROW_SCALE_ZOOM_FACTOR;
    else
        globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] /= ARROW_SCALE_ZOOM_FACTOR;
}

void legacyWatcher::rotateX(float deg)
{
    if (globalActiveView==legacyWatcher::ManetView)
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
    else
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

void legacyWatcher::rotateY(float deg)
{
    if (globalActiveView==legacyWatcher::ManetView)
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
    else
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

void legacyWatcher::rotateZ(float deg)
{
    if (globalActiveView==legacyWatcher::ManetView)
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
    else
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

int legacyWatcher::Key(unsigned char key)
{
    int handled=0;
    char keystr[2] = { 0, 0 };
    fprintf(stderr, "%s: got key %s\n", __func__, isprint(key) ? (keystr[0] = key, keystr) : "not printable");

    // fprintf(stderr, "%s: %s%s%s%s (0x%x)\n", __func__,
    //         mod & GLUT_ACTIVE_SHIFT ? "<shift> " : "", 
    //         mod & GLUT_ACTIVE_CTRL ? "<ctrl> " : "",
    //         mod & GLUT_ACTIVE_ALT ? "<alt> " : "",
    //         isprint(key) ? (keystr[0] = key, keystr) : keystr, 
    //         key);

    switch (key) 
    {
        case 27:
            exit(1);

        case 'n':
            shiftCenterIn();
            handled=1;
            break;
        case 'm':
            shiftCenterOut();
            handled=1;
            break;
        case 'q':
            zoomOut();
            handled=1;
            break;
        case 'w':
            zoomIn();
            handled=1;
            break;

        case 'a':
            textZoomOut();
            handled=1;
            break;
        case 's':
            textZoomIn();
            handled=1;
            break;

        case 'a' - 'a' + 1:
            arrowZoomOut();
            handled=1;
            break;
        case 's' - 'a' + 1:
            arrowZoomIn();
            handled=1;
            break;

        case 'z':
            compressDistance();
            handled=1;
            break;
        case 'x':
            expandDistance();
            handled=1;
            break;

        case 'e':
            rotateX(-5.0);
            handled=1;
            break;
        case 'r':
            rotateX(5.0);
            handled=1;
            break;
        case 'd':
            rotateY(-5.0);
            handled=1;
            break;
        case 'f':
            rotateY(5.0);
            handled=1;
            break;
        case 'c':
            rotateZ(-5.0);
            handled=1;
            break;
        case 'v':
            rotateZ(5.0);
            handled=1;
            break;

        case 'b':
        case 'B':
            globalDispStat.familyBitmap ^= (1 << COMMUNICATIONS_LABEL_FAMILY_BANDWIDTH);
            handled=1;
            break;

        case 'r' - 'a' + 1:
            textZoomReset();
            arrowZoomReset();
            viewpointReset();
            handled=1;
            break;

        case '=':
        case '+':
            globalAutoCenterNodesFlag = 1; 
            if(globalManet)
            {
                scaleAndShiftToCenter(globalManet, ScaleAndShiftUpdateAlways);
            }
            handled=1;
            globalAutoCenterNodesFlag = 0; 
            break;

        case ' ':
            globalReplay.runFlag = !globalReplay.runFlag;
            if (globalReplay.runFlag)
            {
                globalReplay.runstartfile = curtime;
                globalReplay.runstartwall = getMilliTime();
            }
            handled=1;
            break;
        case 't':
            globalReplay.step = 1000;
            handled=1;
            break;

        case 'h':
        case 'H':
        case '?':
            handled=0;
            break;
        default:
            // Use to print to stdout, now does GL overlay in class that calls this function
            handled=0;
            break;
    }
    return handled;
}

// UNUSED NOW static void Mouse(int button, int updown, int mousex, int mousey)
// UNUSED NOW {
// UNUSED NOW     if(globalManet && 
// UNUSED NOW             globalManet->numnodes && 
// UNUSED NOW             button == GLUT_LEFT_BUTTON && 
// UNUSED NOW             updown == GLUT_DOWN)
// UNUSED NOW     {
// UNUSED NOW         unsigned int dist;
// UNUSED NOW         manetNode *n = closestNode(
// UNUSED NOW                 globalManet, mousex, mousey, (unsigned int)(4.0 * globalManetAdj.scaleX / 0.02), &dist);
// UNUSED NOW         if(n)
// UNUSED NOW         {
// UNUSED NOW             if(dist < 5)
// UNUSED NOW             {
// UNUSED NOW                 globalSelectedNode = n;
// UNUSED NOW                 globalSelectedNodeScreenX = mousex;
// UNUSED NOW                 globalSelectedNodeScreenY = mousey;
// UNUSED NOW                 globalSelectedNodeDeltaX = 0;
// UNUSED NOW                 globalSelectedNodeDeltaY = 0;
// UNUSED NOW             }
// UNUSED NOW         }
// UNUSED NOW     }
// UNUSED NOW     else if(updown == GLUT_UP)
// UNUSED NOW     {
// UNUSED NOW         globalSelectedNode = 0;
// UNUSED NOW     }
// UNUSED NOW     return;
// UNUSED NOW }

// UNUSED NOW static void Motion(int mousex, int mousey)
// UNUSED NOW {
// UNUSED NOW     if(globalSelectedNode)
// UNUSED NOW     {
// UNUSED NOW         GLdouble modelmatrix[16];
// UNUSED NOW         GLdouble projmatrix[16];
// UNUSED NOW         GLint viewport[4];
// UNUSED NOW         glPushMatrix();
// UNUSED NOW         glMatrixMode(GL_MODELVIEW);
// UNUSED NOW         glLoadIdentity();
// UNUSED NOW         glTranslatef(0.0, 0.0, -20.0);
// UNUSED NOW         glScalef(globalManetAdj.scaleX, globalManetAdj.scaleY, globalManetAdj.scaleZ);
// UNUSED NOW         glRotatef(globalManetAdj.angleX, 1.0, 0.0, 0.0);
// UNUSED NOW         glRotatef(globalManetAdj.angleY, 0.0, 1.0, 0.0);
// UNUSED NOW         glRotatef(globalManetAdj.angleZ, 0.0, 0.0, 1.0);
// UNUSED NOW         glTranslatef(globalManetAdj.shiftX, globalManetAdj.shiftY, globalManetAdj.shiftZ + 3);
// UNUSED NOW         glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
// UNUSED NOW         glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
// UNUSED NOW         glGetIntegerv(GL_VIEWPORT, viewport);
// UNUSED NOW         glPopMatrix();
// UNUSED NOW         {
// UNUSED NOW             XYWorldZToWorldXWorldY xyz =
// UNUSED NOW             {
// UNUSED NOW                 mousex, viewport[3] - mousey, globalSelectedNode->z, 0, 0
// UNUSED NOW             };
// UNUSED NOW             if(xyAtZForModelProjViewXY(
// UNUSED NOW                         &xyz, 1, modelmatrix, projmatrix, viewport) == 0)
// UNUSED NOW             {
// UNUSED NOW                 globalSelectedNodeDeltaX = 
// UNUSED NOW                     globalSelectedNode->x - xyz.worldX_ret;
// UNUSED NOW                 globalSelectedNodeDeltaY = 
// UNUSED NOW                     globalSelectedNode->y - xyz.worldY_ret;
// UNUSED NOW             }
// UNUSED NOW         }
// UNUSED NOW     }
// UNUSED NOW     return;
// UNUSED NOW }

static GLenum Args(int , char **)
{
    return GL_TRUE;
}

// GTL - not used.
// static void crossProduct(double *c, double a[3], double b[3])
// {  
//     double len;
//     c[0] = (a[1] * b[2]) - (b[1] * a[2]);
//     c[1] = (a[2] * b[0]) - (b[2] * a[0]);
//     c[2] = (a[0] * b[1]) - (b[0] * a[1]);
// 
//     len = sqrt((c[0] * c[0]) + (c[1] * c[1]) + (c[2] * c[2]));
//     c[0] /= len;
//     c[1] /= len;
//     c[2] /= len;
// }

// static void drawManet(void)
void legacyWatcher::drawManet(void)
{
    static const GLfloat blue[] = { 0.2f, 0.2f, 1.0f, 1.0f };
    // static const GLfloat blue2[] = { 0.8f, 0.8f, 1.0f, 1.0f };
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

    // glPushMatrix();
    // glTranslatef(0.0, 0.0, -50.0);
    // static const GLfloat light_pos[] = { 0.0, 0.0, 0.0, 1.0 };
    // glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    // glPopMatrix();

    // watcher::Skybox *sb=watcher::Skybox::getSkybox();
    // if (sb)
    //     sb->drawSkybox(globalManetAdj.angleX, globalManetAdj.angleY, globalManetAdj.angleZ);

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

    drawText(-400, -360, 0, globalDispStat.scaleText[NODE_DISPLAY_MANET], buff, 1.0);

    if (globalShowPositionFlag)
    {
        snprintf(buff, sizeof(buff),
                "Location: %3.1f, %3.1f, %3.1f scale %f layers: 0x%x  time: %ld @%ld",
                globalManetAdj.shiftX, globalManetAdj.shiftY, globalManetAdj.shiftZ, 
                globalManetAdj.scaleX,
                globalDispStat.familyBitmap,
                (long)( curtime / 1000), (long)((curtime - begintime)) / 1000);
        drawText(-200, -360, 0, globalDispStat.scaleText[NODE_DISPLAY_MANET], buff, 1.0);
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

        glPushAttrib(GL_ALL_ATTRIB_BITS);
            // glColor4f(1.0, 0.0, 0.0, 0.5);
            drawGraph(m, phy, globalDispStat.scaleText[NODE_DISPLAY_MANET], 1);
        glPopAttrib();
    
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
    glTranslatef(0.0, 0.0, + 0.8);
    watcherGraphDraw(&userGraph, NODE_DISPLAY_MANET, &globalDispStat, m->curtime);
    floatingLabelDraw(&floatingLabelList, NODE_DISPLAY_MANET, &globalDispStat, m->curtime);
    glPopMatrix();

    if (globalDispStat.backgroundImage)
    {
        BackgroundImage::getInstance().drawImage(); 
    }

    glFlush();
}

void legacyWatcher::drawHierarchy(void)
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
    glTranslatef(-350.0, 450.0, 75.0);

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
}

/**************************************************************************************
 *
 * These functions are the watcher's implementation of a clustering algorithm.
 *
 */
/* walk the arrays of label pointers, and remove all of them
*/

void legacyWatcher::clearAllLabels()
{
    int nod;
    manetNode *us;
    manet *m=globalManet;

    for(nod = 0; nod < m->numnodes; nod++)
    {
        us = &(m->nlist[nod]);
        nodeLabelRemoveAll(us);
        us->cluster->needupdate = 1;
    }
}

void legacyWatcher::clearAllEdges()
{
    watcherGraphEdgeNuke(&userGraph);
}

/* This is called in either goodwin or live mode, to init our
 * data structures
 */
void nodeOpenSucceed(manetNode *us)
{
    int i;

    // Look for and replace control-addr based watherproperty with MANET-addr based one. 
    // HACK and not a good one.
    fprintf(stderr, "-----------node open success. old us->addr %d.%d.%d.%d\n", PRINTADDR(us->addr));
    WatcherPropertiesList::iterator propIndex=GlobalWatcherPropertiesList.begin();
    for( ; propIndex!=GlobalWatcherPropertiesList.end();propIndex++)
        if((*propIndex)->identifier==us->addr) 
        {
            LOG_DEBUG("Replacing control network address watcher property identifier for this node with a MANET addr based one"); 
            (*propIndex)->identifier=communicationsNodeAddress(us->cluster->cs);
        }

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
    messageHandlerSet(us->cluster->cs, COMMUNICATIONS_MESSAGE_INBOUND, COMMUNICATIONS_MESSAGE_AFTERALL, COMMUNICATIONS_MESSAGE_READONLY, IDSCOMMUNICATIONS_MESSAGE_WATCHER_PROPERTY, gotMessageWatcherProperty, us);
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

void nodeGotPacket(manetNode *, packet *)
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
    int msgnum = *((int*)messageStatusHandlerData);
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

void gotMessageWatcherProperty(void *data, const struct MessageInfo *mi)
{
    manetNode *us = (manetNode*)data;

    WatcherPropertyInfo messageProp;
    unsigned char *pos=(unsigned char *)messageInfoRawPayloadGet(mi);
    pos = communicationsWatcherPropertyUnmarshal(pos, &messageProp);

    LOG_DEBUG("Got WatcherProperty message for node index " << index); 

    // If a color message, use existing color field in maneNode and return.
    if (messageProp.property==WATCHER_PROPERTY_COLOR) // just copy the color over onto the manetNode's color field.
    {
        int index=manetGetNodeNum(us->manet, messageProp.identifier);
        if (index==-1)
        {
            LOG_WARN("Got watcher property color message, but am unable to find the node with address: " << messageProp.identifier); 
            return;
        }

        nodeColor(&(us->manet->nlist[index]), us->manet->nlist[index].cluster->color);
        for (int i=0;i<4;i++)
            us->manet->nlist[index].cluster->color[i]=messageProp.data.color[i];
        us->cluster->needupdate = 1;
        return;
    }

    // Not a color: stuff the property into the watcher properties list.
    WatcherPropertyData *propp;
    WatcherPropertiesList::iterator propIndex=GlobalWatcherPropertiesList.begin();
    for( ; propIndex!=GlobalWatcherPropertiesList.end();propIndex++)
        if((*propIndex)->identifier==messageProp.identifier) 
            break;

    if (propIndex==GlobalWatcherPropertiesList.end())
    {
        LOG_ERROR("Got property message for node we know nothing about. Got: " << PRINTADDRCPP(messageProp.identifier)); 
    }
    else
    {
        LOG_DEBUG("Modifing existing properties for node " << messageProp.identifier); 
        propp=*propIndex;

        // If the property is in the incoming message, set it. If it's an effect, inverse the current state (off->on, on->off)
        switch (messageProp.property)
        {
            case WATCHER_PROPERTY_COLOR:
                // handled above. Will not get here. 
                break;
            case WATCHER_PROPERTY_SHAPE: 
                propp->shape=messageProp.data.shape; 
                break;
            case WATCHER_PROPERTY_SIZE: 
                propp->size=messageProp.data.size; 
                break;
            case WATCHER_PROPERTY_EFFECT:
                switch (messageProp.data.effect)
                {
                    case WATCHER_EFFECT_SPARKLE: 
                        propp->sparkle=propp->sparkle?0:1;
                        break;
                    case WATCHER_EFFECT_SPIN: 
                        propp->spin=propp->spin?0:1;
                        if (!propp->spin) 
                        {
                            propp->spinRotation_x=0;
                            propp->spinRotation_y=0;
                            propp->spinRotation_z=0;
                        }
                        break;
                    case WATCHER_EFFECT_FLASH: 
                        propp->flash=propp->flash?0:1;
                        propp->isFlashed=0;
                        break;
                }
                break;
        }
    }
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

    if (globalGPSDataFormat == GPS_DATA_FORMAT_UTM)
    {
        //
        // There is no UTM zone information in the UTM GPS packet, so we assume all data is in a single
        // zone. Because of this, no attempt is made to place the nodes in the correct positions on the 
        // planet surface. We just use the "lat" "long" data as pure x and y coords in a plane, offset
        // by the first coord we get. (Nodes are all centered around 0,0 where, 0,0 is defined 
        // by the first coord we receive. 
        //
        if (location->lon < 91 && location->lon > 0) 
            LOG_WARN("Received GPS data that looks like lat/long in degrees, but GPS data format mode is set to UTM in cfg file."); 

        static double utmXOffset=0.0, utmYOffset=0.0;
        static bool utmOffInit=false;
        if (utmOffInit==false)
        {
            utmOffInit=true;
            utmXOffset=location->lon;
            utmYOffset=location->lat;

            LOG_INFO("Got first UTM coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" << utmXOffset << " y=" << utmYOffset);
        }

        us->x=location->lon-utmXOffset;
        us->y=location->lat-utmYOffset;    
        us->z=location->alt;

        LOG_DEBUG("UTM given locations: lon=" << location->lon << " lat=" << location->lat << " alt=" << location->alt);
        LOG_DEBUG("UTM node coords: x=" << us->x << " y=" << us->y << " z=" << us->z);
    }
    else // default to lat/long/alt WGS84
    {
        if (location->lon > 180)
            LOG_WARN("Received GPS data that may be UTM (long>180), but GPS data format mode is set to lat/long degrees in cfg file."); 

        us->x = location->lon * GPSScale;
        us->y = location->lat * GPSScale;
        us->z = location->alt - 20;

        static double xOff=0.0, yOff=0.0;
        static bool xOffInit=false;
        if (xOffInit==false)
        {
            xOffInit=true;
            xOff=us->x;
            yOff=us->y;

            LOG_INFO("Got first Lat/Long coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" 
                    << xOff << " y=" << yOff);
        }

        us->x-=xOff;
        us->y-=yOff;

        LOG_DEBUG("Got GPS: long:" << location->lon << " lat:" << location->lat << " alt:" << location->alt); 
        LOG_DEBUG("translated GPS: x:" << us->x << " y:" << us->y << " z:" << us->z); 

    }

    globalGpsValidFlag[us->index] = 1;

    tim = location->time;      /* GPS time in milliseconds (idsCommunication's canonical unit)  */

    LOG_DEBUG("got GPS Info: " << us->index << "long:" << us->x << " lat:" << us->y << " alt:" << us->z); 

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

    // graphUnmarshal(globalGraphManet, us->manet->numnodes, payload);
    watcher::WatcherScrollingGraphControl *graphData=watcher::WatcherScrollingGraphControl::getWatcherScrollingGraphControl();
    graphData->unmarshalWatcherGraphMessage(us->addr, payload); 
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

        // fprintf(stderr, "got an edge, %d.%d.%d.%d (%d)  to %d.%d.%d.%d (%d)  length %f\n", PRINTADDR(edge.a), a, PRINTADDR(edge.b), b, edge.value);

        if ((a >= 0) && (b >= 0))
            globalGraphManet[a + b * us->manet->numnodes] = edge.value * globalGraphScaleManet;
    }
}


/* This is called when a neighbor arrives or departs.  
 * I'm using it as a signal to redraw the display.
 */
static void detectorNeighborUpdate(void *data, CommunicationsNeighbor *)
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

int legacyWatcher::isRunningInPlaybackMode()
{
    return globalGoodwin == NULL ? false : true;
}

GlobalManetAdj &legacyWatcher::getManetAdj()
{
    return globalManetAdj;
}

#define GETMAXFD(mfd, nfd) do { (mfd) = ((nfd) > (mfd)) ? (nfd) : (mfd); } while(0)

/* When we are in playback mode, this will call comunicationsLogStep,
 * Otherwise it calls communicationsReadReady.
 */

int legacyWatcher::checkIOGoodwin(int)
{
    manet *m = globalManet;
    int guiUpdate=0;

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
                nevents = communicationsLogStep(globalGoodwin, 1, &m->curtime);
                if (nevents < 0)
                    if (globalExitAtEofFlag)
                        exit(0);        // Shut it down - shut it all down!
                    else    
                        return 1;       // Just return. GUI still refreshes, but no more new data/events happen.

                if (nevents > 0 && globalDoMetricsFlag)
                {
                    nodeMobilityCountEdges(&m->nlist[0], 0);
                    nodeHierarchyCountEdges(&m->nlist[0], 0);

                    if ((m->curtime - globalLastMetricsTick) > 1000)
                    {
                        update(m);
                        nodeMetrics(&(m->nlist[0]), 0);
                        globalLastMetricsTick = m->curtime;
                        guiUpdate=1;
                    }
                }
            }
            update(m);
            if ((globalReplay.stoptime > 0) && (globalManet->curtime > globalReplay.stoptime))
                exit(0);
        }
        for(i = 0; i < m->numnodes; i++)
            m->nlist[i].cluster->needupdate = 0;
    }
    curtime = communicationsLogTimeGet(globalGoodwin);
    globalReplay.step = - 1;

    return guiUpdate;
}

int legacyWatcher::checkIOLive(int)
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

    return needupdate;
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

void legacyWatcher::setBackgroundColor(float r, float g, float b, float a)
{
    // Set the default clear color
    // Seems to control the background "diffuse" color.
    glClearColor(r,g,b,a);
}

void legacyWatcher::getBackgroundColor(float &r, float &g, float &b, float &a)
{
    GLfloat cols[4]={0.0, 0.0, 0.0, 0.0}; 
    glGetFloatv(GL_COLOR_CLEAR_VALUE, cols);
    r=cols[0];
    g=cols[1];
    b=cols[2];
    a=cols[3];
}

void legacyWatcher::initWatcherGL()
{
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // // glEnable(GL_LINE_SMOOTH);
    // // glEnable(GL_CULL_FACE); 
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glShadeModel(GL_SMOOTH);

    // glEnable(GL_LIGHTING);

    // // GLfloat ambLight[] = { 0.25, 0.25, 0.25, 1.0 };    // Ambient light is not full strength, but white
    // // glLightfv(GL_LIGHT1, GL_AMBIENT, ambLight);
    // // glLightfv(GL_LIGHT1, GL_SPECULAR, ambLight);

    // // GLfloat diffLight[]= { 1.0f, 1.0f, 1.0f, 1.0f };    // diffuse light is full strength
    // // glLightfv(GL_LIGHT1, GL_DIFFUSE, diffLight);

    // // GLfloat posLight[]= { 200.0f, 200.0f, 200.0f, 1.0f }; // light is over right shoulder
    // // glLightfv(GL_LIGHT1, GL_POSITION, posLight);

    // // glEnable(GL_LIGHT1);

    // enable lighting
    glEnable(GL_LIGHTING); 

    // enable objects behind one another
    glEnable(GL_DEPTH_TEST);

    // Allow colors to bleed through by alpha (orange edges when nieghor and one hop routing draw the same link).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the shading model
    glShadeModel(GL_SMOOTH); 

    // Uncomment these if you want to use glColor(c) instead of 
    // glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c);
    // glEnable(GL_COLOR_MATERIAL);
    // glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Spec is LIGHT0
    // GLfloat specular[]={0.5, 0.5, 0.5, 1.0};
    // GLfloat posSpecLight[]= { -500.0f, 500.0f, 100.0f, 1.0f };
    // glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    // glLightfv(GL_LIGHT0, GL_POSITION, posSpecLight);
    // glEnable(GL_LIGHT0); 

    // Amb is LIGHT1
    GLfloat ambLight[] = { 0.25, 0.25, 0.25, 1.0 };
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambLight);
    glEnable(GL_LIGHT1);
    
    // or use the ambient light model
    // GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    
    // Diffuse is LIGHT2
    GLfloat diffLight[]= { 0.5, 0.5, 0.5, 1.0f };    
    glLightfv(GL_LIGHT2, GL_DIFFUSE, diffLight);
    GLfloat posDiffLight[]= { 500.0, 500.0f, 100.0f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, posDiffLight); 
    glEnable(GL_LIGHT2);
}

static void manetWindowInit(void)
{
    if (globalWatcherMovementEnableFlag)
        globalWatcherMovementState = watcherMovementInit(globalManet);

    globalManetAdj = globalManetAdjInit;

    return;
} 

static void hierarchyWindowInit()
{
    globalHierarchyAdj = globalHierarchyAdjInit;
    return;
} 

static void loadGoodwinFile()
{
    if (globalGoodwin)
        communicationsLogClose(globalGoodwin);

    if (globalgoodwinfilename)
    {
        int goodwinfd;
        printf("Opening goodwin file %s\n", globalgoodwinfilename); 
        goodwinfd = open(globalgoodwinfilename, O_RDONLY);
        if (goodwinfd < 0)
        {
            fprintf(stderr, "could not open goodwin file!\n");
            exit(1);
        }
        globalGoodwin = communicationsLogLoad(goodwinfd);
    }
    else
        globalGoodwin = NULL;
}

void legacyWatcher::pauseGoodwin()
{
    globalReplay.runFlag=0;
    globalReplay.runstartfile = curtime;
    globalReplay.runstartwall = getMilliTime();
}
void legacyWatcher::continueGoodwin()
{
    globalReplay.runFlag=1;
    globalReplay.runstartfile = curtime;
    globalReplay.runstartwall = getMilliTime();
}
void legacyWatcher::stopGoodwin()
{
    pauseGoodwin();
}
void legacyWatcher::startGoodwin()
{
    // GTL - This doesn't work, don't use this until it works.
    loadGoodwinFile();
    continueGoodwin(); 
}
void legacyWatcher::setGoodwinPlaybackSpeed(int val)
{
    // val varies 0 - 8.
    long speed=0x10000000|(1<<val);     // format for speed gotten from watcher.cpp
    globalReplay.speed = speed&0x0FFFF;
    if (globalReplay.runFlag)			/* If speed is changed, reset starttime, so we don't jump  */
    {
        globalReplay.runstartfile = curtime;
        globalReplay.runstartwall = getMilliTime();
    }
}
bool legacyWatcher::runningGoodwin()
{
    return globalGoodwin!=NULL;
}

int legacyWatcher::legacyWatcherMain(int argc, char **argv)
{
    Config *conf;
    int i, ch;
    destime starttime = 0;
    int relativestart = 0;
    int relativestop = 0;
    int startrunning = 0;
    int winx = 0, winy = 0, winwidth = 500, winheight = 500, winpos = 0;

    globalDispStat.minPriority = COMMUNICATIONS_LABEL_PRIORITY_INFO;
    // globalDispStat.familyBitmap = COMMUNICATIONS_LABEL_FAMILY_ALL;
    globalDispStat.familyBitmap = 0;
    globalDispStat.scaleText[NODE_DISPLAY_MANET] = 0.08;
    globalDispStat.scaleText[NODE_DISPLAY_HIERARCHY] = 0.08;
    globalDispStat.scaleLine[NODE_DISPLAY_MANET] = 1.0;
    globalDispStat.scaleLine[NODE_DISPLAY_HIERARCHY] = 1.0;
    globalDispStat.monochromeMode = 0;
    globalDispStat.threeDView = 1;
    globalDispStat.backgroundImage = 1;

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

    GPSScale = configSetDouble(conf, "watcher_gpsscale", 80000);

    globalgoodwinfilename = argc > 1 ? strdup(argv[1]) : NULL;
    loadGoodwinFile(); 

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

    // Create properties for all known nodes. 
    for (i=0;i<globalManet->numnodes;i++)
    {
        LOG_DEBUG("Creating new properties for node " << (0x000000FF & (globalManet->nlist[i].addr>>8)) << "." << (0x000000FF & globalManet->nlist[i].addr));
        WatcherPropertyData *propp=new WatcherPropertyData;  // I don't know when this data gets deleted.  Is there a watcher cleanup function that gets called as shutdown?
        GlobalWatcherPropertiesList.push_back(propp); 

        propp->identifier=globalManet->nlist[i].addr;           // indexed by node address.
        watcher::loadWatcherPropertyData(propp, i+1);      // loadProperties() uses identifier to load properties from the watcher.cfg file. 
    }
    // set default background image size (with a little padding WRT to the "playing field"
    {
        struct mobilityManetState *m=globalManet->mobility;
        BackgroundImage &bg=BackgroundImage::getInstance();
        bg.setDrawingCoords(-(m->maxx/3), m->maxx+(m->maxx/3), -(m->maxy/3), m->maxy+(m->maxy/3), -25);
    }

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
    // We still use GLUT to draw text
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

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

    return 0;
}

void packetSend(manetNode *, packet *, int)
{
    fprintf(stderr, "This is the watcher, this shouldn't be called\n");
    abort();
}

// GTL - start of support to raise non-leaf nodes off the z axis. 
//
// static void scaleNodesZAxis(const float scaleVal)
// {
//     manet *m=globalManet;
//     for(int i = 0; i < m->numnodes; i++)
//     {
//         manetNode *us=&(m->nlist[i]); 
//         us->z *= scaleVal*us->level;
//     }
// }

int legacyWatcher::doIdle()
{
    int refresh=0;

    // Update all spin animations if needed.
    WatcherPropertiesList::iterator pp;
    destime curtime=globalGoodwin ? globalManet->curtime : getMilliTime(); 
    for (pp=GlobalWatcherPropertiesList.begin(); pp!=GlobalWatcherPropertiesList.end(); pp++)
    {
        if ((*pp)->spin && curtime > (*pp)->nextSpinUpdate)  // Are we spinning and do we need to update the rotation? 
        {
            // UPdate all rotations - even if we're in non 3d mode.
            (*pp)->spinRotation_x+=WatcherPropertyData::spinIncrement;
            (*pp)->spinRotation_y+=WatcherPropertyData::spinIncrement;
            (*pp)->spinRotation_z+=WatcherPropertyData::spinIncrement;
            (*pp)->nextSpinUpdate=curtime+WatcherPropertyData::spinTimeout;

            // LOG_DEBUG("Updated spin rotation for node " << (*pp)->identifier); 
            refresh=1;
        }
        if ((*pp)->flash && curtime > (*pp)->nextFlashUpdate)  // Are we flashing and do we need to invert the color?
        {
            (*pp)->isFlashed=(*pp)->isFlashed?0:1;
            (*pp)->nextFlashUpdate=curtime+WatcherPropertyData::flashInterval;
            refresh=1;
        }

    }

    return refresh;
}

unsigned int legacyWatcher::getNodeIdAtCoords(const int x, const int y)
{
    TRACE_ENTER();
    unsigned int dist_ret;
    unsigned int retVal;
    manetNode *node = closestNode(globalManet, x, y, 10, &dist_ret);
    if (node)
        retVal=node->addr;
    else 
        retVal=0;
    TRACE_EXIT_RET(retVal);
    return retVal;
}

int legacyWatcher::getNodeStatus(const int x, const int y, char *nodeStatusBuf, size_t bufSize)
{
    unsigned int dist_ret;
    manetNode *node = closestNode(globalManet, x, y, 10, &dist_ret);
    if (node)
    {
        WatcherPropertyData *propertyData=findWatcherPropertyData(node->addr, GlobalWatcherPropertiesList);
        if (propertyData) 
            propertyData->flash=!propertyData->flash;

        snprintf(nodeStatusBuf, bufSize, "Node address: %d.%d.%d.%d\nGPS Coords: %f, %f, %f", PRINTADDR(node->addr), node->x, node->y, node->z); 
        return 1; 
    }
    return 0;
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
        void (*nodeDrawFn)(
                manetNode *, 
                NodeDisplayType, 
                NodeDisplayStatus const *,
                WatcherPropertyData *) = (!((m->nlist[i].cluster->cs) && (communicationsLinkup(m->nlist[i].cluster->cs)))) ? nodeDrawFrowny : nodeDraw;
        NodeDisplayStatus everythingStat = globalDispStat;
        everythingStat.familyBitmap = 0xFFFFFFFF;
        everythingStat.minPriority = 0;

        WatcherPropertyData *propertyData=findWatcherPropertyData(m->nlist[i].addr, GlobalWatcherPropertiesList); 

        // if(propertyData)
        //     LOG_DEBUG("Using properties for node " << m->nlist[i].addr; 

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
            nodeDrawFn(&(m->nlist[i]), dispType, ds, propertyData);
            m->nlist[i].y -= ndy;
            m->nlist[i].x -= ndx;
            m->nlist[i].color = NULL;
        }
        else
        {
            m->nlist[i].x += ndx;
            m->nlist[i].y += ndy;
            nodeDrawFn(&(m->nlist[i]), dispType, ds, propertyData);
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


