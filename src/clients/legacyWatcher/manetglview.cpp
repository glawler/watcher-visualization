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

/** 
 * @file manetglview.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <QtGui>
#include <QtOpenGL>
#include <math.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <algorithm>        // for fill_n()
#include <values.h>  // DBL_MAX
#include <GL/glut.h>
#include <sstream>

#include <libwatcher/watcherGraph.h>
#include <libwatcher/seekWatcherMessage.h>  // for epoch, eof
#include <libwatcher/playbackTimeRange.h>  // for epoch, eof
#include <libwatcher/listStreamsMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/streamDescriptionMessage.h>

#include "watcherAboutDialog.h"
#include "watcherConfigurationDialog.h"
#include "manetglview.h"
#include "watcherScrollingGraphControl.h"
#include "singletonConfig.h"
#include "backgroundImage.h"
#include "logger.h"
#include "watcherStreamListDialog.h"

INIT_LOGGER(manetGLView, "manetGLView");

using namespace watcher;
using namespace watcher::event;
using namespace std;
using namespace libconfig;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;

namespace watcher 
{ 
    float fast_arctan2( float y, float x )
    {
        const float ONEQTR_PI = M_PI / 4.0;
        const float THRQTR_PI = 3.0 * M_PI / 4.0;
        float r, angle;
        float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
        if ( x < 0.0f ) {
            r = (x + abs_y) / (abs_y - x);
            angle = THRQTR_PI;
        }
        else {
            r = (x - abs_y) / (x + abs_y);
            angle = ONEQTR_PI;
        }
        angle += (0.1963f * r * r - 0.9817f) * r;
        if ( y < 0.0f )
            return( -angle );     // negate if in quad III or IV
        else
            return( angle );
    }
}

/*
 * Get the world coordinate (x,y,z) for the projected coordinates (x, y)
 * at the world coordinate "z" using the given transformation matrices.
 */
int manetGLView::xyAtZForModelProjViewXY(
        XYWorldZToWorldXWorldY *xyz,
        size_t xyz_count,
        GLdouble modelmatrix[16], 
        GLdouble projmatrix[16], 
        GLint viewport[4])
{
    int ret;
    GLdouble modelmatrixinv[16];
    // get camera position in world coordinates
    invert4x4(modelmatrixinv, modelmatrix);
    for(;;)
    {
        if(xyz_count)
        {
            GLdouble wx, wy, wz;
            --xyz_count;
            if(gluUnProject(
                        xyz[xyz_count].x, 
                        xyz[xyz_count].y,
                        0,
                        modelmatrix, 
                        projmatrix, 
                        viewport, 
                        &wx, &wy, &wz) == GL_TRUE)
            {
                if(modelmatrixinv[15] != 0.0)
                {
                    // camera position is modelmatrixinv*[0 0 0 1]
                    GLdouble cx = modelmatrixinv[12]/modelmatrixinv[15];
                    GLdouble cy = modelmatrixinv[13]/modelmatrixinv[15];
                    GLdouble cz = modelmatrixinv[14]/modelmatrixinv[15];
                    // get (x, y) at the given z plane
                    GLdouble ratio = (cz)/(cz - wz);
                    xyz[xyz_count].worldX_ret = (cx - wx)*ratio;
                    xyz[xyz_count].worldY_ret = (cy - wy)*ratio;
                }
                else
                {
                    // orthographic projection, camera essentially at infinity.
                    xyz[xyz_count].worldX_ret = wx;
                    xyz[xyz_count].worldY_ret = wy;
                }
            }
            else
            {
                ret = EINVAL;
                break;
            }
        }
        else
        {
            ret = 0;
            break;
        }
    }
    return ret;
} // xyAtZForModelProjViewXY

void manetGLView::invert4x4(GLdouble dst[16], GLdouble const src[16])
{
    // use Cramer's Method
    // From Intel document AP-928 "Streaming SIMD Extensions - Inverse
    // of 4x4 Matrix",
    // <http://www.intel.com/design/pentiumiii/sml/24504301.pdf>
    GLdouble tmp[16];
    GLdouble det;
    size_t i;

    // pairs for first 8 elements 
    // From here on, we use the transpose of
    GLdouble pair0 = src[10]*src[15];
    GLdouble pair1 = src[14]*src[11];
    GLdouble pair2 = src[6]*src[15];
    GLdouble pair3 = src[14]*src[7];
    GLdouble pair4 = src[6]*src[11];
    GLdouble pair5 = src[10]*src[7];
    GLdouble pair6 = src[2]*src[15];
    GLdouble pair7 = src[14]*src[3];
    GLdouble pair8 = src[2]*src[11];
    GLdouble pair9 = src[10]*src[3];
    GLdouble pair10 = src[2]*src[7];
    GLdouble pair11 = src[6]*src[3];
    // pairs for second 8 elements
    GLdouble pair12 = src[8]*src[13];
    GLdouble pair13 = src[12]*src[9];
    GLdouble pair14 = src[4]*src[13];
    GLdouble pair15 = src[12]*src[5];
    GLdouble pair16 = src[4]*src[9];
    GLdouble pair17 = src[8]*src[5];
    GLdouble pair18 = src[0]*src[13];
    GLdouble pair19 = src[12]*src[1];
    GLdouble pair20 = src[0]*src[9];
    GLdouble pair21 = src[8]*src[1];
    GLdouble pair22 = src[0]*src[5];
    GLdouble pair23 = src[4]*src[1];

    // first 8 elements
    tmp[0] = (pair0*src[5]) + (pair3*src[9]) + (pair4*src[13]);
    tmp[0] -= (pair1*src[5]) + (pair2*src[9]) + (pair5*src[13]);
    tmp[1] = (pair1*src[1]) + (pair6*src[9]) + (pair9*src[13]);
    tmp[1] -= (pair0*src[1]) + (pair7*src[9]) + (pair8*src[13]);
    tmp[2] = (pair2*src[1]) + (pair7*src[5]) + (pair10*src[13]);
    tmp[2] -= (pair3*src[1]) + (pair6*src[5]) + (pair11*src[13]);
    tmp[3] = (pair5*src[1]) + (pair8*src[5]) + (pair11*src[9]);
    tmp[3] -= (pair4*src[1]) + (pair9*src[5]) + (pair10*src[9]);
    tmp[4] = (pair1*src[4]) + (pair2*src[8]) + (pair5*src[12]);
    tmp[4] -= (pair0*src[4]) + (pair3*src[8]) + (pair4*src[12]);
    tmp[5] = (pair0*src[0]) + (pair7*src[8]) + (pair8*src[12]);
    tmp[5] -= (pair1*src[0]) + (pair6*src[8]) + (pair9*src[12]);
    tmp[6] = (pair3*src[0]) + (pair6*src[4]) + (pair11*src[12]);
    tmp[6] -= (pair2*src[0]) + (pair7*src[4]) + (pair10*src[12]);
    tmp[7] = (pair4*src[0]) + (pair9*src[4]) + (pair10*src[8]);
    tmp[7] -= (pair5*src[0]) + (pair8*src[4]) + (pair11*src[8]);
    // second 8 elements
    tmp[8] = (pair12*src[7]) + (pair15*src[11]) + (pair16*src[15]);
    tmp[8] -= (pair13*src[7]) + (pair14*src[11]) + (pair17*src[15]);
    tmp[9] = (pair13*src[3]) + (pair18*src[11]) + (pair21*src[15]);
    tmp[9] -= (pair12*src[3]) + (pair19*src[11]) + (pair20*src[15]);
    tmp[10] = (pair14*src[3]) + (pair19*src[7]) + (pair22*src[15]);
    tmp[10] -= (pair15*src[3]) + (pair18*src[7]) + (pair23*src[15]);
    tmp[11] = (pair17*src[3]) + (pair20*src[7]) + (pair23*src[11]);
    tmp[11] -= (pair16*src[3]) + (pair21*src[7]) + (pair22*src[11]);
    tmp[12] = (pair14*src[10]) + (pair17*src[14]) + (pair13*src[6]);
    tmp[12] -= (pair16*src[14]) + (pair12*src[6]) + (pair15*src[10]);
    tmp[13] = (pair20*src[14]) + (pair12*src[2]) + (pair19*src[10]);
    tmp[13] -= (pair18*src[10]) + (pair21*src[14]) + (pair13*src[2]);
    tmp[14] = (pair18*src[6]) + (pair23*src[14]) + (pair15*src[2]);
    tmp[14] -= (pair22*src[14]) + (pair14*src[2]) + (pair19*src[6]);
    tmp[15] = (pair22*src[10]) + (pair16*src[2]) + (pair21*src[6]);
    tmp[15] -= (pair20*src[6]) + (pair23*src[10]) + (pair17*src[2]);

    // determinent
    det = (src[0]*tmp[0]) + (src[4]*tmp[1]) + (src[8]*tmp[2]) +
        (src[12]*tmp[3]);
    // adjust inverse
    for(i = 0;i < 16; i++) 
    {
        tmp[i] /= det;
    }
#if 0
    // check
    {
#  define MULV(i,j) ((src[((i)*4)]*tmp[(j)]) + (src[((i)*4)+1]*tmp[(j)+4]) + \
        (src[((i)*4)+2]*tmp[(j)+8]) + (src[((i)*4)+3]*tmp[(j)+12]))
        GLdouble foo[16] =
        {
            MULV(0,0), MULV(0,1), MULV(0,2), MULV(0,3), 
            MULV(1,0), MULV(1,1), MULV(1,2), MULV(1,3), 
            MULV(2,0), MULV(2,1), MULV(2,2), MULV(2,3), 
            MULV(3,0), MULV(3,1), MULV(3,2), MULV(3,3)
        }; 
        fprintf(stderr,
                "%s: --------------------------------------------\n",
                __func__);
        fprintf(stderr, "%s: %6.2lf %6.2lf %6.2lf %6.2lf     "
                "%6.2lf %6.2lf %6.2lf %6.2lf     "
                "%6.2lf %6.2lf %6.2lf %6.2lf\n", 
                __func__, src[0], src[4], src[8], src[12],
                tmp[0], tmp[4], tmp[8], tmp[12],
                foo[0], foo[4], foo[8], foo[12]);
        fprintf(stderr, "%s: %6.2lf %6.2lf %6.2lf %6.2lf  X  "
                "%6.2lf %6.2lf %6.2lf %6.2lf  =  "
                "%6.2lf %6.2lf %6.2lf %6.2lf\n", 
                __func__, src[1], src[5], src[9], src[13],
                tmp[1], tmp[5], tmp[9], tmp[13],
                foo[1], foo[5], foo[9], foo[13]);
        fprintf(stderr, "%s: %6.2lf %6.2lf %6.2lf %6.2lf     "
                "%6.2lf %6.2lf %6.2lf %6.2lf     "
                "%6.2lf %6.2lf %6.2lf %6.2lf\n", 
                __func__, src[2], src[6], src[10], src[14],
                tmp[2], tmp[6], tmp[10], tmp[14],
                foo[2], foo[6], foo[10], foo[14]);
        fprintf(stderr, "%s: %6.2lf %6.2lf %6.2lf %6.2lf     "
                "%6.2lf %6.2lf %6.2lf %6.2lf     "
                "%6.2lf %6.2lf %6.2lf %6.2lf\n", 
                __func__, src[3], src[7], src[11], src[15],
                tmp[3], tmp[7], tmp[11], tmp[15],
                foo[3], foo[7], foo[11], foo[15]);
        fprintf(stderr,
                "%s: --------------------------------------------\n",
                __func__);
    }
