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
#include <values.h>  // DBL_MAX
#include <GL/glut.h>

#include <libwatcher/watcherGraph.h>
#include <libwatcher/seekWatcherMessage.h>  // for epoch, eof
#include <libwatcher/playbackTimeRange.h>  // for epoch, eof

#include "manetglview.h"
#include "watcherScrollingGraphControl.h"
#include "singletonConfig.h"
#include "backgroundImage.h"

INIT_LOGGER(manetGLView, "manetGLView");

using namespace watcher;
using namespace watcher::event;
using namespace std;
using namespace libconfig;
using namespace boost;
using namespace boost::graph;
using namespace boost::date_time;
using namespace boost::posix_time;

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
    bool includeAntenna = isActive(ANTENNARADIUS_LAYER); 
    // bool includeHierarchy = isActive(HIERARCHY_LAYER); 

    // find drawing extents
    WatcherGraph::vertexIterator vi, vend;
    for(tie(vi, vend)=vertices(wGraph.theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &n=wGraph.theGraph[*vi]; 
        GLdouble x, y, z; 
        gps2openGLPixels(n.gpsData->dataFormat, n.gpsData->x, n.gpsData->y, n.gpsData->z, x, y, z); 

        double r = 0;
        if(includeAntenna)
            r = antennaRadius; 

        // if(includeHierarchy)
        // {
        //     double hr = HIERARCHY_RADIUS(m->nlist[i].level);
        //     if(r < hr)
        //     {
        //         r = hr;
        //     }
        // }
        {
            double nodeXMin = x - r;
            double nodeXMax = x + r;
            double nodeYMin = y - r;
            double nodeYMax = y + r;
            if(nodeXMin < xMin) xMin = nodeXMin;
            if(nodeXMax > xMax) xMax = nodeXMax;
            if(nodeYMin < yMin) yMin = nodeYMin;
            if(nodeYMax > yMax) yMax = nodeYMax;
        }
        if(z < zMin)
            zMin = z;
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

void manetGLView::gps2openGLPixels(const GPSMessage::DataFormat &format, const double inx, const double iny, const double inz, GLdouble &x, GLdouble &y, GLdouble &z) 
{
    TRACE_ENTER();

    // GTL - this should be done when the message is recv'd - not every time we need to 
    // compute the points - which is a whole hell of a lot of times.

    if (format==GPSMessage::UTM)
    {
        //
        // There is no UTM zone information in the UTM GPS packet, so we assume all data is in a single
        // zone. Because of this, no attempt is made to place the nodes in the correct positions on the 
        // planet surface. We just use the "lat" "long" data as pure x and y coords in a plane, offset
        // by the first coord we get. (Nodes are all centered around 0,0 where, 0,0 is defined 
        // by the first coord we receive. 
        //
        if (iny < 91 && inx > 0) 
            LOG_WARN("Received GPS data that looks like lat/long in degrees, but GPS data format mode is set to UTM in cfg file."); 

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
            LOG_WARN("Received GPS data that may be UTM (long>180), but GPS data format mode is set to lat/long degrees in cfg file."); 

        x=inx*gpsScale;
        y=iny*gpsScale;
        z=inz-20;

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
    TRACE_EXIT();
}

manetGLView::manetGLView(QWidget *parent) : 
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    streamRate(1.0),
    playbackPaused(false),
    currentMessageTimestamp(0),
    playbackRangeEnd(0),
    playbackRangeStart(0),
    showWallTimeinStatusString(true),
    showPlaybackTimeInStatusString(true),
    showPlaybackRangeString(true),
    showVerboseStatusString(false),
    statusFontPointSize(10),
    statusFontName("Helvetica"),
    autoCenterNodesFlag(false)
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
    TRACE_EXIT();
}

manetGLView::~manetGLView()
{
    TRACE_ENTER();
    for (vector<StringIndexedMenuItem*>::iterator i=layerMenuItems.begin(); i!=layerMenuItems.end(); ++i)
        delete *i;
    TRACE_EXIT();
}

void manetGLView::addLayerMenuItem(const GUILayer &layer, bool active)
{
    TRACE_ENTER();
    LayerListItemPtr item(new LayerListItem);
    item->layer=layer;
    item->active=active;
    knownLayers.push_back(item);

    if (layerMenu)
    {
        QAction *action=new QAction(QString::fromStdString(layer), (QObject*)this);
        action->setCheckable(true);

        StringIndexedMenuItem *item = new StringIndexedMenuItem(QString::fromStdString(layer)); 
        connect(action, SIGNAL(triggered(bool)), item, SLOT(showMenuItem(bool)));
        connect(item, SIGNAL(showMenuItem(QString, bool)), this, SLOT(layerToggle(QString, bool)));
        connect(this, SIGNAL(layerToggled(QString, bool)), item, SLOT(setChecked(QString, bool)));
        connect(item, SIGNAL(setChecked(bool)), action, SLOT(setChecked(bool)));

        layerMenuItems.push_back(item);     // We have to keep 'item' alive somewhere. 

        layerMenu->addAction(action); 
    }

    // Could use a few more type conversions for string here...
    emit layerToggled(QString::fromStdString(string(item->layer)), item->active);

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

    //
    // Check configuration for GUI settings.
    //
    Config &cfg=SingletonConfig::instance();
    SingletonConfig::lock();
    libconfig::Setting &root=cfg.getRoot();

    string prop="server";
    if (!root.lookupValue(prop, serverName))
    {
        LOG_FATAL("Please specify the server name in the cfg file");
        LOG_FATAL("I set the default to localhost, but that may not be what you want."); 
        root.add(prop, Setting::TypeString)="localhost"; 
        TRACE_EXIT_RET_BOOL(false); 
        return false;
    }

    prop="layers";
    if (!root.exists(prop))
        root.add(prop, libconfig::Setting::TypeGroup);

    libconfig::Setting &layers=cfg.lookup(prop);

    // We cheat a little here and always make sure there's an antenna radius "layer"
    if (!layers.exists(ANTENNARADIUS_LAYER))
        layers.add(ANTENNARADIUS_LAYER, Setting::TypeBoolean)=false;

    LOG_INFO("Reading layer states from cfg file");
    int layerNum=layers.getLength();
    for (int i=0; i<layerNum; i++)
    {
        string name(layers[i].getName());
        LOG_DEBUG("Reading layer menu config for " << name); 
        bool val=true;
        if (!layers.lookupValue(name, val))
            layers.add(name, Setting::TypeBoolean)=val;  // Shouldn't happen unless misformed cfg file. 
        addLayerMenuItem(name, val);
    }

    // Force a PHYSICAL_LAYER to at least be an option in the menu.
    bool foundPhy=false;
    BOOST_FOREACH(LayerListItemPtr &llip, knownLayers)
        if (llip->layer==PHYSICAL_LAYER)
            foundPhy=true;
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
        { "showPlaybackRange", true, &showPlaybackRangeString }
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
    emit checkPlaybackTime(showPlaybackTimeInStatusString);
    emit checkPlaybackRange(showPlaybackRangeString);
    emit checkWallTime(showWallTimeinStatusString);

    struct 
    {
        const char *prop; 
        const char *def; 
        string *val; 
    } strVals[] = 
    {
        { "statusFontName", "Helvetica", &statusFontName } 
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
            root.add(prop, libconfig::Setting::TypeFloat)=floatVal;
        *floatVals[i].val=floatVal; 
        LOG_DEBUG("Setting " << floatVals[i].prop << " to " << floatVal);
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
        char *ext=rindex(strVal.data(), '.');
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

    SingletonConfig::unlock();

    // 
    // Set up timer callbacks.
    //
    QTimer *checkIOTimer = new QTimer(this);
    QObject::connect(checkIOTimer, SIGNAL(timeout()), this, SLOT(checkIO()));
    checkIOTimer->start(20);

    QTimer *watcherIdleTimer = new QTimer(this);
    QObject::connect(watcherIdleTimer, SIGNAL(timeout()), this, SLOT(watcherIdle()));
    watcherIdleTimer->start(30); 

    TRACE_EXIT();
    return true;
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

void manetGLView::initializeGL()
{
    TRACE_ENTER();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE); 

    glShadeModel(GL_SMOOTH); 

    // LIGHT0
    GLfloat posLight0[]={ 50.0f, 50.0f, 000.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posLight0);

    GLfloat ambLight0[]={ 0.0, 0.0, 0.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambLight0); 
    
    GLfloat specLight0[]={ 1.0, 1.0, 1.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_SPECULAR, specLight0);
    
    GLfloat diffLight0[]= { 1.0, 1.0, 1.0, 1.0 }; 
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffLight0);

    GLfloat ambLightDef[]={0.1,0.1,0.1,1.0}; // OPenGL's default is: 0.2,0.2,0.2,1
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambLightDef); 
    
    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT0); 

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    TRACE_EXIT();
}

