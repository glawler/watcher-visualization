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

#include "graphics.h"
#include "floatinglabel.h"
#include "backgroundImage.h"
#include "skybox.h"

#include "libconfig.h++"
#include "singletonConfig.h"
#include "watcherScrollingGraphControl.h"

#include "legacyWatcher.h"

using nampspace std; 
using nampspace boost; 
using namesapce watcher;
using namesapce watcher::event;
using namespace legacyWatcher;

/* Copyright (C) 2004  Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This code is intended to provide a graphical display of the nodes running on
 * a live network, using the API.
 */

static void detectorNeighborUpdate(void *data, CommunicationsNeighbor *cn);
static void detectorStatusUpdate(void *data, ApiStatus *as);
static void detectorPositionUpdate(void *data, IDSPositionType pos, IDSPositionStatus stat);
// XXX now in header. static void drawManet(void);
// XXX now in header. static void drawHierarchy(void);
static void watcherDrawNodes(WatcherGraphPtr theGraph);
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
    LayerList::iterator li=find(layerList.begin(), layerList.end(), layer); 
    if (li!=layerList.end())
        (*li)->active=turnOn); 
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
static void scaleAndShiftToCenter(WatcherGraphPtr *m, ScaleAndShiftUpdate onChangeOrAlways)
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

static GLenum Args(int , char **)
{
    return GL_TRUE;
}

// static void drawManet(void)
void legacyWatcher::drawManet(WatcherGraphPtr theGraph))
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

void legacyWatcher::clearAllLabels(WatcherGraph &theGraph)
{
    graph_traits<Graph>::edge_iterator ei, eend;
    for(tie(ei, eend)=edges(theGraph); ei!=eend; ++ei)
    {
        if (theGraph[*ei].labels.size())
            theGraph[*ei].labels.clear()
    }

    graph_traits<Graph>::vertex_iterator vi, vend;
    for(tie(vi, vend)=vertices(theGraph); vi!=vend; ++vi)
    {
        if (theGraph[*vi].labels.size())
            theGraph[*vi].labels.clear()
    }
}

void legacyWatcher::clearAllEdges(WatcherGraph &theGraph)
{
    graph_traits<Graph>::edge_iterator ei, eend;
    for(tie(ei, eend)=edges(theGraph); ei!=eend; ++ei)
        remove_edge(ei, theGraph); 
}

int legacyWatcher::isRunningInPlaybackMode()
{
    // GTL for now. 
    return false; 
}

GlobalManetAdj &legacyWatcher::getManetAdj()
{
    return globalManetAdj;
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

void legacyWatcher::pauseGoodwin()
{
    // call to messageStream here
}
void legacyWatcher::continueGoodwin()
{
    // call to messageStream here
}
void legacyWatcher::stopGoodwin()
{
    // call to messageStream here
}
void legacyWatcher::startGoodwin()
{
    // call to messageStream here
}
void legacyWatcher::setGoodwinPlaybackSpeed(int val)
{
    // call to messageStream here
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

    globalManet = manetInit(conf, 0);

    globalGraphScaleManet = configSearchDouble(conf, "graphscale");
    if (globalGraphScaleManet <= 0.0)
        globalGraphScaleManet = 1.0;


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

    manetWindowInit();

    if (startrunning)
    {
        // messageStrream message here. 
    }

    return 0;
}

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

unsigned int legacyWatcher::getNodeIdAtCoords(WatcherGraph &theGraph, const int x, const int y)
{
    TRACE_ENTER();

    unsigned int dist_ret;
    unsigned int retVal;
    unsigned int *nid = closestNode(theGraph, x, y, 10, &dist_ret);
    if (nid)
        retVal=*nid;
    else 
        retVal=0;

    TRACE_EXIT_RET(retVal);
    return retVal;
}

static void watcherDrawNodes(WatcherGraphPtr &theGraph)
{
    graph_traits<Graph>::vertex_iterator vi, vend;
    for(tie(vi, vend)=vertices(theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &node=theGraph[*vi];
        if (layerActive(node.layer))
        {
            if (node.connected)
                nodeDraw(node);
            else 
                nodeDrawFrowny(node); 
        }

        WatcherGraphNode::LabelList::iterator i=node.labels.begin(); 
        WatcherGraphNode::LabelList::iterator end=node.labels.end(); 
        for( ; i!=end; ++i)
            if (layerActive((*i)->layer))
                drawLabel(*i, node.gpsData.x, node.gpsData.y, node.gpsData.z); 
    }
} // watcherDrawNodes