# undef MULV
#endif
    memcpy(dst, tmp, 16*sizeof(GLdouble));
    return;
} /* invert4x4 */
//
// Get a box at z that can be seen though the given viewport
//
// (assumes viewport[0] = viewport[1] = 0)
//
// returns non-zero on success.
//
int manetGLView::visibleDrawBoxAtZ(
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

void manetGLView::maxRectangle(
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
// Make the (xMin,yMin)-(xMax,yMax) rectangle on the xy-plane at
// z visible on the Manet screen
//
// onChangeOrAlways - ScaleAndShiftUpdateOnChange to only update scale
//                    and shift values if things have changed enough,
//                    ScaleAndShiftUpdateAlways to update even if things
//                    haven't changed at all Usually will not update
//                    scaling or shifting if nothing has changed
//
void manetGLView::scaleAndShiftToSeeOnManet(
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
        glScalef(1.0, 1.0, manetAdj.scaleZ); // getting scale x and y so start with unity
        glRotatef(manetAdj.angleX, 1.0, 0.0, 0.0);
        glRotatef(manetAdj.angleY, 0.0, 1.0, 0.0);
        glRotatef(manetAdj.angleZ, 0.0, 0.0, 1.0);
        glTranslatef(0.0, 0.0, manetAdj.shiftZ + 3); // getting shift x and y so start with zero
        glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glPopMatrix();
        if(visibleDrawBoxAtZ(viewport, z, modelmatrix, projmatrix, nodesWidth/nodesHeight, &wXMin, &wYMin, &wXMax, &wYMax)) 
        {
            // static time_t tick = 0;
            // time_t now = time(0);
            // get shift and scale
            manetAdj.shiftX = ((wXMin + wXMax) / 2) - ((xMin + xMax) / 2);
            manetAdj.shiftY = ((wYMin + wYMax) / 2) - ((yMin + yMax) / 2);
            manetAdj.scaleX = (wXMax - wXMin) / nodesWidth;
            manetAdj.scaleY = (wYMax - wYMin) / nodesHeight;
            if(manetAdj.scaleX > manetAdj.scaleY)
                manetAdj.scaleX = manetAdj.scaleY;
            else
                manetAdj.scaleY = manetAdj.scaleX;

            BackgroundImage &bgImage=BackgroundImage::getInstance(); 
            if (bgImage.centerImage())
            {
                GLfloat x,y,z,w,h;
                bgImage.getDrawingCoords(x,w,y,h,z);
                bgImage.setDrawingCoords(manetAdj.shiftX, w, manetAdj.shiftY, h, z); 
                bgImage.centerImage(false); 
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
                nccnt = 0;
            if(upfcnt)
                upfcnt = 0;
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
void manetGLView::scaleAndShiftToCenter(ScaleAndShiftUpdate onChangeOrAlways)
{
    double xMin = DBL_MAX;
    double xMax = -DBL_MAX;
    double yMin = DBL_MAX;
    double yMax = -DBL_MAX;
    double zMin = DBL_MAX;
    bool includeAntenna = false;     // antenna currently broken
    // bool includeHierarchy = isActive(HIERARCHY_LAYER); 

    // find drawing extents
    for (size_t i=0; i<wGraph->numValidNodes; i++) 
    {
        if (!wGraph->nodes[i].isActive) 
            continue;

        double r = 0;
        if(includeAntenna)
            r = antennaRadius; 

        {
            double nodeXMin = wGraph->nodes[i].x - r;
            double nodeXMax = wGraph->nodes[i].x + r;
            double nodeYMin = wGraph->nodes[i].y - r;
            double nodeYMax = wGraph->nodes[i].y + r;
            if(nodeXMin < xMin) xMin = nodeXMin;
            if(nodeXMax > xMax) xMax = nodeXMax;
            if(nodeYMin < yMin) yMin = nodeYMin;
            if(nodeYMax > yMax) yMax = nodeYMax;
        }
        if(wGraph->nodes[i].z < zMin)
            zMin = wGraph->nodes[i].z;
    }
    scaleAndShiftToSeeOnManet(xMin, yMin, xMax, yMax, zMin, onChangeOrAlways);
} // scaleAndShiftToCenter


void manetGLView::getShiftAmount(GLdouble &x_ret, GLdouble &y_ret)
{
    GLdouble z;
    GLdouble modelmatrix[16];
    GLdouble projmatrix[16];
    GLint viewport[4];
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);
    glScalef(manetAdj.scaleX, manetAdj.scaleY, manetAdj.scaleZ);
    glRotatef(manetAdj.angleX, 1.0, 0.0, 0.0);
    glRotatef(manetAdj.angleY, 0.0, 1.0, 0.0);
    glRotatef(manetAdj.angleZ, 0.0, 0.0, 1.0);
    glTranslatef(manetAdj.shiftX, manetAdj.shiftY, manetAdj.shiftZ + 3);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glPopMatrix();

    // if(globalManet)
    // {
    //     for(i = 0; ; ++i)
    //     {
    //         if(i == globalManet->numnodes)
    //         {
    //             z = -20;
    //             break;
    //         }
    //         if(globalGpsValidFlag[i])
    //         {
    //             z = globalManet->nlist[i].z;
    //             break;
    //         }
    //     }
    // }
    // else
    // {
    //     z = -20;
    // }
    z=-20;
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

void manetGLView::shiftBackgroundCenterLeft(double dx)
{
    GLfloat x, y, w, h, z;
    BackgroundImage &bg=BackgroundImage::getInstance();
    bg.getDrawingCoords(x, w, y, h, z);
    bg.setDrawingCoords(x+dx, w, y, h, z);
}

void manetGLView::shiftBackgroundCenterUp(double dy)
{
    GLfloat x, y, w, h, z;
    BackgroundImage &bg=BackgroundImage::getInstance();
    bg.getDrawingCoords(x, w, y, h, z);
    bg.setDrawingCoords(x, w, y+dy, h, z);
}

void manetGLView::shiftCenterRight()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterRight(shift);
}
void manetGLView::shiftCenterRight(double shift)
{
    manetAdj.shiftX -= shift;
    autoCenterNodesFlag=false;
} 

void manetGLView::shiftCenterLeft()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterLeft(shift);
}
void manetGLView::shiftCenterLeft(double shift)
{
    manetAdj.shiftX += shift;
    autoCenterNodesFlag=false;
} 

void manetGLView::shiftCenterDown()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterDown(shift);
}
void manetGLView::shiftCenterDown(double shift)
{
    manetAdj.shiftY += shift;
    autoCenterNodesFlag=false;
} 

void manetGLView::shiftCenterUp()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterUp(shift);
}
void manetGLView::shiftCenterUp(double shift)
{
    manetAdj.shiftY -= shift;
    autoCenterNodesFlag=false;
} 

void manetGLView::shiftCenterIn()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterIn(shift);
}
void manetGLView::shiftCenterIn(double shift)
{
    manetAdj.shiftZ -= shift;
    autoCenterNodesFlag=false;
} 

void manetGLView::shiftCenterOut()
{
    GLdouble shift, dummy;
    getShiftAmount(shift, dummy);
    shiftCenterOut(shift);
}
void manetGLView::shiftCenterOut(double shift)
{
    manetAdj.shiftZ += shift;
    autoCenterNodesFlag=false;
} 

void manetGLView::viewpointReset(void)
{
    manetAdj = manetAdjInit;
    update();
}

void manetGLView::zoomOut()
{
    manetAdj.scaleX /= 1.05;
    if (manetAdj.scaleX < 0.001) 
        manetAdj.scaleX = 0.001;
    manetAdj.scaleY = manetAdj.scaleX;
    autoCenterNodesFlag = 0;
}

void manetGLView::zoomIn()
{
    manetAdj.scaleX *= 1.05;
    manetAdj.scaleY = manetAdj.scaleX;
    autoCenterNodesFlag = 0;
}

void manetGLView::compressDistance()
{
    manetAdj.scaleZ -= 0.1;
    if (manetAdj.scaleZ < 0.02)
        manetAdj.scaleZ = 0.02;
    if(autoCenterNodesFlag)
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
} 

void manetGLView::expandDistance()
{
    manetAdj.scaleZ += 0.1;
} 

#define TEXT_SCALE 20
#define ARROW_SCALE_ZOOM_FACTOR 1.05

void manetGLView::textZoomReset(void)
{
    scaleText=TEXT_SCALE;
}

void manetGLView::arrowZoomReset(void)
{
    scaleLine= 1.0;
}

void manetGLView::arrowZoomIn(void)
{
    scaleLine*= ARROW_SCALE_ZOOM_FACTOR;
}

void manetGLView::arrowZoomOut(void)
{
    scaleLine/= ARROW_SCALE_ZOOM_FACTOR;
}

void manetGLView::rotateX(float deg)
{
    manetAdj.angleX += deg;
    while(manetAdj.angleX >= 360.0)
        manetAdj.angleX -= 360.0;
    while(manetAdj.angleX < 0)
        manetAdj.angleX += 360.0;
    if(autoCenterNodesFlag)
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
} 

void manetGLView::rotateY(float deg)
{
    manetAdj.angleY += deg;
    while(manetAdj.angleY >= 360.0)
        manetAdj.angleY -= 360.0;
    while(manetAdj.angleY < 0)
        manetAdj.angleY += 360.0;
    if(autoCenterNodesFlag)
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
} 

void manetGLView::rotateZ(float deg)
{
    manetAdj.angleZ += deg;
    while(manetAdj.angleZ >= 360.0)
        manetAdj.angleZ -= 360.0;
    while(manetAdj.angleZ < 0)
        manetAdj.angleZ += 360.0;
    if(autoCenterNodesFlag)
        scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
} 
//static 
bool manetGLView::gps2openGLPixels(double &x, double &y, double &z, const GPSMessage::DataFormat &format)
{
    // TRACE_ENTER();

    double inx=x, iny=y, inz=z;

    if (format==GPSMessage::UTM)
    {
        //
        // There is no UTM zone information in the UTM GPS packet, so we assume all data is in a single
        // zone. Because of this, no attempt is made to place the nodes in the correct positions on the 
        // planet surface. We just use the "lat" "long" data as pure x and y coords in a plane, offset
        // by the first coord we get. (Nodes are all centered around 0,0 where, 0,0 is defined 
        // by the first coord we receive. 
        //
        // if (iny < 91 && inx > 0) 
        //     LOG_WARN("Received GPS data that looks like lat/long in degrees, but GPS data format mode is set to UTM in cfg file."); 

        static double utmXOffset=0.0, utmYOffset=0.0;
        static bool utmOffInit=false;
        if (utmOffInit==false)
        {
            utmOffInit=true;
            utmXOffset=inx;
            utmYOffset=iny; 

            LOG_INFO("Got first UTM coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" << utmXOffset << " y=" << utmYOffset);
        }

        x=inx-utmXOffset;
        y=iny-utmYOffset;    
        z=inz;

        // LOG_DEBUG("UTM given locations: lon=" << location->lon << " lat=" << location->lat << " alt=" << location->alt);
        // LOG_DEBUG("UTM node coords: x=" << us->x << " y=" << us->y << " z=" << us->z);
    }
    else // default to lat/long/alt WGS84
    {
        if (inx > 180) 
            LOG_WARN("Received GPS data (" << inx << ", " << iny << ", " << inz << ") that may be UTM (long>180), but GPS data format mode is set to lat/long degrees in cfg file."); 

        x=inx*gpsScale;
        y=iny*gpsScale;
        z=inz;

        static double xOff=0.0, yOff=0.0;
        static bool xOffInit=false;
        if (xOffInit==false)
        {
            xOffInit=true;
            xOff=inx;
            yOff=iny;

            LOG_INFO("Got first Lat/Long coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" 
                    << xOff << " y=" << yOff);
        }

        x-=xOff;
        y-=yOff;

        // LOG_DEBUG("Got GPS: long:" << location->lon << " lat:" << location->lat << " alt:" << location->alt); 
        // LOG_DEBUG("translated GPS: x:" << us->x << " y:" << us->y << " z:" << us->z); 
    }
    // LOG_DEBUG("Converted GPS from " << inx << ", " << iny << ", " << inz << " to " << x << ", " << y << ", " << z); 
    // TRACE_EXIT();
    return true;
}

manetGLView::manetGLView(QWidget *parent) : 
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    streamsDialog(NULL),
    wGraph(NULL),
    watcherdConnectionThread(NULL),
    maintainGraphThread(NULL),
    checkIOThread(NULL),
    streamRate(1.0),
    playbackPaused(false),
    autorewind(false), 
    messageStreamFiltering(false), 
    sliderPressed(false),
    currentMessageTimestamp(0),
    playbackRangeEnd(0),
    playbackRangeStart(0),
    showWallTimeinStatusString(true),
    showPlaybackTimeInStatusString(true),
    showPlaybackRangeString(true),
    showVerboseStatusString(false),
    showDebugInfo(false), 
    showStreamDescription(true),
    scaleText(20.0), 
    scaleLine(1.0), 
    gpsScale(60000), 
    layerPadding(2.0), 
    antennaRadius(100.0),
    ghostLayerTransparency(0.15),
    statusFontPointSize(10),
    maxNodes(0),
    maxLayers(0),
    monochromeMode(false), 
    threeDView(false),
    backgroundImage(false), 
    statusFontName("Helvetica"),
    hierarchyRingColor(colors::blue),
    playbackStartTime(SeekMessage::eof),  // live mode
    autoCenterNodesFlag(false),
    nodesDrawn(0), edgesDrawn(0), labelsDrawn(0),
    framesDrawn(0), fpsTimeBase(0), framesPerSec(0.0)
{
    TRACE_ENTER();
    manetAdjInit.angleX=0.0;
    manetAdjInit.angleY=0.0;
    manetAdjInit.angleZ=0.0;
    manetAdjInit.scaleX=0.035;
    manetAdjInit.scaleY=0.035;
    manetAdjInit.scaleZ=0.03;
    manetAdjInit.shiftX=0.0;
    manetAdjInit.shiftY=0.0;
    manetAdjInit.shiftZ=0.0;
    setFocusPolicy(Qt::StrongFocus); // tab and click to focus

    // Don't oeverwrite QPAinter...
    setAutoFillBackground(false);

    connect(this, SIGNAL(connectNewLayer(const QString)), this, SLOT(newLayerConnect(const QString)));

    TRACE_EXIT();
}

manetGLView::~manetGLView()
{
    shutdown();
}

void manetGLView::shutdown() 
{
    if (messageStream) { 
        if (messageStream->connected())  
            messageStream->stopStream();
        // Don't do this as the thread may still use it.
        // messageStream.reset(); 
    }

    saveConfiguration();

    // order is important here. Destroy in dependency order. 
    boost::thread *threads[]={
        checkIOThread,
        maintainGraphThread, 
        watcherdConnectionThread, 
    };
    for (unsigned int i=0; i<sizeof(threads)/sizeof(threads[0]); i++) {
        if (threads[i]) {
            threads[i]->interrupt(); // this does not seem to work
            threads[i]->join();
            delete threads[i];
            threads[i]=NULL;
        }
    }
    if (wGraph) {
        delete wGraph;
        wGraph=NULL;
    }

    for (vector<StringIndexedMenuItem*>::iterator i=layerMenuItems.begin(); i!=layerMenuItems.end(); ++i)
        delete *i;
}

void manetGLView::newLayerConnect(const QString &name) 
{
    QAction *action=new QAction(name, (QObject*)this);
    action->setCheckable(true);

    StringIndexedMenuItem *item = new StringIndexedMenuItem(name); 
    connect(action, SIGNAL(triggered(bool)), item, SLOT(showMenuItem(bool)));
    connect(item, SIGNAL(showMenuItem(QString, bool)), this, SLOT(layerToggle(QString, bool)));
    connect(this, SIGNAL(layerToggled(QString, bool)), item, SLOT(setChecked(QString, bool)));
    connect(item, SIGNAL(setChecked(bool)), action, SLOT(setChecked(bool)));
    layerMenuItems.push_back(item);     // We have to keep 'item' alive somewhere. 
    layerMenu->addAction(action); 
}

void manetGLView::addLayerMenuItem(const GUILayer &layer, bool active)
{
    TRACE_ENTER();

    // name2Layer() creates layer, loads configuration, and sets as active
    if (!wGraph->layerExists(layer)) {
        size_t l=wGraph->name2LayerIndex(layer);
        wGraph->layers[l].isActive=active;
    }

    if (layerMenu) {
        emit connectNewLayer(QString(layer.c_str())); 
    }

    // Could use a few more type conversions for string here...
    emit layerToggled(QString::fromStdString(string(layer)), active);

    TRACE_EXIT();
}

void manetGLView::showPlaybackTime(bool isOn)
{
    TRACE_ENTER();
    showPlaybackTimeInStatusString=isOn;
    TRACE_EXIT();
}
void manetGLView::showPlaybackRange(bool isOn)
{
    TRACE_ENTER();
    showPlaybackRangeString=isOn;
    TRACE_EXIT();
}
void manetGLView::showWallTime(bool isOn)
{
    TRACE_ENTER();
    showWallTimeinStatusString=isOn;
    TRACE_EXIT();
}