void manetGLView::checkIO()
{
    TRACE_ENTER();

    if (!messageStream)
    {
        messageStream=MessageStream::createNewMessageStream(serverName); 
        messageStream->setStreamTimeStart(SeekMessage::eof);
        messageStream->startStream();
        messageStream->getMessageTimeRange();
    }

    while(messageStream && messageStream->isStreamReadable())
    {
        MessagePtr message;
        messageStream->getNextMessage(message);
        LOG_DEBUG("Got message: " << *message);
        wGraph.updateGraph(message);

        // DataPoint data is handled directly by the scrolling graph thing.
        if (message->type==DATA_POINT_MESSAGE_TYPE)
        {
            WatcherScrollingGraphControl *sgc=WatcherScrollingGraphControl::getWatcherScrollingGraphControl();
            sgc->handleDataPointMessage(dynamic_pointer_cast<DataPointMessage>(message));
        }
        else if (message->type==PLAYBACK_TIME_RANGE_MESSAGE_TYPE)
        {
            PlaybackTimeRangeMessagePtr trm(dynamic_pointer_cast<PlaybackTimeRangeMessage>(message));
            playbackRangeEnd=trm->max_;
            playbackRangeStart=trm->min_;
            // updatePlaybackSliderRange();
        }

        currentMessageTimestamp=message->timestamp;
        // if (currentMessageTimestamp>playbackRangeEnd)
            updatePlaybackSliderRange();

        // Really need to make layers a member of a base class...
        GUILayer layer;
        switch (message->type)
        {
            case LABEL_MESSAGE_TYPE: layer=(dynamic_pointer_cast<LabelMessage>(message))->layer; break;
            case EDGE_MESSAGE_TYPE: layer=(dynamic_pointer_cast<EdgeMessage>(message))->layer; break;
            case COLOR_MESSAGE_TYPE: layer=(dynamic_pointer_cast<ColorMessage>(message))->layer; break;
            case CONNECTIVITY_MESSAGE_TYPE: layer=(dynamic_pointer_cast<ConnectivityMessage>(message))->layer; break;
            default: break;
        }

        if (!layer.empty())
        {
            LOG_DEBUG("Seeing if " << layer << " is known to us or not."); 
            bool found=false;
            BOOST_FOREACH(LayerListItemPtr &llip, knownLayers)
            {
                if (llip->layer==layer)
                {
                    found=true;
                    break;
                }
            }
            if(!found)
            {
                LOG_DEBUG("Adding new layer to known layers: " << layer); 
                addLayerMenuItem(layer, true); 
            }
        }
    }

    updateGL();  // redraw
    TRACE_EXIT();
}