bool manetGLView::loadConfiguration()
{
    TRACE_ENTER();

    scaleText=TEXT_SCALE;
    scaleLine=1.0;
    monochromeMode = false;
    threeDView = true;
    backgroundImage = true;
    bool retVal=true;


    //
    // Check configuration for GUI settings.
    //
    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();
    libconfig::Setting &root=cfg.getRoot();

    root.lookupValue("maxNodes", maxNodes);
    root.lookupValue("maxLayers", maxLayers); 

    if (!root.lookupValue("server", serverName)) {
        serverName="localhost"; 
        LOG_WARN("Please specify the server name in the cfg file");
        LOG_WARN("I set the default to localhost, but that may not be what you want."); 
        root.add("server", Setting::TypeString)=serverName;
    }

    while (maxNodes==0 || maxLayers==0 || serverName.empty()) { 
        WatcherConfigurationDialog *d=new WatcherConfigurationDialog(serverName, maxNodes, maxLayers);  
        d->setModal(true); 
        d->exec();
        LOG_DEBUG("WatcherConfigDialog result: " << ((d->result()==QDialog::Rejected)?"Rejected":"Accepted")); 
        if (d->result()==QDialog::Rejected) { 
            SingletonConfig::unlock();
            LOG_FATAL("User did not give needed info to start, exiting."); 
            return false;
        }
        LOG_DEBUG("Got values from conf dialog - server: " << serverName << ", maxNodes: " << maxNodes << ", maxLayers: " << maxLayers); 
        if (!root.exists("server"))
            root.add("server", Setting::TypeString);
        root["server"]=serverName;
        if (!root.exists("maxNodes"))
            root.add("maxNodes", Setting::TypeInt);
        root["maxNodes"]=static_cast<int>(maxNodes);
        if (!root.exists("maxLayers"))
            root.add("maxLayers", Setting::TypeInt);
        root["maxLayers"]=static_cast<int>(maxLayers);
    }

    // WatcherGraph() grabs the Config lock, so it must be unlocked when created.
    SingletonConfig::unlock();
    wGraph=new WatcherGraph(maxNodes, maxLayers); 
    wGraph->locationTranslationFunction=boost::bind(&manetGLView::gps2openGLPixels, this, _1, _2, _3, _4); 
    SingletonConfig::lock();

    try {
        string prop="layers";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);

        libconfig::Setting &layers=cfg.lookup(prop);

        // antenna radius is not a layer, move to 'view' menu or elsewhere
        // // We cheat a little here and always make sure there's an antenna radius "layer"
        // if (!layers.exists(ANTENNARADIUS_LAYER))
        //     layers.add(ANTENNARADIUS_LAYER, Setting::TypeBoolean)=false;

        LOG_INFO("Reading layer states from cfg file");
        bool foundPhy=false;
        int layerNum=layers.getLength();
        for (int i=0; i<layerNum; i++)
        {
            string name(layers[i].getName());
            LOG_DEBUG("Reading layer menu config for " << name); 
            bool val=true;
            if (!layers.lookupValue(name, val))
                layers.add(name, Setting::TypeBoolean)=val;  // Shouldn't happen unless misformed cfg file. 
            addLayerMenuItem(name, val);
            if (name==PHYSICAL_LAYER) 
                foundPhy=true;
        }
        if (!foundPhy)
            addLayerMenuItem(PHYSICAL_LAYER, true);

        struct 
        {
            const char *prop; 
            bool def; 
            bool *val; 
        } boolVals[] = 
        {
            { "nodes3d", true, &threeDView },
            { "monochrome", false, &monochromeMode }, 
            { "displayBackgroundImage", true, &backgroundImage },
            { "showVerboseStatusString", false, &showVerboseStatusString }, 
            { "showWallTime", true, &showWallTimeinStatusString }, 
            { "showPlaybackTime", true, &showPlaybackTimeInStatusString },
            { "showPlaybackRange", true, &showPlaybackRangeString },
            { "showDebugInfo", false, &showDebugInfo },
            { "autorewind", true, &autorewind },
            { "messageStreamFiltering", false, &messageStreamFiltering }
        }; 
        for (size_t i=0; i<sizeof(boolVals)/sizeof(boolVals[0]); i++)
        {
            prop=boolVals[i].prop;
            bool boolVal=boolVals[i].def; 
            if (!root.lookupValue(prop, boolVal))
                root.add(prop, libconfig::Setting::TypeBoolean)=boolVal;
            LOG_DEBUG("Setting " << boolVals[i].prop << " to " << (boolVal?"true":"false")); 
            *boolVals[i].val=boolVal; 
        }
        emit threeDViewToggled(threeDView);
        emit monochromeToggled(monochromeMode);
        emit backgroundImageToggled(backgroundImage);
        emit loopPlaybackToggled(autorewind);
        emit enableStreamFiltering(messageStreamFiltering);
        emit checkPlaybackTime(showPlaybackTimeInStatusString);
        emit checkPlaybackRange(showPlaybackRangeString);
        emit checkWallTime(showWallTimeinStatusString);

        string hrc("blue");
        struct 
        {
            const char *prop; 
            const char *def; 
            string *val; 
        } strVals[] = 
        {
            { "statusFontName", "Helvetica", &statusFontName },
            { "hierarchyRingColor", "blue", &hrc } 
        }; 
        for (size_t i=0; i<sizeof(strVals)/sizeof(strVals[0]); i++)
        {
            prop=strVals[i].prop;
            string strVal=strVals[i].def; 
            if (!root.lookupValue(prop, strVal))
                root.add(prop, libconfig::Setting::TypeString)=strVal;
            *strVals[i].val=strVal; 
            LOG_DEBUG("Setting " << strVals[i].prop << " to " << strVal);
        }
        hierarchyRingColor.fromString(hrc);

        struct 
        {
            const char *prop; 
            float def; 
            float *val; 
        } floatVals[] = 
        {
            { "scaleText", 1.0, &scaleText }, 
            { "scaleLine", 1.0, &scaleLine }, 
            { "layerPadding", 1.0, &layerPadding }, 
            { "gpsScale", 80000.0, &gpsScale }, 
            { "antennaRadius", 200.0, &antennaRadius },
            { "ghostLayerTransparency", 0.15, &ghostLayerTransparency }
        }; 
        for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++)
        {
            prop=floatVals[i].prop;
            float floatVal=floatVals[i].def;
            if (!root.lookupValue(prop, floatVal))
                root.add(prop, libconfig::Setting::TypeFloat)=floatVals[i].def;
            *floatVals[i].val=floatVal;
            LOG_DEBUG("Setting " << floatVals[i].prop << " to " << *floatVals[i].val); 
        }

        struct 
        {
            const char *prop; 
            int def; 
            int *val; 
        } intVals[] = 
        {
            { "statusFontPointSize", 12, &statusFontPointSize }
        }; 
        for (size_t i=0; i<sizeof(intVals)/sizeof(intVals[0]); i++)
        {
            prop=intVals[i].prop;
            int intVal=intVals[i].def; 
            if (!root.lookupValue(prop, intVal))
                root.add(prop, libconfig::Setting::TypeInt)=intVal;
            *intVals[i].val=intVal; 
            LOG_DEBUG("Setting " << intVals[i].prop << " to " << intVal);
        }

        struct 
        {
            const char *prop; 
            long long int def;
            long long int *val; 
        } longlongIntVals[] = 
        {
            { "playbackStartTime", SeekMessage::eof, &playbackStartTime } 
        }; 
        for (size_t i=0; i<sizeof(longlongIntVals)/sizeof(longlongIntVals[0]); i++) {
            prop=longlongIntVals[i].prop;
            long long int intVal=longlongIntVals[i].def; 
            if (!root.lookupValue(prop, intVal))
                root.add(prop, libconfig::Setting::TypeInt64)=intVal;
            *longlongIntVals[i].val=intVal; 
            LOG_DEBUG("Setting " << longlongIntVals[i].prop << " to " << intVal);
        }

        //
        // Load background image settings
        //
        prop="backgroundImage";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &s=cfg.lookup(prop);

        prop="imageFile"; 
        string strVal;
        if (!s.lookupValue(prop, strVal) || strVal=="none")   // If we don't have the setting or the it's set to 'do not use bg image'
        {
            LOG_INFO("watcherBackground:imageFile entry not found (or it equals \"none\") in configuration file, "
                    "disabling background image functionality");
            if (strVal.empty())
                s.add(prop, libconfig::Setting::TypeString)="none";
            toggleBackgroundImage(false);
            emit enableBackgroundImage(false);
        }
        else
        {
            BackgroundImage &bgImage=BackgroundImage::getInstance(); 
            const char *ext=rindex(strVal.data(), '.');
            if (!ext)
            {
                LOG_ERROR("I have no idea what kind of file the background image " << strVal << " is. I only support BMP and PPM"); 
                exit(1);
            }
            else if (0==strncasecmp(ext+sizeof(char), "bmp", 3))
            {
                if (!bgImage.loadBMPFile(strVal.data()))
                {
                    LOG_FATAL("Unable to load background BMP image in watcher from file: " << strVal); 
                    TRACE_EXIT_RET_BOOL(false);
                    return false;
                }
            }
            else if (0==strncmp("ppm", ext+sizeof(char), 3))
            {
                if (!bgImage.loadPPMFile(strVal.data()))
                {
                    LOG_FATAL("Unable to load background PPM image in watcher from file: " << strVal); 
                    TRACE_EXIT_RET_BOOL(false);
                    return false;
                }
            }
        }
        // bg image location and size.
        prop="coordinates";
        float coordVals[5]={0.0, 0.0, 0.0, 0.0, 0.0};
        if (!s.exists(prop))
        {
            s.add(prop, libconfig::Setting::TypeArray);
            for (size_t i=0; i<sizeof(coordVals)/sizeof(coordVals[0]); i++)
                s[prop].add(libconfig::Setting::TypeFloat)=coordVals[i];
        }
        else
        {
            for (size_t i=0; i<sizeof(coordVals)/sizeof(coordVals[0]); i++)
                coordVals[i]=s[prop][i];
            BackgroundImage &bg=BackgroundImage::getInstance();
            bg.setDrawingCoords(coordVals[0], coordVals[1], coordVals[2], coordVals[3], coordVals[4]); 
        }

        //
        // Load viewpoint
        //
        prop="viewPoint";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &vp=cfg.lookup(prop); 

        struct 
        {
            const char *type;
            float *data[3];
        } viewPoints[] =
        {
            { "angle", { &manetAdj.angleX, &manetAdj.angleY, &manetAdj.angleZ }},
            { "scale", { &manetAdj.scaleX, &manetAdj.scaleY, &manetAdj.scaleZ }},
            { "shift", { &manetAdj.shiftX, &manetAdj.shiftY, &manetAdj.shiftZ }}
        };
        for (size_t i=0; i<sizeof(viewPoints)/sizeof(viewPoints[0]);i++)
        {
            if (!vp.exists(viewPoints[i].type))
            {
                vp.add(viewPoints[i].type, libconfig::Setting::TypeArray);
                for (size_t j=0; j<sizeof(viewPoints[i].data)/sizeof(viewPoints[i].data[0]); j++)
                    vp[viewPoints[i].type].add(libconfig::Setting::TypeFloat);
            }
            else
            {
                libconfig::Setting &s=vp[viewPoints[i].type];
                for (int j=0; j<s.getLength(); j++)
                    *viewPoints[i].data[j]=s[j];
            }
        }

        LOG_INFO("Set viewpoint - angle: " << manetAdj.angleX << ", " << manetAdj.angleY << ", " << manetAdj.angleZ);
        LOG_INFO("Set viewpoint - scale: " << manetAdj.scaleX << ", " << manetAdj.scaleY << ", " << manetAdj.scaleZ);
        LOG_INFO("Set viewpoint - shift: " << manetAdj.shiftX << ", " << manetAdj.shiftY << ", " << manetAdj.shiftZ);

        // background color
        prop="backgroundColor";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &bgColSet=cfg.lookup(prop);

        struct 
        {
            const char *name;
            float val;
        } bgColors[] = 
        {
            { "r", 0.0 }, 
            { "g", 0.0 }, 
            { "b", 0.0 }, 
            { "a", 255.0 }
        };
        for (size_t i=0; i<sizeof(bgColors)/sizeof(bgColors[0]);i++)
            if (!bgColSet.lookupValue(bgColors[i].name, bgColors[i].val))
                bgColSet.add(bgColors[i].name, libconfig::Setting::TypeFloat)=bgColors[i].val;

        glClearColor(bgColors[0].val, bgColors[1].val,bgColors[2].val,bgColors[3].val);

    }
    catch (const libconfig::SettingException &e) {
        LOG_ERROR("Error loading configuration at " << e.getPath() << ": " << e.what());
        retVal=false;
    }

    SingletonConfig::unlock();

    // 
    // Set up timer callbacks.
    //
    QTimer *watcherIdleTimer = new QTimer(this);
    QObject::connect(watcherIdleTimer, SIGNAL(timeout()), this, SLOT(watcherIdle()));
    watcherIdleTimer->start(100); 

    // start work threads
    watcherdConnectionThread=new boost::thread(boost::bind(&manetGLView::connectStream, this));

    TRACE_EXIT();
    return retVal;
}

QSize manetGLView::minimumSizeHint() const
{
    TRACE_ENTER();
    QSize retVal=QSize(50, 50);
    TRACE_EXIT_RET(retVal.width() << retVal.height()); 
    return retVal;
}

QSize manetGLView::sizeHint() const
{
    TRACE_ENTER();
    QSize retVal=QSize(400, 400);
    TRACE_EXIT_RET(retVal.width() << retVal.height()); 
    return retVal;
}

// Values figure out by hand using Number and shift keys.
static GLfloat matShine=0.6;
static GLfloat specReflection[] = { 0.05, 0.05, 0.05, 1.0f };
static GLfloat globalAmbientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat posLight0[]={ 50.0f, 50.0f, 000.0f, 1.0f };
static GLfloat ambLight0[]={ 0.25, 0.25, 0.25, 1.0f };
static GLfloat specLight0[]={ 0.1f, 0.1f, 0.1f, 1.0f };
static GLfloat diffLight0[]={ 0.05, 0.05, 0.05, 1.0f };

void manetGLView::initializeGL()
{
    TRACE_ENTER();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); 

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE); 

    glEnable(GL_TEXTURE_2D);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

    glShadeModel(GL_SMOOTH); 
    // glShadeModel(GL_FLAT); 

    // LIGHT0
    glLightfv(GL_LIGHT0, GL_POSITION, posLight0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambLight0); 
    glLightfv(GL_LIGHT0, GL_SPECULAR, specLight0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffLight0);

    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0); 

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    TRACE_EXIT();
}

void manetGLView::connectStream() 
{
    TRACE_ENTER();
    while(1) {

        this_thread::interruption_point();
        if (!messageStream) 
            messageStream=MessageStreamPtr(new MessageStream(serverName)); 

        while (!messageStream->connected()) {
            this_thread::interruption_point();
            if (!messageStream->connect(true)) { 
                LOG_WARN("Unable to connect to server at " << serverName << ". Trying again in 2 seconds");
                sleep(2); 
            }
        }

        messageStream->setDescription("legacy watcher gui");
        messageStream->startStream();
        messageStream->getMessageTimeRange();
        messageStream->enableFiltering(messageStreamFiltering);

        // Tell the watcherd that we want/don't want messages for this layer.
        if (messageStreamFiltering) { 
            for (size_t l=0; l<wGraph->numValidLayers; l++) { 
                MessageStreamFilterPtr f(new MessageStreamFilter);
                f->setLayer(wGraph->layers[l].layerName); 
                if (wGraph->layers[l].isActive) 
                    messageStream->addMessageFilter(f);
                else
                    messageStream->removeMessageFilter(f);
            }
        }

        // spawn work threads
        if (!checkIOThread) 
            checkIOThread=new boost::thread(boost::bind(&manetGLView::checkIO, this));
        if (!maintainGraphThread) 
            maintainGraphThread=new boost::thread(boost::bind(&manetGLView::maintainGraph, this));

        do {
            sleep(2); 
            this_thread::interruption_point();
            if (!messageStream || !messageStream->connected())
                break;
        } while(1);
    }
    TRACE_EXIT();
}

void manetGLView::checkIO()
{
    TRACE_ENTER();

    while(true) {
        this_thread::interruption_point();
        bool timeRangeMessageSent=false;
        MessagePtr message;
        while(messageStream && messageStream->getNextMessage(message)) {
            static unsigned long long messageCount=0;
            LOG_DEBUG("Got message number " <<  ++messageCount << " : " << *message);

            if (!isFeederEvent(message->type)) {
                if (message->type==PLAYBACK_TIME_RANGE_MESSAGE_TYPE) {
                    PlaybackTimeRangeMessagePtr trm(dynamic_pointer_cast<PlaybackTimeRangeMessage>(message));
                    playbackRangeEnd=trm->max_;
                    playbackRangeStart=trm->min_;
                    if (playbackSlider)
                        playbackSlider->setRange(playbackRangeStart/1000, playbackRangeEnd/1000);
                    if (!currentMessageTimestamp)
                        currentMessageTimestamp=
                            playbackStartTime==SeekMessage::epoch ? playbackRangeStart : 
                            playbackStartTime==SeekMessage::eof ? playbackRangeEnd : playbackStartTime;
                    timeRangeMessageSent=false;
                } else if (message->type == LIST_STREAMS_MESSAGE_TYPE) {
                    ListStreamsMessagePtr m(dynamic_pointer_cast<ListStreamsMessage>(message));
                    BOOST_FOREACH(EventStreamInfoPtr ev, m->evstreams) {
                        streamsDialog->addStream(ev->uid, ev->description);
                    }
                } else if (message->type == SPEED_MESSAGE_TYPE) {
                    // notification from the watcher daemon that the shared stream speed has changed
                    SpeedMessagePtr sm(dynamic_pointer_cast<SpeedMessage>(message));
                    changeSpeed(sm->speed);
                    if (sm->speed == 0)
                        playbackPaused = true;
                } else if (message->type == STOP_MESSAGE_TYPE) {
                    playbackPaused = true;
                } else if (message->type == START_MESSAGE_TYPE) {
                    playbackPaused = false;
                } else if (message->type == STREAM_DESCRIPTION_MESSAGE_TYPE) {
                    StreamDescriptionMessagePtr m(dynamic_pointer_cast<StreamDescriptionMessage>(message));
                    streamDescription = m->desc;
                }

                // End of handling non feeder messages. 
                continue;
            }

            // When control reaches this point, events are being streamed
            playbackPaused = false;

            // DataPoint data is handled directly by the scrolling graph thing.
            if (message->type==DATA_POINT_MESSAGE_TYPE) {
                WatcherScrollingGraphControl *sgc=WatcherScrollingGraphControl::getWatcherScrollingGraphControl();
                sgc->handleDataPointMessage(dynamic_pointer_cast<DataPointMessage>(message));
            }

            currentMessageTimestamp=message->timestamp;
            if (!sliderPressed)
                playbackSlider->setValue(currentMessageTimestamp/1000); 

            if (currentMessageTimestamp>playbackRangeEnd+10000 && !timeRangeMessageSent) { 
                messageStream->getMessageTimeRange();
                timeRangeMessageSent=true;
            }

            // Really need to make layers a member of a base class...
            GUILayer layer;
            switch (message->type)
            {
                case LABEL_MESSAGE_TYPE: layer=(dynamic_pointer_cast<LabelMessage>(message))->layer; break;
                case EDGE_MESSAGE_TYPE: layer=(dynamic_pointer_cast<EdgeMessage>(message))->layer; break;
                case COLOR_MESSAGE_TYPE: layer=(dynamic_pointer_cast<ColorMessage>(message))->layer; break;
                case CONNECTIVITY_MESSAGE_TYPE: layer=(dynamic_pointer_cast<ConnectivityMessage>(message))->layer; break;
                case NODE_PROPERTIES_MESSAGE_TYPE: layer=(dynamic_pointer_cast<NodePropertiesMessage>(message))->layer; break;
                default: break;
            }

            // do this before calling updateGraph() as it will create the layer if not found. 
            if (!layer.empty()) {
                if (!wGraph->layerExists(layer)) {
                    LOG_DEBUG("Adding new layer to layer menu: " << layer); 
                    addLayerMenuItem(layer, true); 
                }
            }

            // update graph is now thread-safe
            wGraph->updateGraph(message);
        }

        TRACE_EXIT();
    }
}

void manetGLView::updatePlaybackSliderRange()
{
    TRACE_ENTER();
    // if (playbackSlider)
    // {
    //     // playbackSlider->setRange(0, (playbackRangeEnd-playbackRangeStart)/1000);
    //     // playbackSlider->setValue((currentMessageTimestamp-playbackRangeStart)/1000); 
    //     playbackSlider->setRange(playbackRangeStart/1000, playbackRangeEnd/1000);
    //     playbackSlider->setValue(currentMessageTimestamp/1000); 
    // }
    TRACE_EXIT();
}

void manetGLView::watcherIdle()
{
    TRACE_ENTER();

    if (autorewind) {
        static time_t noNewMessagesForSeconds=0;
        if (currentMessageTimestamp==playbackRangeEnd) { 
            time_t now=time(NULL);
            if (!noNewMessagesForSeconds)
                noNewMessagesForSeconds=now;

            if (now-noNewMessagesForSeconds>10) {
                LOG_WARN("Autorewind engaged - jumping to start of data...");
                LOG_DEBUG("playbackRangeEnd " << playbackRangeEnd << " noNewMessagesForSeconds: " << noNewMessagesForSeconds); 
                noNewMessagesForSeconds=0;
                rewindToStartOfPlayback();
            }
        }
        else 
            noNewMessagesForSeconds=0;
    }

    updateGL();

    TRACE_EXIT();
}

void manetGLView::maintainGraph() 
{
    TRACE_ENTER();
    while (true) {
        this_thread::interruption_point(); 
        if (!playbackPaused)       // If paused, just keep things as they are.
            wGraph->doMaintanence(currentMessageTimestamp); // check expiration, etc. 
        usleep(100000);
    }
    TRACE_EXIT();
}

void manetGLView::paintGL()
{
    TRACE_ENTER();

    if (!messageStream || !messageStream->connected())
        drawNotConnectedState();
    else 
    {
        drawManet();

        glPushMatrix();
        glPushAttrib(GL_ALL_ATTRIB_BITS); 

        // drawStatusString uses QPainter so must be called 
        // after all openGL calls
        drawStatusString(); 

        if (showDebugInfo)
            drawDebugInfo();

        glPopAttrib(); 
        glPopMatrix();
    }

    TRACE_EXIT();
}

void manetGLView::drawDebugInfo()
{
    TRACE_ENTER();

    ostringstream info;
    info << "Messages sent: " << messageStream->messagesSent << endl;
    info << "Messages arrived: " << messageStream->messagesArrived << endl;
    info << "Messages dropped: " << messageStream->messagesDropped << endl;
    info << "Messages queued: " << messageStream->messageQueueSize() << endl;

    info << "N/E/L: " << nodesDrawn << "/" << edgesDrawn << "/" << labelsDrawn << endl;
    nodesDrawn=edgesDrawn=labelsDrawn=0;

    framesDrawn++;
    int time=glutGet(GLUT_ELAPSED_TIME);
    if (time - fpsTimeBase > 1000) {
        framesPerSec = framesDrawn*1000.0/(time-fpsTimeBase);
        fpsTimeBase = time;        
        framesDrawn = 0;
    }
    info << "FPS: " << framesPerSec << endl;
    // info << "mat shine: " << matShine << endl;
    // info << "spec reflect: " << specReflection[0] << endl;
    // info << "amb light: " << ambLight0[0] << endl;
    // info << "diff light: " << diffLight0[0] << endl;
    // info << "spec light: " << specLight0[0] << endl;
    // info << "global amb light: " << globalAmbientLight[0] << endl;

    QPainter painter(this);
    QString text(info.str().c_str());
    QFont font(statusFontName.c_str(), statusFontPointSize); 
    QFontMetrics metrics = QFontMetrics(font);
    int border = qMax(4, metrics.leading());
    QRect rect = metrics.boundingRect(0, 0, width() - 2*border, int(height()*0.125), Qt::AlignLeft | Qt::TextWordWrap, text);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font);
    painter.setPen(Qt::white); 
    double padding=0.01;
    painter.drawText(width()*padding, height()*padding, rect.width(), rect.height(), Qt::AlignLeft | Qt::TextWordWrap, text);
    painter.end();

    TRACE_EXIT();
}

void manetGLView::drawNotConnectedState()
{
    TRACE_ENTER();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    glScalef(1.0, 1.0, 1.0);

    // Color -- red
    glColor4ub(0xFF, 0x00, 0x00, 0xFF); 

    // "head"
    glTranslatef(0.0, 0.0, -10.0);
    glNormal3f(0.0, 0.0, 1.0); 
    glutSolidTorus(0.1, 2.5, 60, 60); 

    // Eyes
    double xOffs[]={-1.0, 1.0}; 
    for (int i=0; i<2; i++) {
        glPushMatrix();
        glTranslatef(xOffs[i], 1.0, 0.0); 
        glRotatef(90, 1.0, 0.0, 0.0); 
        glRotatef(45, 0.0, 1.0, 0.0); 
        GLUquadricObj *quadric=gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluQuadricOrientation(quadric,GLU_OUTSIDE);
        gluCylinder(quadric, 0.06, 0.03, 0.5, 32, 32); 
        glRotatef(90, 0.0, 1.0, 0.0); 
        gluCylinder(quadric, 0.06, 0.03, 0.5, 32, 32); 
        glRotatef(90, 0.0, 1.0, 0.0); 
        gluCylinder(quadric, 0.06, 0.03, 0.5, 32, 32); 
        glRotatef(90, 0.0, 1.0, 0.0); 
        gluCylinder(quadric, 0.06, 0.03, 0.5, 32, 32); 
        gluDeleteQuadric(quadric);
        glPopMatrix();
    }

    // mouth
    {
        GLUquadricObj *quadric=gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        gluQuadricOrientation(quadric,GLU_OUTSIDE);
        glPushMatrix();
        glTranslatef(-1.0, -1.0, 0.0); 
        glRotatef(90, 1.0, 0.0, 0.0); 
        glRotatef(315, 0.0, 1.0, 0.0); 
        gluCylinder(quadric, 0.06, 0.03, 0.8, 32, 32); 
        glRotatef(135, 0.0, 1.0, 0.0); 
        gluCylinder(quadric, 0.06, 0.06, 2.0, 32, 32); 
        glPopMatrix();
        glPushMatrix();
        glTranslatef(1.0, -1.0, 0.0); 
        glRotatef(90, 1.0, 0.0, 0.0); 
        glRotatef(45, 0.0, 1.0, 0.0); 
        gluCylinder(quadric, 0.06, 0.03, 0.8, 32, 32); 
        glPopMatrix();
    }


    QString errMess("Unable to connect to watcher daemon on ");
    errMess+=serverName.c_str();
    errMess+=". Trying to reconnect every 2 seconds.";
    renderText(12, height()-12, errMess); 

    glPopMatrix();

    TRACE_EXIT();
}

void manetGLView::drawText( GLdouble x, GLdouble y, GLdouble z, GLdouble scale, char *text, GLdouble lineWidth)
{
    int i;
    GLfloat lineheight=- glutStrokeWidth(GLUT_STROKE_ROMAN,'W') * scale;   // TOJ: need scale arg here?  

    glPushMatrix();
    glPushAttrib(GL_LINE_WIDTH);
    glLineWidth(lineWidth); 

    glTranslatef(x,y,z);
    glScaled(scale,scale,scale);
    for (i = 0; text[i]; i++)
        switch(text[i])
        {
            case '\n':
                glPopMatrix();
                glTranslatef(0.0,lineheight,0.0);
                glPushMatrix();
                glScaled(scale,scale,scale);
                break;
            default:
                glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
                break;
        }
    glPopAttrib(); 
    glPopMatrix();

}

void manetGLView::drawManet(void)
{
    // watcher::Skybox *sb=watcher::Skybox::getSkybox();
    // if (sb)
    //     sb->drawSkybox(manetAdj.angleX, manetAdj.angleY, manetAdj.angleZ);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);

    glScalef(manetAdj.scaleX, manetAdj.scaleY, manetAdj.scaleZ);
    glRotatef(manetAdj.angleX, 1.0, 0.0, 0.0);
    glRotatef(manetAdj.angleY, 0.0, 1.0, 0.0);
    glRotatef(manetAdj.angleZ, 0.0, 0.0, 1.0);
    glTranslatef(manetAdj.shiftX, manetAdj.shiftY, manetAdj.shiftZ);

    if (backgroundImage)        // need to draw this fisrt for transparency to work properly. 
    {
        BackgroundImage &bgi=BackgroundImage::getInstance();
        GLfloat minx, width, miny, height, z;
        bgi.getDrawingCoords(minx, width, miny, height, z); 
        if (z>-25)
            bgi.setDrawingCoords(minx, width, miny, height, -25); 
        qglColor(QColor("white")); 
        bgi.drawImage(); 
    }

    drawGraph(wGraph); 
}