void manetGLView::updatePlaybackSliderRange()
{
    TRACE_ENTER();
    if (playbackSlider)
    {
        if (currentMessageTimestamp>playbackRangeEnd)
            playbackRangeEnd=currentMessageTimestamp;
        playbackSlider->setRange(0, (playbackRangeEnd-playbackRangeStart)/1000);
        playbackSlider->setValue((currentMessageTimestamp-playbackRangeStart)/1000); 
    }
    TRACE_EXIT();
}

void manetGLView::watcherIdle()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void manetGLView::paintGL() 
{
    TRACE_ENTER();

    wGraph.doMaintanence(); // check expiration, etc. 
    drawManet();

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
        bgi.drawImage(); 
    }

    bool first=true;
    for (LayerList::iterator li=knownLayers.begin(); li!=knownLayers.end(); ++li)
    {
        if ((*li)->layer==ANTENNARADIUS_LAYER)
            continue;

        if ((*li)->active)
        {
            // each layer is 'layerPadding' above the rest. 
            // layer order is currently defined by order of appearance
            // in the cfg file. Only offset after the first layer.
            if (!first)
                glTranslatef(0.0, 0.0, layerPadding);  
            else
                first=false;
            glPushMatrix();
            // LOG_DEBUG("Drawing layer: " << (*li)->layer); 
            drawLayer((*li)->layer); 
            glPopMatrix();
        }
    }

    // draw status string
    {
        glPushMatrix();

        ptime now = from_time_t(time(NULL));
        glScalef(0.02, 0.02, 0.02);

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

        if (showWallTimeinStatusString)
        {
            buf+=" Wall Time: ";
            buf+=posix_time::to_simple_string(now);
        }
        if (showPlaybackTimeInStatusString)
        {
            buf+=" Play Time: ";
            buf+=posix_time::to_simple_string(from_time_t(currentMessageTimestamp/1000));
        }
        if (showPlaybackRangeString)
        {
            buf+=" Time Range: ";
            buf+=posix_time::to_simple_string(from_time_t(playbackRangeStart/1000));
            buf+=" to ";
            buf+=posix_time::to_simple_string(from_time_t(playbackRangeEnd/1000));
        }

        if (nowTS-2500.0<currentMessageTimestamp)
            qglColor(QColor(monochromeMode ? "black" : "green")); 
        else if (playbackRangeEnd==currentMessageTimestamp && nowTS > playbackRangeEnd)
            qglColor(QColor(monochromeMode ? "black" : "blue")); 
        else
            qglColor(QColor(monochromeMode ? "black" : "red")); 

        if (showVerboseStatusString)
        {
            char buff[256];
            snprintf(buff, sizeof(buff), " Loc: %3.1f, %3.1f, %3.1f, scale: %f, %f, %f, text: %f lpad: %f",
                    manetAdj.shiftX, manetAdj.shiftY, manetAdj.shiftZ, 
                    manetAdj.scaleX, manetAdj.scaleY, manetAdj.scaleZ, 
                    scaleText, layerPadding); 
            buf+=string(buff); 
        }
        renderText(12, height()-12, QString(buf.c_str()), QFont(statusFontName.c_str(), statusFontPointSize)); 

        glPopMatrix();
    }

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
    connect(playbackSlider, SIGNAL(sliderMoved()), this, SLOT(sliderMovedInGUI()));
}

void manetGLView::sliderMovedInGUI()
{
    TRACE_ENTER();
    if (!playbackSlider)
        return;
    // GTL - This does not appear to do anything.
    playbackSlider->setStatusTip(QString(playbackSlider->value()));
    TRACE_EXIT();
}


void manetGLView::updatePlaybackSliderFromGUI()
{
    TRACE_ENTER();
    if (!messageStream || !playbackSlider)
    {
        TRACE_EXIT();
        return;
    }
   
    pausePlayback(); 
    messageStream->setStreamTimeStart(playbackRangeStart+(playbackSlider->value()*1000));
    messageStream->startStream(); 
    // no need to set range or current message timestamp, they'll be updated when 
    // we get the first message at the new timestamp

    TRACE_EXIT();
}