void manetGLView::drawGraph(WatcherGraph *&graph) 
{
    // draw all physical layer nodes
    for (size_t n=0; n<graph->numValidNodes; n++) 
        if (graph->nodes[n].isActive)
            drawNode(graph->nodes[n], true);

    // draw all edges and labels on all active layers
    // (must grab read lock around dynamic data like label lists)
    for (size_t l=0; l<graph->numValidLayers; l++) { 
        if (!graph->layers[l].isActive) 
            continue;
        bool layerEmpty=true;
        for (size_t i=0; i<graph->numValidNodes; i++) {
            if (graph->nodes[i].isActive) {
                // drawNode(graph->nodes[i], false);      // "ghost" nodes
                for (unsigned int j=0; j<graph->numValidNodes; j++) 
                    if (graph->nodes[j].isActive && graph->layers[l].edges[i][j]) {
                        drawEdge(graph->layers[l].edgeDisplayInfo, graph->nodes[i], graph->nodes[j]);
                        layerEmpty=false;
                    }
            }
        }

        {
            shared_lock<shared_mutex> readLock(graph->layers[l].floatingLabelsMutex); 
            BOOST_FOREACH(const WatcherLayerData::FloatingLabels::value_type &label, graph->layers[l].floatingLabels)  {
                drawLabel(label.lat, label.lng, label.alt, label); 
            }
        }

        for (unsigned int n=0; n<graph->numValidNodes; n++) {
            if (graph->nodes[n].isActive) {
                shared_lock<shared_mutex> readLock(graph->layers[l].nodeLabelsMutexes[n]); 
                BOOST_FOREACH(const WatcherLayerData::NodeLabels::value_type &label, graph->layers[l].nodeLabels[n]) {
                    drawLabel(graph->nodes[n].x, graph->nodes[n].y, graph->nodes[n].z, label); 
                }
            }
        }
        // move up "layerPadding" units before drawing the next layer if the previous layer was not empty.
        if (!layerEmpty) 
            glTranslatef(0.0, 0.0, layerPadding);  
    }
}

void manetGLView::drawStatusString()
{
    // // draw status string
    QPainter painter(this);

    ptime now = from_time_t(time(NULL));

    string buf;

    Timestamp nowTS=getCurrentTime();
    if (playbackPaused)
        buf="(paused)";
    else if (nowTS-2500.0<currentMessageTimestamp)
        buf="(live)";
    else if (playbackRangeEnd==currentMessageTimestamp && nowTS > playbackRangeEnd)
        buf="(end of data)";
    else
        buf="(playback)";

    if (showStreamDescription) {
        buf += "\nDescription: ";
        buf += streamDescription;
    }

    if (showWallTimeinStatusString)
    {
        buf+="\nWall Time: ";
        buf+=posix_time::to_iso_extended_string(now);
    }
    if (showPlaybackTimeInStatusString)
    {
        buf+="\nPlay Time: ";
        buf+=posix_time::to_iso_extended_string(from_time_t(currentMessageTimestamp/1000));
    }
    if (showPlaybackRangeString)
    {
        buf+="\nTime Range: ";
        buf+=posix_time::to_iso_extended_string(from_time_t(playbackRangeStart/1000));
        buf+=" to ";
        buf+=posix_time::to_iso_extended_string(from_time_t(playbackRangeEnd/1000));
    }

    if (nowTS-2500.0<currentMessageTimestamp)
        painter.setPen(QColor(monochromeMode ? "black" : "green")); 
    else if (playbackRangeEnd==currentMessageTimestamp && nowTS > playbackRangeEnd)
        painter.setPen(QColor(monochromeMode ? "black" : "blue")); 
    else
        painter.setPen(QColor(monochromeMode ? "black" : "red")); 

    if (showVerboseStatusString)
    {
        char buff[256];
        snprintf(buff, sizeof(buff), "\nLoc: %3.1f, %3.1f, %3.1f, scale: %f, %f, %f, text: %f lpad: %f",
                manetAdj.shiftX, manetAdj.shiftY, manetAdj.shiftZ, 
                manetAdj.scaleX, manetAdj.scaleY, manetAdj.scaleZ, 
                scaleText, layerPadding); 
        buf+=string(buff); 
    }

    QString text(buf.c_str());
    QFont font(statusFontName.c_str(), statusFontPointSize); 
    QFontMetrics metrics = QFontMetrics(font);
    int border = qMax(4, metrics.leading());
    QRect rect = metrics.boundingRect(0, 0, width() - 2*border, int(height()*0.125), Qt::AlignCenter | Qt::TextWordWrap, text);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font);
    double padding=0.01;
    painter.drawText(width()*padding, height()-rect.height()-(height()*padding), rect.width(), rect.height(), Qt::AlignLeft | Qt::TextWordWrap, text);
    painter.end();
}

void manetGLView::setPlaybackSlider(QSlider *s)
{
    TRACE_ENTER();
    if (!s)
    {
        LOG_WARN("Passed a null pointer to setPlaybackSlider() - that can't be good."); 
        TRACE_EXIT();
        return;
    }

    playbackSlider=s;
    connect(playbackSlider, SIGNAL(sliderReleased()), this, SLOT(updatePlaybackSliderFromGUI()));
    connect(playbackSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMovedInGUI(int)));
    connect(playbackSlider, SIGNAL(sliderPressed()), this, SLOT(sliderPressedInGUI()));
}

void manetGLView::sliderPressedInGUI()
{
    TRACE_ENTER();
    if (!playbackSlider)
        return;
    sliderPressed=true;
    TRACE_EXIT();
}

void manetGLView::sliderMovedInGUI(int /*newVal*/)
{
    TRACE_ENTER();
    if (!playbackSlider)
        return;
    // playbackSlider->setStatusTip(QString(playbackSlider->value()));
    // LOG_DEBUG("sliderMoved - new value: " << newVal); 
    TRACE_EXIT();
}

void manetGLView::updatePlaybackSliderFromGUI()
{
    TRACE_ENTER();
    if (!messageStream || !playbackSlider) {
        TRACE_EXIT();
        return;
    }

    Timestamp newStart(playbackSlider->value());
    newStart*=1000;
    LOG_DEBUG("slider update - new start time: " << newStart << " slider position: " << playbackSlider->value() << " cur mess ts: " << currentMessageTimestamp); 

    currentMessageTimestamp=newStart;  // So it displays in status string immediately. 
    playbackSlider->setValue(newStart/1000); 
    messageStream->clearMessageCache();
    messageStream->setStreamTimeStart(newStart); 
    messageStream->startStream(); 

    sliderPressed=false;
    TRACE_EXIT();
}

void manetGLView::drawEdge(const EdgeDisplayInfo &edge, const NodeDisplayInfo &node1, const NodeDisplayInfo &node2)
{
    TRACE_ENTER(); 
    edgesDrawn++;

    GLdouble x1=node1.x;
    GLdouble y1=node1.y;
    GLdouble z1=node1.z;
    GLdouble x2=node2.x;
    GLdouble y2=node2.y;
    GLdouble z2=node2.z;

    double width=edge.width;

    GLfloat edgeColor[]={
        edge.color.r/255.0, 
        edge.color.g/255.0, 
        edge.color.b/255.0, 
        edge.color.a/255.0, 
    };

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    if (monochromeMode)
        glColor4fv(black);
    else
        glColor4fv(edgeColor);

    if (!threeDView)
    {
        double ax=fast_arctan2(x1-x2,y1-y2);
        double cmx = sin(ax)*width;   // cos(a-M_PI_2)*width
        double cpx = -cmx;            // cos(a+M_PI_2)*width
        double smx = -cos(ax)*width;  // sin(a-M_PI_2)*width
        double spx = -smx;            // cos(a+M_PI_2)*width

        glBegin(GL_POLYGON);

        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(x1+smx,y1+cmx,z1);
        glVertex3f(x1+spx,y1+cpx,z1);
        glVertex3f(x2+spx,y2+cpx,z2);
        glVertex3f(x1+smx,y1+cmx,z1);

        glEnd();
    }
    else
    {
        GLUquadricObj *quadric=gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);

        float vx = x2-x1;
        float vy = y2-y1;
        float vz = z2-z1;

        //handle the degenerate case of z1 == z2 with an approximation
        if(vz == 0)
            vz = .0001;

        float v = sqrt( vx*vx + vy*vy + vz*vz );
        float ax = 57.2957795*acos( vz/v );
        if ( vz < 0.0 )
            ax = -ax;
        float rx = -vy*vz;
        float ry = vx*vz;

        glPushMatrix();

        //draw the cylinder body
        glTranslatef( x1,y1,z1 );
        glRotatef(ax, rx, ry, 0.0);
        gluQuadricOrientation(quadric,GLU_OUTSIDE);
        gluCylinder(quadric, width, 0, v, 15, 15);

        glPopMatrix();

        gluDeleteQuadric(quadric);
    }

    // draw the edge's label, if there is one.
    if (edge.label!="none")
    {
        if (monochromeMode)
        {
            const GLfloat localblack[]={0.0,0.0,0.0,1.0};
            glColor4fv(localblack);
        }
        else
        {
            // This color should be cfg-erable.
            const GLfloat clr[]={
                edge.labelColor.r/255.0, 
                edge.labelColor.g/255.0, 
                edge.labelColor.b/255.0, 
                edge.labelColor.a/255.0
            };
            glColor4fv(clr);
        }

        GLdouble lx=(x1+x2)/2.0; 
        GLdouble ly=(y1+y2)/2.0; 
        GLdouble lz=(z1+z2)/2.0; 
        GLdouble a=fast_arctan2(x1-x2 , y1-y2);
        GLdouble th=10.0;
        renderText(lx+sin(a-M_PI_2),ly+cos(a-M_PI_2)*th, lz, 
                QString(edge.label.c_str()),
                QFont(QString(edge.labelFont.c_str()), (int)(edge.labelPointSize*manetAdj.scaleX*scaleText)));
    }

    TRACE_EXIT(); 
}