void manetGLView::drawEdge(const WatcherGraphEdge &edge, const WatcherGraphNode &node1, const WatcherGraphNode &node2)
{
    TRACE_ENTER(); 

    GLdouble x1, y1, z1, x2, y2, z2, width;
    gps2openGLPixels(node1.gpsData->dataFormat, node1.gpsData->x, node1.gpsData->y, node1.gpsData->z, x1, y1, z1); 
    gps2openGLPixels(node2.gpsData->dataFormat, node2.gpsData->x, node2.gpsData->y, node2.gpsData->z, x2, y2, z2); 

    width=edge.displayInfo->width;

    GLfloat edgeColor[]={
        edge.displayInfo->color.r/255.0, 
        edge.displayInfo->color.g/255.0, 
        edge.displayInfo->color.b/255.0, 
        edge.displayInfo->color.a/255.0, 
    };

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    if (monochromeMode)
        glColor4fv(black);
    else
        glColor4fv(edgeColor);

    if (!threeDView)
    {
        double ax=atan2(x1-x2,y1-y2);
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
        glPushMatrix();
        glTranslatef(x1,y1,z1);

        // gluCylinder draws "out the z axis", so rotate view 90 on the y axis and angle-between-the-nodes on the x axis
        // before drawing the cylinder

        glRotated(90.0, 0.0, 1.0, 0.0);                             // y rotate
        glRotated(atan2(y1-y2,x2-x1)*(180/M_PI), 1.0, 0.0, 0.0);    // x rotate, y1-y2,x2-x1 was trail and error wrt the quadrants

        // glRotated(atan2(z1-z2,y2-y1)*(180/M_PI), 0.0, 1.0, 0.0);    // y rotate, y1-y2,x2-x1 was trail and error wrt the quadrants
        // glRotated(atan2(x1-x2,z2-z1)*(180/M_PI), 0.0, 0.0, 1.0);    // z rotate, y1-y2,x2-x1 was trail and error wrt the quadrants
        // float distance=sqrt(pow(x1-x2,2)+pow(y1-y2,2)+pow(z1-z2,2));
        float distance=sqrt(pow(x1-x2,2)+pow(y1-y2,2)+pow(z1-z2,2));

        GLUquadric *q=gluNewQuadric();
        gluCylinder(q, width, 0, distance, 10, 10); 
        gluDeleteQuadric(q);

        glPopMatrix(); 
    }

    // draw the edge's label, if there is one.
    if (edge.displayInfo->label!="none")
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
                edge.displayInfo->labelColor.r/255.0, 
                edge.displayInfo->labelColor.g/255.0, 
                edge.displayInfo->labelColor.b/255.0, 
                edge.displayInfo->labelColor.a/255.0
            };
            glColor4fv(clr);
        }

        GLdouble lx=(x1+x2)/2.0; 
        GLdouble ly=(y1+y2)/2.0; 
        GLdouble lz=(z1+z2)/2.0; 
        GLdouble a=atan2(x1-x2 , y1-y2);
        GLdouble th=10.0;
        renderText(lx+sin(a-M_PI_2),ly+cos(a-M_PI_2)*th, lz, 
                QString(edge.displayInfo->label.c_str()),
                QFont(QString(edge.displayInfo->labelFont.c_str()), (int)(edge.displayInfo->labelPointSize*manetAdj.scaleX*scaleText)));
    }

    TRACE_EXIT(); 
}

bool manetGLView::isActive(const watcher::GUILayer &layer)
{
    TRACE_ENTER();

    // GTL fix this
    // LayerList::const_iterator li=std::find(knownLayers.begin(), knownLayers.end(), layer); 
    bool retVal=false;
    BOOST_FOREACH(LayerListItemPtr &llip, knownLayers)
        if (llip->layer==layer)
            retVal=llip->active;

    TRACE_EXIT_RET_BOOL(retVal);
    return retVal;
}

void manetGLView::drawNode(const WatcherGraphNode &node, bool physical)
{
    TRACE_ENTER(); 

    // LOG_DEBUG("Drawing node on " << (physical?"non":"") << "physical layer."); 

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat nodeColor[]={
        node.displayInfo->color.r/255.0, 
        node.displayInfo->color.g/255.0, 
        node.displayInfo->color.b/255.0, 
        physical ? node.displayInfo->color.a/255.0 : ghostLayerTransparency
    };

    GLdouble x, y, z; 
    gps2openGLPixels(node.gpsData->dataFormat, node.gpsData->x, node.gpsData->y, node.gpsData->z, x, y, z); 

    if (monochromeMode)
        glColor4fv(black);
    else
        glColor4fv(nodeColor);

    switch(node.displayInfo->shape)
    {
        case NodeDisplayInfo::CIRCLE: drawSphere(x, y, z, 4, node.displayInfo); break;
        case NodeDisplayInfo::SQUARE: drawCube(x, y, z, 4, node.displayInfo); break;
        case NodeDisplayInfo::TRIANGLE: drawPyramid(x, y, z, 4, node.displayInfo); break;
        case NodeDisplayInfo::TORUS: drawTorus(x, y, z, 4, node.displayInfo); break;
        case NodeDisplayInfo::TEAPOT: drawTeapot(x, y, z, 4, node.displayInfo); break;
    }

    drawNodeLabel(node, physical);

    if (isActive(ANTENNARADIUS_LAYER))
        drawWireframeSphere(x, y, z, antennaRadius, node.displayInfo); 

    TRACE_EXIT(); 
}

void manetGLView::drawNodeLabel(const WatcherGraphNode &node, bool physical)
{
    TRACE_ENTER();

    const GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat nodeColor[]={
        node.displayInfo->labelColor.r/255.0, 
        node.displayInfo->labelColor.g/255.0, 
        node.displayInfo->labelColor.b/255.0, 
        physical ? node.displayInfo->labelColor.a/255.0 : ghostLayerTransparency
    };

    if (monochromeMode)
        glColor4fv(black);
    else
        glColor4fv(nodeColor);

    // a little awkward since we're mixing enums, reserved strings, and free form strings
    char buf[64]; 
    if (!node.nodeId.is_v4())
        snprintf(buf, sizeof(buf), "%s", node.nodeId.to_string().c_str());  // punt
    else
    {
        unsigned long addr=node.nodeId.to_v4().to_ulong(); // host byte order. 

        if (node.displayInfo->label==NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::FOUR_OCTETS))
            snprintf(buf, sizeof(buf), "%lu.%lu.%lu.%lu", ((addr)>>24)&0xFF,((addr)>>16)&0xFF,((addr)>>8)&0xFF,(addr)&0xFF); 
        else if (node.displayInfo->label==NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::THREE_OCTETS))
            snprintf(buf, sizeof(buf), "%lu.%lu.%lu", ((addr)>>16)&0xFF,((addr)>>8)&0xFF,(addr)&0xFF); 
        else if (node.displayInfo->label==NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::TWO_OCTETS))
            snprintf(buf, sizeof(buf), "%lu.%lu", ((addr)>>8)&0xFF,(addr)&0xFF); 
        else if (node.displayInfo->label==NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::LAST_OCTET))
            snprintf(buf, sizeof(buf), "%lu", (addr)&0xFF); 
        else if (node.displayInfo->label=="none")
            buf[0]='\0';
        else if (node.displayInfo->label==NodeDisplayInfo::labelDefault2String(NodeDisplayInfo::HOSTNAME))
        {
            struct in_addr saddr; 
            saddr.s_addr=htonl(addr); 
            struct hostent *he=gethostbyaddr((const void *)saddr.s_addr, sizeof(saddr.s_addr), AF_INET); 

            if (he) 
            {
                snprintf(buf, sizeof(buf), "%s", he->h_name); 
                // only do the lookup one time successfully per host. 
                node.displayInfo->label=buf; 
            }
            else
            {
                LOG_WARN("Unable to get hostnmae for node " << node.nodeId); 
                node.displayInfo->label="UnableToGetHostNameSorry";
            }
        }
        else
            snprintf(buf, sizeof(buf), "%s", node.displayInfo->label.c_str());  // use what is ever there. 
    }        

    GLdouble x, y, z; 
    gps2openGLPixels(node.gpsData->dataFormat, node.gpsData->x, node.gpsData->y, node.gpsData->z, x, y, z); 
    renderText(x, y+6, z+5, QString(buf),
                QFont(node.displayInfo->labelFont.c_str(), 
                     (int)(node.displayInfo->labelPointSize*manetAdj.scaleX*scaleText))); 

    TRACE_EXIT();
}

void manetGLView::drawLabel(GLfloat inx, GLfloat iny, GLfloat inz, const LabelDisplayInfoPtr &label)
{
    TRACE_ENTER(); 
    // 
    int fgColor[]={
        label->foregroundColor.r, 
        label->foregroundColor.g, 
        label->foregroundColor.b, 
        label->foregroundColor.a
    };
    int bgColor[]={
        label->backgroundColor.r, 
        label->backgroundColor.g, 
        label->backgroundColor.b, 
        label->backgroundColor.a
    };

    if (monochromeMode)
        for (unsigned int i=0; i<sizeof(bgColor)/sizeof(bgColor[0]); i++)
            if (i==3)
                fgColor[i]=255;
            else
                fgColor[i]=0; 

    float offset=4.0;
        
    QFont f(label->fontName.c_str(), (int)(label->pointSize*manetAdj.scaleX*scaleText)); 

    // Do cheesy shadow effect as I can't get a proper bounding box around the text as
    // QFontMetric lisea bout how wide/tall the bounding box is. 
    if (!monochromeMode)
    {
        qglColor(QColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3])); 
        QFontMetrics fm(f);
        double shadowOffset=fm.height()*.02;
        renderText(inx+offset+shadowOffset, iny+offset+shadowOffset, inz+1.0, label->labelText.c_str(), f); 
    }

    qglColor(QColor(fgColor[0], fgColor[1], fgColor[2], fgColor[3])); 
    renderText(inx+offset, iny+offset, inz+2.0, label->labelText.c_str(), f); 

    TRACE_EXIT(); 
}