bool manetGLView::isActive(const watcher::GUILayer &layer)
{
    TRACE_ENTER();
    bool retVal=false;
    if (wGraph->layerExists(layer))
        retVal=wGraph->layers[wGraph->name2LayerIndex(layer)].isActive;
    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

void manetGLView::drawNode(const NodeDisplayInfo &node, bool physical)
{
    TRACE_ENTER(); 
    if (physical)
        nodesDrawn++;

    // LOG_DEBUG("Drawing node on " << (physical?"non":"") << "physical layer."); 

    GLdouble x=node.x;
    GLdouble y=node.y;
    GLdouble z=node.z;

    glPushMatrix();
    glTranslated(x, y, z);

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat nodeColor[]={
        node.color.r/255.0, 
        node.color.g/255.0, 
        node.color.b/255.0, 
        physical ? node.color.a/255.0 : ghostLayerTransparency
    };

    drawNodeLabel(node, physical);

    if (monochromeMode)
        glColor4fv(black);
    else
        glColor4fv(nodeColor);

    // if (isActive(ANTENNARADIUS_LAYER)) { 
    //     // 30.86666666666666666666 meters == 1 second of latitude
    //     // 1/30.86666666666666666666 of a second of lat == 1 meter
    //     // 0.000278 is a second in decimal degrees
    //     // 0.000278*(1/30.86666)=.00000900648142688583==change in lat for one meter.
    //     // This is still wrong though - or at least does not match MANE's idea of distance.
    //     // drawWireframeSphere(antennaRadius*0.00000900648142688583*gpsScale);
    //     // The number below is just an eyeball value.
    //     drawWireframeSphere(antennaRadius*0.000015*gpsScale);
    // }

    // Handle size after drawing antenna so as to not scale it
    handleSize(node);
    handleProperties(node);

    handleSpin(threeDView, node);
    switch(node.shape)
    {
        case NodePropertiesMessage::CIRCLE: drawSphere(4); break;
        case NodePropertiesMessage::SQUARE: drawCube(4); break;
        case NodePropertiesMessage::TRIANGLE: drawPyramid(4); break;
        case NodePropertiesMessage::TORUS: drawTorus(2,4); break;
        case NodePropertiesMessage::TEAPOT: drawTeapot(4); break;
        case NodePropertiesMessage::NOSHAPE: /* What is the shape of no shape? */ break;
    }

    glPopMatrix();

    TRACE_EXIT(); 
}

void manetGLView::drawNodeLabel(const NodeDisplayInfo &node, bool physical)
{
    TRACE_ENTER();

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat nodeColor[]={
        node.labelColor.r/255.0, 
        node.labelColor.g/255.0, 
        node.labelColor.b/255.0, 
        physical ? node.labelColor.a/255.0 : ghostLayerTransparency
    };

    if (monochromeMode)
        glColor4fv(black);
    else
        glColor4fv(nodeColor);

    // LOG_DEBUG("drawing node - font: " << node.labelFont << ", size: " << node.labelPointSize); 

    renderText(0, 6, 3, QString(node.get_label().c_str()),
            QFont(node.labelFont.c_str(), 
                (int)(node.labelPointSize*manetAdj.scaleX*scaleText))); 

    TRACE_EXIT();
}

void manetGLView::drawLabel(GLfloat inx, GLfloat iny, GLfloat inz, const LabelDisplayInfo &label)
{
    TRACE_ENTER(); 
    labelsDrawn++;
    // 
    int fgColor[]={
        label.foregroundColor.r, 
        label.foregroundColor.g, 
        label.foregroundColor.b, 
        label.foregroundColor.a
    };
    int bgColor[]={
        label.backgroundColor.r, 
        label.backgroundColor.g, 
        label.backgroundColor.b, 
        label.backgroundColor.a
    };

    if (monochromeMode)
        for (unsigned int i=0; i<sizeof(bgColor)/sizeof(bgColor[0]); i++)
            if (i==3)
                fgColor[i]=255;
            else
                fgColor[i]=0; 

    float offset=4.0;

    // LOG_DEBUG("Building QFont for label \"" << label.labelText << "\": name: " << label.fontName << ", size: " <<  label.pointSize <<  " (adj:" << (int)(label.pointSize*manetAdj.scaleX*scaleText) << ")"); 
    QFont f(label.fontName.c_str(), (int)(label.pointSize*manetAdj.scaleX*scaleText)); 

    // Do cheesy shadow effect as I can't get a proper bounding box around the text as
    // QFontMetric lisea bout how wide/tall the bounding box is. 
    // if (!monochromeMode)
    // {
    //     qglColor(QColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3])); 
    //     QFontMetrics fm(f);
    //     double shadowOffset=fm.height()*.02;
    //     renderText(inx+offset+shadowOffset, iny+offset+shadowOffset, inz+1.0, label->labelText.c_str(), f); 
    // }

    qglColor(QColor(fgColor[0], fgColor[1], fgColor[2], fgColor[3])); 
    renderText(inx+offset, iny+offset, inz+2.0, label.labelText.c_str(), f); 

    // Use GLUT to draw labels, will be much faster.
    // glColor3f(fgColor[0], fgColor[1], fgColor[2]);
    // glPushMatrix();
    // glLoadIdentity();
    // setOrthographicProjection();
    // for (char c=label->labelText.c_str(); *c != '\0'; c++) 
    //     glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    // glPopMatrix();
    // resetPerspectiveProjection();

    TRACE_EXIT(); 
}

void manetGLView::resizeGL(int width, int height)
{
    TRACE_ENTER();
    // GLint viewport[4];
    // glGetIntegerv(GL_VIEWPORT, viewport);
    // fprintf(stderr, "Reshape cur (%d, %d)\n", viewport[2], viewport[3]);
    // fprintf(stderr, "Reshape given (%d, %d)\n", awidth, aheight);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, GLfloat(width) / GLfloat(height), 1.0, 50.0);
    if(autoCenterNodesFlag) 
        scaleAndShiftToCenter(ScaleAndShiftUpdateOnChange);

    TRACE_EXIT();
}

void manetGLView::resetPosition()
{
    TRACE_ENTER();
    manetAdj = manetAdjInit;
    updateGL();
    TRACE_EXIT();
}

void manetGLView::showKeyboardShortcuts()
{
    TRACE_ENTER();
    const char *help=
        "Keyboard shortcuts:\n"
        "C   - change background color\n"
        "F   - change information text font\n"
        "T   - reset layer padding to 0\n"
        "D   - Display debugging information\n"
        "k/l - increase/decrease gps scale\n"
        "a/s - increase/decrease node label font size\n"
        "t/y - increase/decrease padding between layers\n" 
        "n/m - shift center in/out\n"
        "z/x - compress/decompress field distance\n"
        "e/r - rotate x axis\n"
        "d/f - rotate y axis\n"
        "c/v - rotate z axis\n"
        "left arrow  - shift field right\n"
        "right arrow - shift field left\n"
        "up arrow    - shift field down\n"
        "down arrow  - shift field up\n"
        "=/+ - auto center nodes\n"
        "space - pause playback\n"
        "?/h - show this dialog\n"; 
    // QMessageBox info;
    // info.addButton(QMessageBox::Ok);
    // info.setWindowModality(Qt::NonModal);
    // info.setText(QString(help)); 
    // info.show();
    QMessageBox::information(this, tr("Keyboard Shortcuts"), help);
    TRACE_EXIT();
}

void manetGLView::spawnAboutBox()
{
    TRACE_ENTER();
    WatcherAboutDialog *d = new WatcherAboutDialog;
    d->show();
    TRACE_EXIT();
}

void manetGLView::setGPSScale() 
{
    TRACE_ENTER();
    bool ok;
    double value=QInputDialog::getDouble(this, tr("GPS Scale"), tr("Please enter the new GPS Scale value"), 
            gpsScale, 1, DBL_MAX, 1, &ok);

    if (ok) {
        LOG_DEBUG("Setting GPS scale to " << value); 
        gpsScale=value;

        Config &cfg=SingletonConfig::instance();
        SingletonConfig::lock();
        libconfig::Setting &root=cfg.getRoot();
        root["gpsScale"]=gpsScale;
        SingletonConfig::unlock();
    }
    TRACE_EXIT();
}

void manetGLView::setEdgeWidth()
{
    TRACE_ENTER();

    bool ok;
    double value=QInputDialog::getDouble(this, tr("Set Edge Width"), tr("Plese enter the new Scale width for all edges"), 
            2, 1, DBL_MAX, 1, &ok);

    if (ok) {
        LOG_DEBUG("Setting all edge widths to " << value); 
        for (size_t l=0; l<wGraph->numValidLayers; l++) 
            wGraph->layers[l].edgeDisplayInfo.width=value;
    }

    TRACE_EXIT();
}

void manetGLView::keyPressEvent(QKeyEvent * event)
{
    TRACE_ENTER();

    int qtKey = event->key();
    Qt::KeyboardModifiers kbMods=event->modifiers();

    switch(qtKey) {
        case Qt::Key_Left:  shiftCenterRight(); break;
        case Qt::Key_Right: shiftCenterLeft(); break;
        case Qt::Key_Up:    shiftCenterDown(); break;
        case Qt::Key_Down:  shiftCenterUp(); break;
        case Qt::Key_N:     shiftCenterIn(); break; 
        case Qt::Key_M:     shiftCenterOut(); break;
        case Qt::Key_Q:     zoomOut(); break;
        case Qt::Key_W:     zoomIn(); break;
        case Qt::Key_A:     scaleText++; break;
        case Qt::Key_S:     scaleText--; if (scaleText<1) scaleText=1; break;
        case Qt::Key_Z:     compressDistance(); break;
        case Qt::Key_X:     expandDistance(); break;
        case Qt::Key_E:     rotateX(-5.0); break;
        case Qt::Key_R:     rotateX(5.0); break;
        case Qt::Key_D:     { 
                                if (kbMods & Qt::ShiftModifier) { 

                                    LOG_DEBUG("Got cap D turning on debugging info"); 
                                    showDebugInfo=!showDebugInfo;
                                    if (messageStream) { 
                                        messageStream->messagesSent=0;
                                        messageStream->messagesArrived=0;
                                        messageStream->messagesDropped=0;
                                    }
                                }
                                else 
                                    rotateY(-5.0); break;
                            }
                            break;
        case Qt::Key_F:
                            if (kbMods & Qt::ShiftModifier) {

                                LOG_DEBUG("Got cap F in keyPressEvent - spawning font chooser for info string"); 
                                bool ok;
                                QFont initial(statusFontName.c_str(), statusFontPointSize); 
                                QFont font=QFontDialog::getFont(&ok, initial, this); 
                                if (ok)
                                {
                                    statusFontName=font.family().toStdString(); 
                                    statusFontPointSize=font.pointSize(); 
                                }
                            }
                            else { 
                                rotateY(5.0);
                            }
                            break; 
        case Qt::Key_C:     { 
                                if (kbMods & Qt::ShiftModifier) { 
                                    LOG_DEBUG("Got cap C in keyPressEvent - spawning color chooser for background color"); 
                                    QRgb rgb=0xffffffff;
                                    bool ok=false;
                                    rgb=QColorDialog::getRgba(rgb, &ok);
                                    if (ok)
                                        glClearColor(qRed(rgb)/255.0, qGreen(rgb)/255.0, qBlue(rgb)/255.0, qAlpha(rgb)/255.0);
                                }
                                else 
                                    rotateZ(-5.0); 
                            }
                            break;
        case Qt::Key_V:     rotateZ(5.0); break;
        case Qt::Key_K:     gpsScale+=10; break;
        case Qt::Key_L:     gpsScale-=10; break;
        case Qt::Key_T:     {
                                if (kbMods & Qt::ShiftModifier)  
                                    layerPadding=0;
                                else
                                    layerPadding+=2; break;
                            }
                            break;
        case Qt::Key_Y:     layerPadding-=2; if (layerPadding<=0) layerPadding=0; break;
                            // case Qt::Key_B:     
                            //     layerToggle(BANDWIDTH_LAYER, isActive(BANDWIDTH_LAYER)); 
                            //     emit bandwidthToggled(isActive(BANDWIDTH_LAYER));
                            //     break;
        case Qt::Key_Equal:
        case Qt::Key_Plus: 
                            autoCenterNodesFlag=true;
                            scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
                            autoCenterNodesFlag=false;
                            break;
        case Qt::Key_Space:
                            break;
                            // globalReplay.runFlag = !globalReplay.runFlag;
                            // if (globalReplay.runFlag)
                            //     messageStream.startStream();
                            // else
                            //     messageStream.stopStream();
                            // break;

                            // GTL TODO: add shortcuts for ff/rew, etc. 
                            // case 't': globalReplay.step = 1000; break;
                            // case 'a' - 'a' + 1: arrowZoomOut(); break;
                            // case 's' - 'a' + 1: arrowZoomIn(); break;
                            // case 'r' - 'a' + 1: textZoomReset(); arrowZoomReset(); viewpointReset(); break;
                            //
        case Qt::Key_QuoteLeft:     {
                                        matShine+=0.05; 
                                        matShine=matShine>1.0 ? 1.0 : matShine; 
                                        glMaterialf(GL_FRONT, GL_SHININESS, matShine);
                                    }
                                    break;
        case Qt::Key_AsciiTilde: {
                                     matShine-=0.05; 
                                     matShine=matShine<0.0 ? 0.0 : matShine;
                                     glMaterialf(GL_FRONT, GL_SHININESS, matShine);
                                 }
                                 break;
        case Qt::Key_1:     {
                                for (unsigned int i=0; i<3; i++) 
                                    specReflection[i]+=0.05; 
                                for (unsigned int i=0; i<3; i++) 
                                    specReflection[i]=specReflection[i]>1.0 ? 1.0 : specReflection[i]; 
                                glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
                            }
                            break;
                            // there is no way to use Key_1 w/Shift - so we lose portability across keyboard layouts. 
        case Qt::Key_Exclam: {
                                 for (unsigned int i=0; i<3; i++) 
                                     specReflection[i]-=0.05;
                                 for (unsigned int i=0; i<3; i++) 
                                     specReflection[i]=specReflection[i]<0.0 ? 0.0 : specReflection[i]; 
                                 glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
                             }
                             break;
        case Qt::Key_2:     {
                                GLenum l=GL_LIGHT0;
                                GLenum p=GL_SPECULAR;
                                GLfloat *d=specLight0; 
                                GLfloat o=0.05;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]+=o;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i]; 
                                glLightfv(l, p, d); 
                            }
                            break;
        case Qt::Key_At: {
                             GLenum l=GL_LIGHT0;
                             GLenum p=GL_SPECULAR;
                             GLfloat *d=specLight0; 
                             GLfloat o=-0.05;
                             for (unsigned int i=0; i<3; i++) 
                                 d[i]+=o;
                             for (unsigned int i=0; i<3; i++) 
                                 d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i]; 
                             glLightfv(l, p, d); 
                         }
                         break;
        case Qt::Key_3:     {
                                GLenum l=GL_LIGHT0;
                                GLenum p=GL_DIFFUSE;
                                GLfloat *d=diffLight0; 
                                GLfloat o=0.05;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]+=o;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i]; 
                                glLightfv(l, p, d); 
                            }
                            break;
        case Qt::Key_NumberSign: {
                                     GLenum l=GL_LIGHT0;
                                     GLenum p=GL_DIFFUSE;
                                     GLfloat *d=diffLight0; 
                                     GLfloat o=-0.05;
                                     for (unsigned int i=0; i<3; i++) 
                                         d[i]+=o;
                                     for (unsigned int i=0; i<3; i++) 
                                         d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i];
                                     glLightfv(l, p, d); 
                                 }
                                 break;
        case Qt::Key_4:     {
                                GLenum l=GL_LIGHT0;
                                GLenum p=GL_AMBIENT;
                                GLfloat *d=ambLight0; 
                                GLfloat o=0.05;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]+=o;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i]; 
                                glLightfv(l, p, d); 
                            }
                            break;
        case Qt::Key_Dollar: {
                                 GLenum l=GL_LIGHT0;
                                 GLenum p=GL_AMBIENT;
                                 GLfloat *d=ambLight0; 
                                 GLfloat o=-0.05;
                                 for (unsigned int i=0; i<3; i++) 
                                     d[i]+=o;
                                 for (unsigned int i=0; i<3; i++) 
                                     d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i];
                                 glLightfv(l, p, d); 
                             }
                             break;
        case Qt::Key_5:     {
                                GLenum l=GL_LIGHT_MODEL_AMBIENT;
                                GLfloat *d=globalAmbientLight; 
                                GLfloat o=0.05;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]+=o;
                                for (unsigned int i=0; i<3; i++) 
                                    d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i]; 
                                glLightModelfv(l, d); 
                            }
                            break;
        case Qt::Key_Percent: {
                                  GLenum l=GL_LIGHT_MODEL_AMBIENT;
                                  GLfloat *d=globalAmbientLight; 
                                  GLfloat o=-0.05;
                                  for (unsigned int i=0; i<3; i++) 
                                      d[i]+=o;
                                  for (unsigned int i=0; i<3; i++) 
                                      d[i]=d[i]>1.0 ? 1.0 : d[i]<0.0 ? 0.0 : d[i]; 
                                  glLightModelfv(l, d); 
                              }
                              break;
        case Qt::Key_Question:
        case Qt::Key_H:
                              showKeyboardShortcuts();
                              break;

        default:
                              event->ignore();
    }

    update();

    TRACE_EXIT();
}

void manetGLView::mouseDoubleClickEvent(QMouseEvent *event)
{
    TRACE_ENTER();

    // Check for shift click and move bg image if shifted
    Qt::KeyboardModifiers mods=event->modifiers();
    if (mods & Qt::ShiftModifier)
    {
        // This should work - but doesn't.
        //
        // GLdouble modelmatrix[16];
        // GLdouble projmatrix[16];
        // GLint viewport[4];

        // glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        // glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        // glGetIntegerv(GL_VIEWPORT, viewport);

        // GLfloat winx=event->x(), winy=event->y(), winz=0.0;
        // winy=viewport[3]-winy;

        // BackgroundImage &bg=BackgroundImage::getInstance();
        // GLfloat x,y,w,h,z;
        // bg.getDrawingCoords(x, w, y, h, z);

        // GLdouble objx, objy, objz;
        // if (gluUnProject(winx, winy, winz, modelmatrix, projmatrix, viewport, &objx, &objy, &objz)==GL_TRUE)
        // {
        //     bg.setDrawingCoords(x+objx, w, y+objy, h, z);
        // }

        // hack - let the centering code center the BG image.
        BackgroundImage &bg=BackgroundImage::getInstance();
        bg.centerImage(true); 
    }
    else
    {
        unsigned int nodeId=getNodeIdAtCoords(event->x(), event->y());
        if(nodeId)
        {
            emit nodeDataInGraphsToggled(nodeId);
        }
    }
    update();
    TRACE_EXIT();
}

unsigned int manetGLView::getNodeIdAtCoords(const int x, const int y)
{
    TRACE_ENTER();

    unsigned int retVal=0;
    unsigned long min_dist = ULONG_MAX; // distance squared to closest
    unsigned r=15;      // Shrug, seems to do the trick
    unsigned long r2 = r*r;
    size_t found = 0;    // index of closest
    GLdouble modelmatrix[16];
    GLdouble projmatrix[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // convert y-from-top to y-from-bottom
    int convy = viewport[3] - y;

    for (size_t i=0; i<wGraph->numValidNodes; i++) 
    {
        if (!wGraph->nodes[i].isActive) 
            continue;

        unsigned int dist;

        GLdouble gx=wGraph->nodes[i].x, gy=wGraph->nodes[i].y, gz=wGraph->nodes[i].z;

        // Convert from 3d pixels to screen coords
        GLdouble sx, sy, sz;
        if(gluProject(gx, gy, gz, modelmatrix, projmatrix, viewport, &sx, &sy, &sz) == GL_TRUE)
        {
            long dx = x - (long)sx;
            long dy = convy - (long)sy;
            dist = (dx*dx) + (dy*dy);

            if (dist<min_dist)
                min_dist=dist;
        }
        else
        {
            LOG_ERROR("getNodeId: Unable to compute screen coords from gl coords."); 
            continue;
        }

        LOG_DEBUG("getNodeId: gx, gy, gz: " << gx << ", " << gy << ", " << gz); 
        LOG_DEBUG("getNodeId: sx, sy, sz: " << sx << ", " << sy << ", " << sz); 
        LOG_DEBUG("getNodeId: x, convy, y, dist, min_dist: " << x << ", " << convy << ", " << y << ", " << dist << ", " << min_dist); 

        dist=dist>r2 ? dist-r2 : 0; 
        if (dist < min_dist)
            found=(unsigned int)wGraph->nodes[i].nodeId.to_v4().to_ulong();
    }
    if (min_dist < ULONG_MAX)
        retVal=found;

    if (retVal)
        LOG_INFO("getNodeId: Found node " << retVal << " at double click location " << x << ", " << y); 
    else
        LOG_INFO("getNodeId: Found no node at double click location " << x << ", " << y); 

    TRACE_EXIT_RET(retVal);
    return retVal;
}

void manetGLView::showNodeSelectedForGraph(unsigned int, bool)
{
    TRACE_ENTER();
    // GTL - doesn't do anything now. But could mark the node as being displayed in the 
    // graph dialogs somehow. Add a little "G" or a little graph representation
    // above the node? 
    TRACE_EXIT();
}

void manetGLView::toggleNodeSelectedForGraph(unsigned int)
{
    TRACE_ENTER();
    // GTL - doesn't do anything now. But could mark the node as being displayed in the 
    // graph dialogs somehow. Add a little "G" or a little graph representation
    // above the node? 
    TRACE_EXIT();
}

void manetGLView::streamFilteringEnabled(bool isEnabled) 
{
    TRACE_ENTER();
    LOG_DEBUG("Change in stream filtering. Filtering is now " << (isEnabled?"enabled":"disabled") << ".");

    messageStreamFiltering=isEnabled;

    if (messageStream && messageStream->connected()) { 
        for (size_t l=0; l<wGraph->numValidLayers; l++) { 
            messageStream->enableFiltering(isEnabled); 
            MessageStreamFilterPtr f(new MessageStreamFilter);
            f->setLayer(wGraph->layers[l].layerName); 
            // If enabling filtering turn on all currently active filters
            // else remove them all
            if (isEnabled && wGraph->layers[l].isActive) 
                messageStream->addMessageFilter(f);
            else
                messageStream->removeMessageFilter(f);
        }
    }
    TRACE_EXIT(); 
}

void manetGLView::scrollingGraphActivated(QString graphName)
{
    TRACE_ENTER();
    LOG_DEBUG("Got activate signal for graph name \"" << graphName.toStdString() << "\""); 
    TRACE_EXIT(); 
}

void manetGLView::mousePressEvent(QMouseEvent *event)
{
    TRACE_ENTER();
    lastPos = event->pos();
    update();
    TRACE_EXIT();
}

void manetGLView::fitToWindow()
{
    TRACE_ENTER();
    resetPosition();
    // GTL - could use existing key handler here...
    autoCenterNodesFlag=true;
    scaleAndShiftToCenter(ScaleAndShiftUpdateAlways);
    autoCenterNodesFlag=false;
    TRACE_EXIT();
}

void manetGLView::mouseMoveEvent(QMouseEvent *event)
{
    TRACE_ENTER();

    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    Qt::MouseButtons buttons = event->buttons();
    Qt::KeyboardModifiers mods=event->modifiers();

    if ((buttons & Qt::LeftButton) && (buttons & Qt::RightButton))
    {
        if (dy>0)
            zoomIn();
        if (dy<0)
            zoomOut();
        updateGL();
    }
    else if ((buttons & Qt::LeftButton) && !(buttons & Qt::RightButton))
    {
        if (mods & Qt::ShiftModifier)
        {
            shiftBackgroundCenterLeft(dx);
            shiftBackgroundCenterUp(-dy);
        }
        else
        {
            shiftCenterLeft(dx);
            shiftCenterUp(dy);
        }
        updateGL();
    } 
    else if (!(buttons & Qt::LeftButton) && (buttons & Qt::RightButton))
    {
        rotateX(dy);
        rotateY(dx);
        updateGL();
    }
    lastPos = event->pos();

    update();

    TRACE_EXIT();
}

void manetGLView::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0)
        zoomIn();
    else
        zoomOut();
    update();
}

void manetGLView::layerToggle(const QString &layerName, const bool turnOn)
{
    TRACE_ENTER();

    GUILayer name=layerName.toStdString();
    size_t layerIndex=wGraph->name2LayerIndex(name);
    wGraph->layers[layerIndex].isActive=turnOn;
    LOG_DEBUG("Turned layer " << name << " (" << layerIndex << ") " << (turnOn ? "on" : "off")); 
    if (messageStreamFiltering && messageStream && messageStream->connected()) {
        // Tell the watcherd that we want/don't want messages for this layer.
        MessageStreamFilterPtr f(new MessageStreamFilter);
        f->setLayer(name); 
        if (turnOn)
            messageStream->addMessageFilter(f);
        else
            messageStream->removeMessageFilter(f);
    }

    TRACE_EXIT();
}

void manetGLView::toggleMonochrome(bool isOn)
{
    TRACE_ENTER();
    static GLfloat previousBackgroundColor[4]={0.0, 0.0, 0.0, 0.0}; 
    monochromeMode=isOn;
    emit monochromeToggled(isOn); 
    if (isOn) {
        glGetFloatv(GL_COLOR_CLEAR_VALUE, previousBackgroundColor);
        glClearColor(1, 1, 1, 1.0);
    }
    else {
        glClearColor(previousBackgroundColor[0], previousBackgroundColor[1], previousBackgroundColor[2], previousBackgroundColor[3]); 
    }
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleThreeDView(bool isOn)
{
    TRACE_ENTER();
    threeDView=isOn;
    emit threeDViewToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleBackgroundImage(bool isOn)
{
    TRACE_ENTER();
    LOG_DEBUG("Turning background image " << (isOn==true?"on":"off")); 
    backgroundImage=isOn; 
    emit backgroundImageToggled(isOn); 
    updateGL();
    TRACE_EXIT();
}
void manetGLView::toggleLoopPlayback(bool isOn) 
{
    TRACE_ENTER();
    LOG_DEBUG("Toggling looped playback: " << (isOn==true?"on":"off"));
    autorewind=isOn;
    emit loopPlaybackToggled(isOn);
    TRACE_EXIT();
}

void manetGLView::clearAll()
{
    TRACE_ENTER();
    wGraph->clear();
    TRACE_EXIT();
}

void manetGLView::clearAllEdges()
{
    TRACE_ENTER();
    for (size_t l=0; l<wGraph->numValidLayers; l++) {
        for (size_t n=0; n<wGraph->numValidNodes; n++) { 
            std::fill_n(wGraph->layers[l].edges[n], maxNodes, 0); 
            std::fill_n(wGraph->layers[l].edgeExpirations[n], maxNodes, watcher::Infinity); 
        }
    }
    emit edgesCleared(); 
    TRACE_EXIT();
}
void manetGLView::clearAllLabels()
{
    TRACE_ENTER();
    // GTL do this
    emit labelsCleared();
    TRACE_EXIT();
}

// Should be called after a glTranslate()
void manetGLView::handleSpin(int threeD, const NodeDisplayInfo &ndi)
{
    if (ndi.spin)
    {
        if (threeD)
        {
            glRotatef(ndi.spinRotation_x, 1.0f, 0.0f, 0.0f);
            glRotatef(ndi.spinRotation_y, 0.0f, 1.0f, 0.0f);
        }
        glRotatef(ndi.spinRotation_z, 0.0f, 0.0f, 1.0f);
    }
}

// Should be called after a glTranslate()
void manetGLView::handleSize(const NodeDisplayInfo &ndi)
{
    glScalef(ndi.size, ndi.size, ndi.size);
}

void manetGLView::handleProperties(const NodeDisplayInfo &ndi)
{
    const GLfloat black[]={0.0,0.0,0.0,1.0};
    BOOST_FOREACH(const NodePropertiesMessage::NodeProperty &p, ndi.nodeProperties) {
        switch(p) { 
            case NodePropertiesMessage::NOPROPERTY: 
                break;

            case NodePropertiesMessage::ROOT:         
                if (isActive(HIERARCHY_LAYER)) { 
                    if (monochromeMode) 
                        glColor4fv(black); 
                    else 
                        glColor4ub(hierarchyRingColor.r, hierarchyRingColor.g, hierarchyRingColor.b, hierarchyRingColor.a);
                    drawTorus(1, 13);
                    drawTorus(1, 10);
                    drawTorus(1, 7); 
                }
                break;
            case NodePropertiesMessage::REGIONAL:     
                if (isActive(HIERARCHY_LAYER)) { 
                    if (monochromeMode) 
                        glColor4fv(black); 
                    else 
                        glColor4ub(hierarchyRingColor.r, hierarchyRingColor.g, hierarchyRingColor.b, hierarchyRingColor.a);
                    drawTorus(1, 10);
                    drawTorus(1, 7);
                }
                break;
            case NodePropertiesMessage::NEIGHBORHOOD: 
                if (isActive(HIERARCHY_LAYER)) {
                    if (monochromeMode) 
                        glColor4fv(black); 
                    else 
                        glColor4ub(hierarchyRingColor.r, hierarchyRingColor.g, hierarchyRingColor.b, hierarchyRingColor.a);
                    drawTorus(1, 7);
                }
                break;
            case NodePropertiesMessage::LEAFNODE:
                break;
            case NodePropertiesMessage::ATTACKER: 
                break;
            case NodePropertiesMessage::VICTIM: 
                break;
        }
    }
}

void manetGLView::drawWireframeSphere(GLdouble radius)
{
    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutWireSphere(radius, 10, 10);
        glPopAttrib();
    }
    else
    {
        GLUquadric* q=NULL;
        q=gluNewQuadric();
        gluDisk(q,radius-1,radius,36,1);
        gluDeleteQuadric(q);
    }
}

void manetGLView::drawPyramid(GLdouble radius)
{
    // fprintf(stdout, "Drawing triangle with \"radius\" : %f. x/y offset is %f\n", radius, offset); 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glScalef(radius*1.2, radius*1.2, radius*1.2);

        glBegin(GL_TRIANGLES);
        {
            // Front
            glNormal3f( 0.0f, 0.0f, 1.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
            // Right
            glNormal3f( 1.0f, 0.0f, 0.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f,-1.0f);
            // Back
            glNormal3f( 0.0f, 0.0f,-1.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f( 1.0f,-1.0f,-1.0f);
            glVertex3f(-1.0f,-1.0f,-1.0f);
            // Left
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glVertex3f( 0.0f, 1.0f, 0.0f);
            glVertex3f(-1.0f,-1.0f,-1.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
        }
        glEnd();

        glBegin(GL_QUADS);
        {
            // Bottom
            glNormal3f( 0.0f,-1.0f, 0.0f);
            glVertex3f(-1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f, 1.0f);
            glVertex3f( 1.0f,-1.0f,-1.0f);
            glVertex3f(-1.0f,-1.0f,-1.0f);
        }
        glEnd();

        glPopAttrib();
    }
    else
    {
        GLfloat offset=2.0*radius*sin(M_PI_4);  // by law of sines

        glPushAttrib(GL_LINE_WIDTH);
        glLineWidth(2.0); 
        glNormal3f( 0.0f, 0.0f, 1.0f); 
        glBegin(GL_LINE_LOOP); 
        {
            glVertex2f(-offset, -offset);
            glVertex2f( offset, -offset); 
            glVertex2f(0, radius); 
        }
        glEnd();
        glPopAttrib();
    }
}

void manetGLView::drawCube(GLdouble radius)
{
    GLfloat widthScaled=radius; 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        // I had this "easy" call to glutDrawSolidCube, but the shadows did not look as good as when I set the 
        // normal myself.
        //  glutSolidCube(widthScaled*2); 
        glScalef(widthScaled*1.2, widthScaled*1.2, widthScaled*1.2);
        glBegin(GL_QUADS);
        // Front Face
        glNormal3f( 0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 1 (Front)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 2 (Front)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Front)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 4 (Front)
        // Back Face
        glNormal3f( 0.0f, 0.0f,-1.0f);                  // Normal Pointing Away From Viewer
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Back)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 2 (Back)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 3 (Back)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 4 (Back)
        // Top Face
        glNormal3f( 0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 1 (Top)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 2 (Top)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Top)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 4 (Top)
        // Bottom Face
        glNormal3f( 0.0f,-1.0f, 0.0f);                  // Normal Pointing Down
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Bottom)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 2 (Bottom)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 3 (Bottom)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 4 (Bottom)
        // Right face
        glNormal3f( 1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 1 (Right)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 2 (Right)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Right)
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 4 (Right)
        // Left Face
        glNormal3f(-1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
        /* glTexCoord2f(0.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Left)
        /* glTexCoord2f(1.0f, 0.0f); */ glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 2 (Left)
        /* glTexCoord2f(1.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 3 (Left)
        /* glTexCoord2f(0.0f, 1.0f); */ glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 4 (Left)
        glEnd();                                // Done Drawing Quads

        glPopAttrib();
    }
    else
    {
        GLfloat offset=widthScaled;
        glLineWidth(2.0); 
        glBegin(GL_LINE_LOOP);
        glVertex2f(-offset, -offset);
        glVertex2f( offset, -offset);
        glVertex2f( offset,  offset);
        glVertex2f(-offset,  offset); 
        glEnd();
        glLineWidth(1.0); 
    }
}

void manetGLView::drawTeapot(GLdouble radius)
{
    GLfloat widthScaled=radius; 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidTeapot(widthScaled*2); 
        glPopAttrib();
    }
    else
    {
        // A flat teapot is just a square
        GLfloat offset=widthScaled;
        glLineWidth(2.0); 
        glBegin(GL_LINE_LOOP);
        glVertex2f(-offset, -offset);
        glVertex2f( offset, -offset);
        glVertex2f( offset,  offset);
        glVertex2f(-offset,  offset); 
        glEnd();
        glLineWidth(1.0); 
    }
}