void manetGLView::drawLayer(const GUILayer &layer)
{
    TRACE_ENTER();

    WatcherGraph::edgeIterator ei, eend;
    for(tie(ei, eend)=edges(wGraph.theGraph); ei!=eend; ++ei)
    {
        WatcherGraphEdge &edge=wGraph.theGraph[*ei]; 
        if (edge.displayInfo->layer==layer)
        {
            const WatcherGraphNode &node1=wGraph.theGraph[source(*ei, wGraph.theGraph)]; 
            const WatcherGraphNode &node2=wGraph.theGraph[target(*ei, wGraph.theGraph)]; 
            drawEdge(edge, node1, node2); 
        }

        WatcherGraphEdge::LabelList::iterator li=edge.labels.begin(); 
        WatcherGraphEdge::LabelList::iterator lend=edge.labels.end(); 
        for( ; li!=lend; ++li)
            if ((*li)->layer==layer)
            {
                const WatcherGraphNode &node1=wGraph.theGraph[source(*ei, wGraph.theGraph)]; 
                const WatcherGraphNode &node2=wGraph.theGraph[target(*ei, wGraph.theGraph)]; 

                double lx=(node1.gpsData->x+node2.gpsData->x)/2.0; 
                double ly=(node1.gpsData->y+node2.gpsData->y)/2.0; 
                double lz=(node1.gpsData->z+node2.gpsData->z)/2.0; 

                GLdouble x, y, z; 
                gps2openGLPixels(node1.gpsData->dataFormat, lx, ly, lz, x, y, z); 
                drawLabel(x, y, z, *li);
            }
    }

    WatcherGraph::vertexIterator vi, vend;
    for(tie(vi, vend)=vertices(wGraph.theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &node=wGraph.theGraph[*vi]; 

        if (layer==PHYSICAL_LAYER)
            drawNode(node, true); 
        else
            if (layerPadding>6 || !isActive(PHYSICAL_LAYER))
                drawNode(node, false); 

        WatcherGraphEdge::LabelList::iterator li=node.labels.begin(); 
        WatcherGraphEdge::LabelList::iterator lend=node.labels.end(); 
        for( ; li!=lend; ++li)
            if ((*li)->layer==layer)
            {
                GLdouble x, y, z; 
                double lx=node.gpsData->x;
                double ly=node.gpsData->y;
                double lz=node.gpsData->z;
                gps2openGLPixels(node.gpsData->dataFormat, lx, ly, lz, x, y, z); 
                drawLabel(x, y, z, *li); 
            }
    }

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

void manetGLView::keyPressEvent(QKeyEvent * event)
{
    TRACE_ENTER();

    quint32 nativeKey = event->nativeVirtualKey();
    int qtKey = event->key();

    switch (nativeKey)
    {
        case 'C':
            {
                LOG_DEBUG("Got cap C in keyPressEvent - spawning color chooser for background color"); 
                QRgb rgb=0xffffffff;
                bool ok=false;
                rgb=QColorDialog::getRgba(rgb, &ok);
                if (ok)
                    glClearColor(qRed(rgb)/255.0, qGreen(rgb)/255.0, qBlue(rgb)/255.0, qAlpha(rgb)/255.0);
            }
            break;
        case 'F': 
            {
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
            break;
        case 'T':
            layerPadding=0;
            break;
    }

    switch(qtKey)
    {
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
        case Qt::Key_D:     rotateY(-5.0); break;
        case Qt::Key_F:     rotateY(5.0); break;
        case Qt::Key_C:     rotateZ(-5.0); break;
        case Qt::Key_V:     rotateZ(5.0); break;
        case Qt::Key_K:     gpsScale+=10; break;
        case Qt::Key_L:     gpsScale-=10; break;
        case Qt::Key_T:     layerPadding+=2; break;
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

    if (!num_vertices(wGraph.theGraph))
    {
        TRACE_EXIT_RET(0);
        return 0;
    }

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

    WatcherGraph::vertexIterator vi, vend;
    for(tie(vi, vend)=vertices(wGraph.theGraph); vi!=vend; ++vi)
    {
        WatcherGraphNode &node=wGraph.theGraph[*vi]; 

        unsigned int dist;

        // Convert from GPS to 3d pixels
        GLdouble gx, gy, gz;
        gps2openGLPixels(node.gpsData->dataFormat, node.gpsData->x, node.gpsData->y, node.gpsData->z, gx, gy, gz);

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
            found=(unsigned int)node.nodeId.to_v4().to_ulong();
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

    GUILayer layer=layerName.toStdString();

    bool found=false;
    BOOST_FOREACH(LayerListItemPtr &llip, knownLayers)
    {
        if (llip->layer==layer)
        {
            llip->active=turnOn;
            found=true;
        }
    }
    if(!found)
    {
        LOG_DEBUG("Adding new layer to known layers: " << layer); 
        LayerListItemPtr item(new LayerListItem);
        item->layer=layer;
        item->active=turnOn;
        knownLayers.push_back(item); 
    }

    TRACE_EXIT();
}

void manetGLView::toggleMonochrome(bool isOn)
{
    TRACE_ENTER();
    static GLfloat previousBackgroundColor[4]={0.0, 0.0, 0.0, 0.0}; 
    monochromeMode=isOn;
    emit monochromeToggled(isOn); 
    if (isOn)
    {
        glGetFloatv(GL_COLOR_CLEAR_VALUE, previousBackgroundColor);
        glClearColor(1, 1, 1, 1.0);
    }
    else
    {
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
void manetGLView::clearAllEdges()
{
    TRACE_ENTER();
    WatcherGraph::edgeIterator ei, eend;
    for(tie(ei, eend)=edges(wGraph.theGraph); ei!=eend; ++ei)
        remove_edge(*ei, wGraph.theGraph);  // GTL does this blow away the label memory as well? 
    TRACE_EXIT();
}
void manetGLView::clearAllLabels()
{
    TRACE_ENTER();
    WatcherGraph::edgeIterator ei, eend;
    for(tie(ei, eend)=edges(wGraph.theGraph); ei!=eend; ++ei)
        if (wGraph.theGraph[*ei].labels.size())
            wGraph.theGraph[*ei].labels.clear();

    WatcherGraph::vertexIterator vi, vend;
    for(tie(vi, vend)=vertices(wGraph.theGraph); vi!=vend; ++vi)
        if (wGraph.theGraph[*vi].labels.size())
            wGraph.theGraph[*vi].labels.clear();

    emit labelsCleared();
    TRACE_EXIT();
}

// Should be called after a glTranslate()
void manetGLView::handleSpin(int threeD, const NodeDisplayInfoPtr &ndi)
{
    if (ndi->spin)
    {
        if (threeD)
        {
            glRotatef(ndi->spinRotation_x, 1.0f, 0.0f, 0.0f);
            glRotatef(ndi->spinRotation_y, 0.0f, 1.0f, 0.0f);
        }
        glRotatef(ndi->spinRotation_z, 0.0f, 0.0f, 1.0f);
    }
}

// Should be called after a glTranslate()
void manetGLView::handleSize(const NodeDisplayInfoPtr &ndi)
{
    glScalef(ndi->size, ndi->size, ndi->size);
}

void manetGLView::drawWireframeSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &/*ndi*/)
{
    glPushMatrix();

    glTranslatef(x, y, z);

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
    glPopMatrix();
}

void manetGLView::drawPyramid( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();

    glTranslated(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    // fprintf(stdout, "Drawing triangle with \"radius\" : %f. x/y offset is %f\n", radius, offset); 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glScalef(9,9,9);        // Eyeballing it.

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
    glPopMatrix();
}

void manetGLView::drawCube(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();
    glTranslated(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    GLfloat widthScaled=radius; 

    if (threeDView)
    {
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        // I had this "easy" call to glutDrawSolidCube, but the shadows did not look as good as when I set the 
        // normal myself.
        //  glutSolidCube(widthScaled*2); 
        glScalef(9,9,9);        // Eyeballing it. - this 9 should be in a header somewhere.
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
    glPopMatrix();
}

void manetGLView::drawTeapot(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();
    glTranslated(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

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
    glPopMatrix();
}

void manetGLView::drawDisk( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{
    GLUquadric* q=NULL;

    glPushMatrix();

    glTranslatef(x, y, z);

    handleSize(ndi);
    handleSpin(threeDView, ndi);

    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);

    glPopMatrix();
}


void manetGLView::drawTorus(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{
    GLfloat inner=radius-1;
    GLfloat outer=radius;

    if (threeDView)
    {
        glPushMatrix();
        glTranslated(x, y, z);
        handleSize(ndi);
        handleSpin(threeDView, ndi);

        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidTorus(inner, outer, 10, 10);  
        glPopAttrib();
        glPopMatrix();
    }
    else
    {
        GLUquadric* q=NULL;
        glPushMatrix();
        glTranslatef(x, y, z);
        handleSize(ndi);
        handleSpin(threeDView, ndi);
        q=gluNewQuadric();
        gluDisk(q,inner, outer,36,1);
        gluDeleteQuadric(q);
        glPopMatrix();
    }
}

void manetGLView::drawSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{

    if (threeDView)
    {
        glPushMatrix();
        glTranslated(x, y, z);
        handleSize(ndi);
        handleSpin(threeDView, ndi);
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidSphere(radius, 10, 10);
        glPopAttrib();
        glPopMatrix();
    }
    else
        drawDisk(x,y,z,radius, ndi); 
}

void manetGLView::drawCircle(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, const NodeDisplayInfoPtr &ndi)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    handleSize(ndi);
    handleSpin(threeDView, ndi);
    GLUquadric* q=NULL;
    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);
    glPopMatrix();
}

void manetGLView::drawFrownyCircle(GLdouble x, GLdouble y, GLdouble z, GLdouble)
{ 
    static GLfloat const dead[]={1.0,0.0,0.0,1.0}; 

    glColor4fv(dead);

    // draw outsize circle
    glTranslatef(x, y, z);
    GLUquadric* q=NULL;
    q=gluNewQuadric();
    gluDisk(q, 6, 7, 36, 1);
    gluDeleteQuadric(q);

    // draw eyes and mouth
    glBegin(GL_LINES); 
    glVertex3f(x-4.0,y+3.0,z); 
    glVertex3f(x-2.0,y+1.0,z); 
    glVertex3f(x-2.0,y+3.0,z); 
    glVertex3f(x-4.0,y+1.0,z); 

    glVertex3f(x+4.0,y+3.0,z); 
    glVertex3f(x+2.0,y+1.0,z); 
    glVertex3f(x+2.0,y+3.0,z); 
    glVertex3f(x+4.0,y+1.0,z); 

    glVertex3f(x-3.0,y-3.0,z); 
    glVertex3f(x-2.0,y-2.0,z); 
    glVertex3f(x-2.0,y-2.0,z); 
    glVertex3f(x+2.0,y-2.0,z); 
    glVertex3f(x+2.0,y-2.0,z); 
    glVertex3f(x+3.0,y-3.0,z); 

#if 0 
    glVertex3f(x+0.0,y-2.0,z); 
    glVertex3f(x+2.0,y-4.0,z); 
    glVertex3f(x+2.0,y-4.0,z); 
    glVertex3f(x+3.0,y-4.0,z); 
    glVertex3f(x+3.0,y-4.0,z); 
    glVertex3f(x+3.0,y-3.0,z); 
    glVertex3f(x+3.0,y-3.0,z); 
    glVertex3f(x+2.0,y-2.0,z); 
#endif 
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
        { "showPlaybackRange", showPlaybackRangeString }
    };

    for (size_t i = 0; i < sizeof(boolConfigs)/sizeof(boolConfigs[0]); i++)
        root[boolConfigs[i].prop]=boolConfigs[i].boolVal;

    string prop="layers";
    libconfig::Setting &layers=cfg.lookup(prop);

    // We have to create cfg layers here as we may've gotten new dynamic layers while we were running.
    // I am beginning (ha!) to dislike libconfig...
    BOOST_FOREACH(LayerListItemPtr &llip, knownLayers)
    {
        int i, numCfgLayers=layers.getLength();
        LOG_DEBUG("numCfgLayers=" << numCfgLayers);
        for (i=0; i<numCfgLayers; i++)
        {
            string layerName=string(layers[i].getName());
            if (layerName==llip->layer)
            {
                layers[i]=llip->active;
                break;
            }
        }
        if (i==numCfgLayers) 
        {
            LOG_DEBUG("Adding layer " << llip->layer << " with value " << llip->active << " to " << layers.getName() << " cfg"); 
            layers.add(string(llip->layer), Setting::TypeBoolean)=llip->active;
        }
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
    for (size_t i=0; i<sizeof(floatVals)/sizeof(floatVals[0]); i++)
        root[floatVals[i].prop]=*floatVals[i].val;

    struct 
    {
        const char *prop; 
        string *val; 
    } strVals[] = 
    {
        { "statusFontName", &statusFontName } 
    }; 
    for (size_t i=0; i<sizeof(strVals)/sizeof(strVals[0]); i++)
        root[strVals[i].prop]=*strVals[i].val;

    struct 
    {
        const char *prop; 
        int *val; 
    } intVals[] = 
    {
        { "statusFontPointSize", &statusFontPointSize }
    }; 
    for (size_t i=0; i<sizeof(intVals)/sizeof(intVals[0]); i++)
        root[intVals[i].prop]=*intVals[i].val;

    root["viewPoint"]["angle"][0]=manetAdj.angleX;
    root["viewPoint"]["angle"][1]=manetAdj.angleY;
    root["viewPoint"]["angle"][2]=manetAdj.angleZ;
    root["viewPoint"]["scale"][0]=manetAdj.scaleX;
    root["viewPoint"]["scale"][1]=manetAdj.scaleY;
    root["viewPoint"]["scale"][2]=manetAdj.scaleZ;
    root["viewPoint"]["shift"][0]=manetAdj.shiftX;
    root["viewPoint"]["shift"][1]=manetAdj.shiftY;
    root["viewPoint"]["shift"][2]=manetAdj.shiftZ;

    BackgroundImage &bg=BackgroundImage::getInstance();
    float bgfloatVals[5];
    bg.getDrawingCoords(bgfloatVals[0], bgfloatVals[1], bgfloatVals[2], bgfloatVals[3], bgfloatVals[4]); 
    root["backgroundImage"]["coordinates"][0]=bgfloatVals[0];
    root["backgroundImage"]["coordinates"][1]=bgfloatVals[1];
    root["backgroundImage"]["coordinates"][2]=bgfloatVals[2];
    root["backgroundImage"]["coordinates"][3]=bgfloatVals[3];
    root["backgroundImage"]["coordinates"][4]=bgfloatVals[4];

    GLfloat cols[4]={0.0, 0.0, 0.0, 0.0}; 
    glGetFloatv(GL_COLOR_CLEAR_VALUE, cols);
    root["backgroundColor"]["r"]=cols[0];
    root["backgroundColor"]["g"]=cols[1];
    root["backgroundColor"]["b"]=cols[2];
    root["backgroundColor"]["a"]=cols[3];

    SingletonConfig::unlock();

    wGraph.saveConfig(); 

    SingletonConfig::saveConfig();

    TRACE_EXIT();
}

void manetGLView::pausePlayback()
{
    TRACE_ENTER();
    playbackPaused=true;
    messageStream->stopStream(); 
    messageStream->getMessageTimeRange();
    TRACE_EXIT();
}
void manetGLView::normalPlayback()
{
    TRACE_ENTER();
    playbackPaused=false;
    playbackSetSpeed(1.0);
    messageStream->startStream(); 
    messageStream->getMessageTimeRange();
    TRACE_EXIT();
}
void manetGLView::reversePlayback()
{
    TRACE_ENTER();
    pausePlayback(); 
    if (streamRate!=0.0)
    {
        if (streamRate<0.0)
            playbackSetSpeed(-abs(streamRate*2));
        else
            playbackSetSpeed(-abs(streamRate));
        playbackPaused=false;
        messageStream->startStream(); 
    }
    TRACE_EXIT();
}
void manetGLView::forwardPlayback()
{
    TRACE_ENTER();
    pausePlayback(); 
    if (streamRate!=0.0)
    {
        if (streamRate>0.0)
            playbackSetSpeed(abs(streamRate*2));
        else
            playbackSetSpeed(abs(streamRate));
        playbackPaused=false;
        messageStream->startStream(); 
    }
    messageStream->getMessageTimeRange();
    TRACE_EXIT();
}
void manetGLView::rewindToStartOfPlayback()
{
    TRACE_ENTER();
    pausePlayback(); 
    messageStream->setStreamTimeStart(SeekMessage::epoch); 
    if (streamRate<0.0)
        normalPlayback();
    else
        messageStream->startStream(); 
    playbackPaused=false;
    TRACE_EXIT();
}
void manetGLView::forwardToEndOfPlayback()
{
    TRACE_ENTER();
    pausePlayback(); 
    messageStream->setStreamTimeStart(SeekMessage::eof); 
    playbackPaused=false;
    messageStream->startStream(); 
    currentMessageTimestamp=playbackRangeEnd;
    messageStream->getMessageTimeRange();
    TRACE_EXIT();
}

void manetGLView::playbackSetSpeed(double x)
{
    TRACE_ENTER();
    LOG_DEBUG("Setting stream rate to " << x); 
    streamRate=x;
    messageStream->setStreamRate(streamRate);
    emit streamRateSet(streamRate);
    TRACE_EXIT();
}