void manetGLView::drawTorus(GLdouble inner, GLdouble outer)
{
    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidTorus(inner, outer, 15, 15);  
        glPopAttrib();
    }
    else
    {
        GLUquadric* q=NULL;
        q=gluNewQuadric();
        gluDisk(q, outer-inner, outer, 36, 1); 
        gluDeleteQuadric(q);
    }
}

void manetGLView::drawSphere(GLdouble radius)
{
    if (threeDView)
    {
        GLUquadricObj *quadric=gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        glPushMatrix();
        gluSphere(quadric, radius, 15, 15);
        glPopMatrix();
        gluDeleteQuadric(quadric);
    }
    else {
        GLUquadric* q=NULL;
        q=gluNewQuadric();
        gluDisk(q,radius-1,radius,36,1);
        gluDeleteQuadric(q);
    }
}

void manetGLView::drawCircle(GLdouble radius)
{
    glPushMatrix();
    GLUquadric* q=NULL;
    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);
    glPopMatrix();
}

void manetGLView::drawFrownyCircle(GLdouble /* radius */)
{ 
    static GLfloat const dead[]={1.0,0.0,0.0,1.0}; 

    glColor4fv(dead);

    // draw outsize circle
    GLUquadric* q=NULL;
    q=gluNewQuadric();
    gluDisk(q, 6, 7, 36, 1);
    gluDeleteQuadric(q);

    // draw eyes and mouth
    glBegin(GL_LINES); 
    glVertex3f(-4.0,+3.0,0); 
    glVertex3f(-2.0,+1.0,0); 
    glVertex3f(-2.0,+3.0,0); 
    glVertex3f(-4.0,+1.0,0); 

    glVertex3f(+4.0,+3.0,0); 
    glVertex3f(+2.0,+1.0,0); 
    glVertex3f(+2.0,+3.0,0); 
    glVertex3f(+4.0,+1.0,0); 

    glVertex3f(-3.0,-3.0,0); 
    glVertex3f(-2.0,-2.0,0); 
    glVertex3f(-2.0,-2.0,0); 
    glVertex3f(+2.0,-2.0,0); 
    glVertex3f(+2.0,-2.0,0); 
    glVertex3f(+3.0,-3.0,0); 

    glEnd(); 
    return; 
} /* drawFrownyCircle */ 

void manetGLView::saveConfiguration()
{
    TRACE_ENTER();
    LOG_DEBUG("Got close event, saving modified configuration"); 

    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock(); 
    Setting &root=cfg.getRoot();

    try {

        struct 
        {
            const char *prop;
            bool boolVal;
        } boolConfigs[] =
        {
            { "nodes3d",        threeDView },
            { "monochrome",     monochromeMode },
            { "displayBackgroundImage", backgroundImage },
            { "showVerboseStatusString", showVerboseStatusString }, 
            { "showWallTime", showWallTimeinStatusString }, 
            { "showPlaybackTime", showPlaybackTimeInStatusString },
            { "showPlaybackRange", showPlaybackRangeString },
            { "showDebugInfo", showDebugInfo },
            { "autorewind", autorewind },
            { "messageStreamFiltering", messageStreamFiltering }
        };

        for (size_t i = 0; i < sizeof(boolConfigs)/sizeof(boolConfigs[0]); i++) {
            if (!root.exists(boolConfigs[i].prop))
                root.add(boolConfigs[i].prop, Setting::TypeBoolean)=boolConfigs[i].boolVal;
            else
                root[boolConfigs[i].prop]=boolConfigs[i].boolVal;
        }

        struct 
        {
            const char *prop; 
            float *val; 
        } floatVals[] = 
        {
            { "scaleText", &scaleText }, 
            { "scaleLine", &scaleLine }, 
            { "layerPadding", &layerPadding }, 
            { "gpsScale", &gpsScale }, 
            { "antennaRadius", &antennaRadius },
            { "ghostLayerTransparency", &ghostLayerTransparency }
        }; 
        for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++) {
            if (!root.exists(floatVals[i].prop))
                root.add(floatVals[i].prop, Setting::TypeFloat)=*floatVals[i].val;
            else
                root[floatVals[i].prop]=*floatVals[i].val;
        }

        string hrc(hierarchyRingColor.toString());
        struct 
        {
            const char *prop; 
            string *val; 
        } strVals[] = 
        {
            { "statusFontName", &statusFontName },
            { "hierarchyRingColor", &hrc }
        }; 
        for (size_t i=0; i<sizeof(strVals)/sizeof(strVals[0]); i++) {
            if (!root.exists(strVals[i].prop))  
                root.add(strVals[i].prop, Setting::TypeString)=*strVals[i].val;
            else
                root[strVals[i].prop]=*strVals[i].val;
        }

        struct 
        {
            const char *prop; 
            int *val; 
        } intVals[] = 
        {
            { "statusFontPointSize", &statusFontPointSize },
            { "maxNodes", (int*)&maxNodes },
            { "maxLayers", (int*)&maxLayers }
        }; 
        for (size_t i=0; i<sizeof(intVals)/sizeof(intVals[0]); i++) {
            if (!root.exists(intVals[i].prop))
                root.add(intVals[i].prop, Setting::TypeInt)=*intVals[i].val;
            else
                root[intVals[i].prop]=*intVals[i].val;
        }

        struct 
        {
            const char *prop; 
            long long int *val; 
        } longlongIntVals[] = 
        {
            { "playbackStartTime", &playbackStartTime } 
        }; 
        for (size_t i=0; i<sizeof(longlongIntVals)/sizeof(longlongIntVals[0]); i++) {
            if (!root.exists(longlongIntVals[i].prop)) 
                root.add(longlongIntVals[i].prop, Setting::TypeInt64)=(int)(*longlongIntVals[i].val); 
            else
                root[longlongIntVals[i].prop]=*longlongIntVals[i].val;
        }

        string prop="viewPoint";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &vp=cfg.lookup(prop); 

        struct 
        {
            const char *type;
            float *data[3];
        } viewPoints[] =
        {
            { "angle", { &manetAdj.angleX, &manetAdj.angleY, &manetAdj.angleZ }},
            { "scale", { &manetAdj.scaleX, &manetAdj.scaleY, &manetAdj.scaleZ }},
            { "shift", { &manetAdj.shiftX, &manetAdj.shiftY, &manetAdj.shiftZ }}
        };
        for (size_t i=0; i<sizeof(viewPoints)/sizeof(viewPoints[0]);i++) {
            if (!vp.exists(viewPoints[i].type)) {
                vp.add(viewPoints[i].type, libconfig::Setting::TypeArray);
                for (size_t j=0; j<sizeof(viewPoints[i].data)/sizeof(viewPoints[i].data[0]); j++)
                    vp[viewPoints[i].type].add(libconfig::Setting::TypeFloat);
            }
        }
        root["viewPoint"]["angle"][0]=fpclassify(manetAdj.angleX)==FP_NAN ? 0.0 : manetAdj.angleX;
        root["viewPoint"]["angle"][1]=fpclassify(manetAdj.angleY)==FP_NAN ? 0.0 : manetAdj.angleY;
        root["viewPoint"]["angle"][2]=fpclassify(manetAdj.angleZ)==FP_NAN ? 0.0 : manetAdj.angleZ;
        root["viewPoint"]["scale"][0]=fpclassify(manetAdj.scaleX)==FP_NAN ? 0.0 : manetAdj.scaleX;
        root["viewPoint"]["scale"][1]=fpclassify(manetAdj.scaleY)==FP_NAN ? 0.0 : manetAdj.scaleY;
        root["viewPoint"]["scale"][2]=fpclassify(manetAdj.scaleZ)==FP_NAN ? 0.0 : manetAdj.scaleZ;
        root["viewPoint"]["shift"][0]=fpclassify(manetAdj.shiftX)==FP_NAN ? 0.0 : manetAdj.shiftX;
        root["viewPoint"]["shift"][1]=fpclassify(manetAdj.shiftY)==FP_NAN ? 0.0 : manetAdj.shiftY;
        root["viewPoint"]["shift"][2]=fpclassify(manetAdj.shiftZ)==FP_NAN ? 0.0 : manetAdj.shiftZ;

        BackgroundImage &bg=BackgroundImage::getInstance();
        float bgfloatVals[5];
        bg.getDrawingCoords(bgfloatVals[0], bgfloatVals[1], bgfloatVals[2], bgfloatVals[3], bgfloatVals[4]); 

        prop="backgroundImage";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &bgset=cfg.lookup(prop);

        prop="imageFile"; 
        if (!bgset.exists(prop))
            bgset.add(prop, libconfig::Setting::TypeString);
        prop="coordinates";
        if (!bgset.exists(prop)) { 
            bgset.add(prop, libconfig::Setting::TypeArray);
            // for (size_t i=0; i<sizeof(bgfloatVals)/sizeof(bgfloatVals[0]); i++)
            //     bgset[prop].add(libconfig::Setting::TypeFloat);                 // I dislike libconfig++
        }

        root["backgroundImage"]["coordinates"][0]=bgfloatVals[0];
        root["backgroundImage"]["coordinates"][1]=bgfloatVals[1];
        root["backgroundImage"]["coordinates"][2]=bgfloatVals[2];
        root["backgroundImage"]["coordinates"][3]=bgfloatVals[3];
        root["backgroundImage"]["coordinates"][4]=bgfloatVals[4];

        prop="backgroundColor";
        if (!root.exists(prop))
            root.add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &bgColSet=cfg.lookup(prop);

        struct 
        {
            const char *name;
            float val;
        } bgColors[] = 
        {
            { "r", 0.0 }, 
            { "g", 0.0 }, 
            { "b", 0.0 }, 
            { "a", 255.0 }
        };
        for (size_t i=0; i<sizeof(bgColors)/sizeof(bgColors[0]);i++)
            if (!bgColSet.exists(bgColors[i].name))
                bgColSet.add(bgColors[i].name, libconfig::Setting::TypeFloat)=bgColors[i].val;

        GLfloat cols[4]={0.0, 0.0, 0.0, 0.0}; 
        glGetFloatv(GL_COLOR_CLEAR_VALUE, cols);
        root["backgroundColor"]["r"]=cols[0];
        root["backgroundColor"]["g"]=cols[1];
        root["backgroundColor"]["b"]=cols[2];
        root["backgroundColor"]["a"]=cols[3];

    }
    catch (const libconfig::SettingException &e) {
        LOG_ERROR("Error loading configuration at " << e.getPath() << ": " << e.what());
    }

    SingletonConfig::unlock();

    if (wGraph) 
        wGraph->saveConfiguration(); 

    SingletonConfig::saveConfig();

    TRACE_EXIT();
}

void manetGLView::pausePlayback()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    messageStream->stopStream(); 
    TRACE_EXIT();
}

void manetGLView::normalPlayback()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    playbackSetSpeed(1.0);
    messageStream->startStream(); 
    TRACE_EXIT();
}

void manetGLView::reversePlayback()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    pausePlayback(); 
    if (streamRate != 0.0) {
        if (streamRate < 0.0)
            playbackSetSpeed(-abs(streamRate*2));
        else
            playbackSetSpeed(-abs(streamRate));
        messageStream->startStream(); 
    }
    TRACE_EXIT();
}

void manetGLView::forwardPlayback()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    pausePlayback(); 
    if (streamRate != 0.0) {
        if (streamRate > 0.0)
            playbackSetSpeed(abs(streamRate*2));
        else
            playbackSetSpeed(abs(streamRate));
        messageStream->startStream(); 
    }
    TRACE_EXIT();
}

void manetGLView::rewindToStartOfPlayback()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    pausePlayback(); 
    messageStream->setStreamTimeStart(SeekMessage::epoch); 
    if (streamRate < 0.0)
        normalPlayback();
    else
        messageStream->startStream(); 
    TRACE_EXIT();
}

void manetGLView::forwardToEndOfPlayback()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    pausePlayback();
    messageStream->setStreamTimeStart(SeekMessage::eof); 
    normalPlayback();
    TRACE_EXIT();
}

/** Called when a message from the watcher daemon is received notifying us that
 * the stream rate has changed.
 */
void manetGLView::changeSpeed(double x)
{
    if ((streamRate > 0.0 && x < 0.0) || (streamRate < 0.0 && x > 0.0)) {
        wGraph->setTimeDirectionForward(x > 0.0);
    }
    streamRate = x;
    emit streamRateSet(streamRate); // update the gui's display
}

void manetGLView::playbackSetSpeed(double x)
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    messageStream->setStreamRate(x);
    TRACE_EXIT();
}

void manetGLView::listStreams()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    if (!streamsDialog) {
        streamsDialog = new WatcherStreamListDialog;
        connect(streamsDialog, SIGNAL(streamChanged(unsigned long)), this, SLOT(selectStream(unsigned long)));
        connect(streamsDialog->refreshButton, SIGNAL(clicked()), this, SLOT(listStreams()));
    }
    streamsDialog->treeWidget->clear();
    streamsDialog->show();

    messageStream->listStreams();
    TRACE_EXIT();
}

void manetGLView::selectStream(unsigned long uid)
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    messageStream->subscribeToStream(uid);
    messageStream->getMessageTimeRange(); // get info for new stream
    TRACE_EXIT();
}

/** Blocks while user is entering the new stream description.  */
void manetGLView::spawnStreamDescription()
{
    TRACE_ENTER();
    if (!messageStream || !messageStream->connected()) {
        TRACE_EXIT();
        return;
    }
    bool ok;
    QString text = QInputDialog::getText(this, tr("Set Stream Description"), tr("Description:"), QLineEdit::Normal, QString(), &ok);
    if (ok && !text.isEmpty())
        messageStream->setDescription(text.toStdString());
    TRACE_EXIT();
}
