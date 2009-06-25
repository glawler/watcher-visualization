#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "node.h"
#include "mobility.h"
#include "graphics.h"

#include "../manetglview.h"

/*  Copyright (C) 2005  McAfee Inc. 
 *  Copyright (C) 2006  Sparta Inc.  Written by the NIP group, SRD, ISSO
 *  All rights reserved.
 */

static const char *rcsid __attribute__ ((unused)) = "$Id: graphics.cpp,v 1.54 2007/07/13 20:40:22 mheyman Exp $";

#ifdef GRAPHICS

extern NodeDisplayStatus globalDispStat; // allocated in legacyWatcher.o
extern WatcherPropertiesList GlobalWatcherPropertiesList; // allocated in legacyWatcher.o

/*
 * Get the distance from "n" to "(x, y)" in screen coordinates given
 * the transform matrices and the view port.
 *
 * n           - node to find distance to 
 *
 * modelmatrix - from glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
 *
 * projmatrix  - from glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
 *
 * viewport    - from glGetIntegerv(GL_VIEWPORT, viewport);
 *
 * x           - x position in screen coordinates
 *
 * y           - y position in screen coordinates (note that this is
 *               with the origin in the bottom left, so if you have the
 *               normal situation of the origin in the top left, you can
 *               pass "viewport[3] - y" to get the proper value).
 *
 * Returns 0 on success.
 */
static int screenDistSquared(
        unsigned int *dist_squared_ret,
        manetNode *n,
        GLdouble modelmatrix[16],
        GLdouble projmatrix[16],
        GLint viewport[4],
        int x,
        int y)
{
    int ret;
    GLdouble sx, sy, sz;
    if(gluProject(n->x, n->y, n->z, modelmatrix, projmatrix, viewport, &sx,
                &sy, &sz) == GL_TRUE)
    {
        long dx = x - (long)sx;
        long dy = y - (long)sy;
        *dist_squared_ret = (dx*dx) + (dy*dy);
        ret = 0;
    }
    else
    {
        ret = EINVAL;
    }
    return ret;
} // screenDistSquared

void invert4x4(GLdouble dst[16], GLdouble const src[16])
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

/*
 * Get the world coordinate (x,y,z) for the projected coordinates (x, y)
 * at the world coordinate "z" using the given transformation matrices.
 */
int xyAtZForModelProjViewXY(
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

/*
 *
 */
int xyAtZForScreenXY(int x, int y, GLdouble z, GLdouble *x_ret, GLdouble *y_ret)
{
    int ret;
    GLdouble modelmatrix[16];
    GLdouble projmatrix[16];
    GLint viewport[4];
    XYWorldZToWorldXWorldY xyz =
    {
        x,
        viewport[3] - y,
        z,
        0,
        0
    };
    //GLfloat sz;
    glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    // convert y-from-top to y-from-bottom
    // get sz
    //glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &sz);
    if((ret =  xyAtZForModelProjViewXY(
                    &xyz, 1, modelmatrix, projmatrix, viewport)) == 0)
    {
        *x_ret = xyz.worldX_ret;
        *y_ret = xyz.worldY_ret;
    }
    return ret;
} /* xyAtZForScreenXY */

/*
 * Returns the closest node to the disk of radus "r" at screen coordinates (x, y).
 *
 * Returns null if no nodes exist in "m".
 */
manetNode *closestNode(manet *m, int x, int y, unsigned int r, unsigned int *dist_ret)
{
    manetNode *ret;
    if(m && m->numnodes && r < USHRT_MAX) // USHRT_MAX == sqrt(UINT_MAX)
    {
        // in case one node directly on top of another, don't find the
        // same node twice in a row. To do this, we keep track of the
        // last node found. (Note that this only works when "m" doesn't
        // change between calls).
        static size_t lastfound = 0;
        unsigned long min_dist = ULONG_MAX; // distance squared to closest
        unsigned long r2 = r*r;
        size_t found = 0;    // index of closest
        GLdouble modelmatrix[16];
        GLdouble projmatrix[16];
        GLint viewport[4];
        manetNode *nbeg = m->nlist;
        manetNode *nend = nbeg + m->numnodes;
        manetNode *nstop = nbeg + lastfound;
        manetNode *n;
        // Matrices and viewport to convert from world to screen
        // coordinates. The same matrices and view port could be used to
        // convert the "(x, y)" from screen to world. Finding the
        // closest node to the converted "(x, y)" in this case is easy
        // for orthographic projection but, under other projections,
        // requires a more complex solution of finding the node closest
        // to the line through the camera and the returned point.
        glGetDoublev(GL_MODELVIEW_MATRIX, modelmatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);
        // convert y-from-top to y-from-bottom
        y = viewport[3] - y;
        for(n = nstop + 1;; ++n)
        {
            unsigned int dist;
            if(n == nend)
            {
                n = nbeg;
            }
            if(screenDistSquared(&dist, n, modelmatrix, projmatrix, viewport, x, y) == 0)
            {
                dist = dist > r2 ? dist - r2 : 0;
                if(dist < min_dist)
                {
                    found = n - nbeg;
                    min_dist = dist;
                }
            }
            if(n == nstop)
            {
                break;
            }
        }
        if(min_dist < ULONG_MAX)
        {
            lastfound = found;
            ret = nbeg + found;
            if(dist_ret)
            {
                *dist_ret = (unsigned int)sqrtf(min_dist + r2) - r;
            }
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        ret = 0;
    }
    return ret;
} /* closestNode */

// Should be called after a glTranslate()
void handleSpin(int threeD, WatcherPropertyData *prop)
{
    if (prop && prop->spin)
    {
        if (threeD)
        {
            glRotatef(prop->spinRotation_x, 1.0f, 0.0f, 0.0f);
            glRotatef(prop->spinRotation_y, 0.0f, 1.0f, 0.0f);
        }
        glRotatef(prop->spinRotation_z, 0.0f, 0.0f, 1.0f);
    }
}

// Should be called after a glTranslate()
void handleSize(WatcherPropertyData *prop)
{
    if (prop && prop->size)
        glScalef(prop->size, prop->size, prop->size);
}

void drawWireframeSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData * /*prop*/)
{
    glPushMatrix();

    glTranslatef(x, y, z);

    if (globalDispStat.threeDView)
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

void drawPyramid( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{
    glPushMatrix();

    glTranslated(x, y, z);

    handleSize(prop);
    handleSpin(globalDispStat.threeDView, prop);

    // fprintf(stdout, "Drawing triangle with \"radius\" : %f. x/y offset is %f\n", radius, offset); 

    if (globalDispStat.threeDView)
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

void drawCube(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{
    glPushMatrix();
    glTranslated(x, y, z);

    handleSize(prop);
    handleSpin(globalDispStat.threeDView, prop);

    GLfloat widthScaled=radius; 

    if (globalDispStat.threeDView)
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

void drawTeapot(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{
    glPushMatrix();
    glTranslated(x, y, z);

    handleSize(prop);
    handleSpin(globalDispStat.threeDView, prop);

    GLfloat widthScaled=radius; 

    if (globalDispStat.threeDView)
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

void drawDisk( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{
    GLUquadric* q=NULL;

    glPushMatrix();

    glTranslatef(x, y, z);

    handleSize(prop);
    handleSpin(globalDispStat.threeDView, prop);

    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);

    glPopMatrix();
}


void drawTorus(GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{
    GLfloat inner=radius-1;
    GLfloat outer=radius;

    if (globalDispStat.threeDView)
    {
        glPushMatrix();
        glTranslated(x, y, z);
        handleSize(prop);
        handleSpin(globalDispStat.threeDView, prop);

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
        handleSize(prop);
        handleSpin(globalDispStat.threeDView, prop);
        q=gluNewQuadric();
        gluDisk(q,inner, outer,36,1);
        gluDeleteQuadric(q);
        glPopMatrix();
    }
}

void drawSphere( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{

    if (globalDispStat.threeDView)
    {
        glPushMatrix();
        glTranslated(x, y, z);
        handleSize(prop);
        handleSpin(globalDispStat.threeDView, prop);
        glPushAttrib(GL_NORMALIZE);
        glNormal3f(0.0, 0.0, 1.0);
        glutSolidSphere(radius, 10, 10);
        glPopAttrib();
        glPopMatrix();
    }
    else
        drawDisk(x,y,z,radius, prop); 
}

void drawCircle( GLdouble x, GLdouble y, GLdouble z, GLdouble radius, WatcherPropertyData *prop)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    handleSize(prop);
    handleSpin(globalDispStat.threeDView, prop);
    GLUquadric* q=NULL;
    q=gluNewQuadric();
    gluDisk(q,radius-1,radius,36,1);
    gluDeleteQuadric(q);
    glPopMatrix();
}


/*
 * Ignores "radius"
 */
void drawFrownyCircle(GLdouble x, GLdouble y, GLdouble z, GLdouble, WatcherPropertyData *prop)
{
    static GLfloat const dead[]={1.0,0.0,0.0,1.0};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, dead);
    drawCircle(x, y, z, 7, prop);
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

void drawText( GLdouble x, GLdouble y, GLdouble z, GLdouble scale, char *text, GLdouble lineWidth)
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

GLfloat drawTextHeight(char *text)
{
	int numlines=1;
	char *p;

	p=text;
	while(*p)
	{
		switch(*p)
		{
			case '\n':
				numlines++;
			break;
		}
		p++;
	}
	if (numlines==0)
		numlines=1;

    return glutStrokeWidth(GLUT_STROKE_ROMAN,'W')*numlines;
}

GLfloat drawTextWidth(char *text)
{
    return glutStrokeLength(GLUT_STROKE_ROMAN,(unsigned char*)text);
}

// Draw Arrow is only called when drawing the hierarchy, not the MANET view, so don't
// need to have a 3d view case.
void drawArrow(GLdouble x1, GLdouble y1, GLdouble x2,GLdouble y2, GLdouble width)
{
    double a;

    glBegin(GL_LINES);
    glVertex3f(x1,y1,0);
    glVertex3f(x2,y2,0);

    a=atan2(x1-x2,y1-y2);

    glVertex3f(x2+sin(a-0.5)*width,y2+cos(a-0.5)*width,0);
    glVertex3f(x2,y2,0);
    glVertex3f(x2,y2,0);
    glVertex3f(x2+sin(a+0.5)*width,y2+cos(a+0.5)*width,0);

    glEnd();
}

void drawHeavyArrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2,GLdouble z2, GLdouble width)
{
    double ax=atan2(x1-x2,y1-y2);
    double cmx = sin(ax)*width;  // cos(a-M_PI_2)*width
    double cpx = -cmx;           // cos(a+M_PI_2)*width
    double smx = -cos(ax)*width; // sin(a-M_PI_2)*width
    double spx = -smx;           // cos(a+M_PI_2)*width

    // double ay=atan2(x1-x2,y1-y2);
    // double cmy = sin(ay)*width;  // cos(a-M_PI_2)*width
    // double cpy = -cmy;           // cos(a+M_PI_2)*width
    // double smy = -cos(ay)*width; // sin(a-M_PI_2)*width
    // double spy = -smy;           // cos(a+M_PI_2)*width

    if (!globalDispStat.threeDView)
    {
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
}


static float theta(points a, points b)
{
	float ang;
	ang=atan2(a.x-b.x, a.y-b.y)/(2*M_PI)*360.0+360;
	if (ang>360.0)
		ang-=360;
	return ang;
}

/* Code to draw the convex hull of a list of points.
** except that it does not work.
**
** This code copied from  (with edits)
** http://alpha.mini.pw.edu.pl/~kotowski/Grafika/GeometricAlgorithms/Index.html
*/
int drawWrap(points p[], int N)
{
int i, min, M; float th, v; points t;
float na;

	min=0;
	for (i = 1; i < N; i++)
		if (p[i].y < p[min].y)
			min = i;


	p[N] = p[min];
	th = 180.0;

	printf("first min: %d\n",min);

	for (M = 0; M < N; M++)
	{
		t = p[M];
		p[M] = p[min];
		p[min] = t;

		min = N;
		v = th-180;
		if (v<0)
			v+=360;
		th = 360.0;

		printf("starting...  v= %f\n",v);
		for (i = M+1; i <= N; i++)
		{
			na=v + theta(p[M],p[i]);
			if (na>360.0)
				na-=360.0;
#if 0
			if (na<0.0)
				na+=360.0;
#endif

			printf("%d: na= %f\n",i,na);

			if ((na < th))
			{
				min = i;
				th = na;
			}
#if 0
			if (theta(p[M], p[i]) > v)
				if (theta(p[M], p[i]) < th)
				{
					min = i;
					th = theta(p[M], p[min]);
				}
#endif
		}
		printf("min= %d  th= %f\n",min,th);

		if (min == N)
			return M;
	}
	return N;
}



static GLfloat neighborcolors[][4] = 
{
    {0.2, 0.2, 1.0, 1.0},
    {0.2, 1.0, 0.2, 1.0},
    {0.3, 1.0, 1.0, 1.0},
    {1.0, 0.2, 0.2, 1.0},
    {1.0, 0.3, 1.0, 1.0},
    {1.0, 1.0, 0.3, 1.0},
    {0.1, 0.1, 0.1, 1.0}
};

static GLfloat neighborcolorsDark[][4] = 
{
    {0.0, 0.0, 0.6, 1.0},
    {0.0, 0.6, 0.0, 1.0},
    {0.0, 0.6, 0.6, 1.0},
    {0.6, 0.0, 0.0, 1.0},
    {0.6, 0.0, 0.6, 1.0},
    {0.8, 0.6, 0.0, 1.0},
    {0.0, 0.0, 0.0, 1.0}
};

static GLfloat neighborcolorsFade[][4] = 
{
    {0.0, 0.0, 0.5, 0.3},
    {0.0, 0.5, 0.0, 0.3},
    {0.0, 0.6, 0.6, 0.3},
    {0.5, 0.0, 0.0, 0.3},
    {0.6, 0.0, 0.6, 0.3},
    {0.6, 0.6, 0.0, 0.3},
    {0.0, 0.0, 0.0, 0.3}
};

#define NBR_COLOR_NUM(nbrColorArray) (sizeof(nbrColorArray)/sizeof(nbrColorArray[0]))

#define TEXT_SCALE 0.08

void nodeDrawLabel(manetNode *us, NodeDisplayType dispType, NodeDisplayStatus const *dispStat, GLfloat nodex, GLfloat nodey, GLfloat nodez)
{

    if (us->labelList)
    {
        NodeLabel *l;
        GLfloat h=0.0,w=0.0;

        for(l=us->labelList;l;l=l->next)
        {
            GLfloat t;

            if (l->priority>dispStat->minPriority)
                continue;
            if ((dispStat) && (!(dispStat->familyBitmap & (1<<l->family))))
                continue;

            h+=drawTextHeight(l->text)* dispStat->scaleText[dispType];
            t=drawTextWidth(l->text)* dispStat->scaleText[dispType];
            if (t>w)
                w=t;
        }
        if(h > 0.0) // if there is text for the label
        {
            GLfloat fgcolor[4],bgcolor[4];
            GLfloat x=nodex+6;
            GLfloat y=nodey-6;
            GLfloat z=nodez+0.3+(0.3*(us->index & 0xFF));
            // static const GLfloat gray[]={0.6,0.6,0.6,1.0};
            static const GLfloat black[]={0.0,0.0,0.0,1.0};
            GLfloat border_width = 
                2.0*(dispStat->scaleText[dispType] > TEXT_SCALE ?
                        sqrt(dispStat->scaleText[dispType]/TEXT_SCALE) :
                        (dispStat->scaleText[dispType]/TEXT_SCALE));
            GLfloat border_width2 = border_width + border_width;
            h+=border_width2;
            w+=border_width2;

            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(nodex,nodey,nodez);
            glVertex3f(x+w*0.03,y,z);
            glVertex3f(x,y,z);
            glVertex3f(x,y,z);
            glVertex3f(x,y-h*0.03,z);
            glEnd();
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
            glBegin(GL_LINE_LOOP);
            glVertex3f(x,y,z+0.2);
            glVertex3f(x+w,y,z+0.2);
            glVertex3f(x+w,y-h,z+0.2);
            glVertex3f(x,y-h,z+0.2);
            glEnd();

            for(l=us->labelList;l;l=l->next)
            {
                GLfloat localh=drawTextHeight(l->text)* dispStat->scaleText[dispType];

                if (l->priority>dispStat->minPriority)
                    continue;
                if ((dispStat) && (!(dispStat->familyBitmap & (1<<l->family))))
                    continue;

                if (dispStat->monochromeMode)
                {
                    bgcolor[0]=1.0;
                    bgcolor[1]=1.0;
                    bgcolor[2]=1.0;
                    bgcolor[3]=1.0;
                }
                else
                {
                    bgcolor[0]=l->bgcolor[0]/255.0;
                    bgcolor[1]=l->bgcolor[1]/255.0;
                    bgcolor[2]=l->bgcolor[2]/255.0;
                    bgcolor[3]=l->bgcolor[3]/255.0;
                }

                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bgcolor);
                glBegin(GL_POLYGON);
                glVertex3f(x  ,y,z+0.1);
                glVertex3f(x+w,y,z+0.1);
                glVertex3f(x+w,y-localh-0.5,z+0.1);
                glVertex3f(x  ,y-localh-0.5,z+0.1);
                glEnd();

                if (dispStat->monochromeMode)
                {
                    fgcolor[0]=0.0;
                    fgcolor[1]=0.0;
                    fgcolor[2]=0.0;
                    fgcolor[3]=1.0;
                }
                else
                {
                    fgcolor[0]=l->fgcolor[0]/255.0;
                    fgcolor[1]=l->fgcolor[1]/255.0;
                    fgcolor[2]=l->fgcolor[2]/255.0;
                    fgcolor[3]=l->fgcolor[3]/255.0;
                }

                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, fgcolor);
                y-=drawTextHeight("X")*dispStat->scaleText[dispType];
                drawText(x+border_width,y-border_width,z+0.2, dispStat->scaleText[dispType], l->text);
                y-=localh-drawTextHeight("X")* dispStat->scaleText[dispType];
            }
            /* Fill in gap at the bottom of the label...  */
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bgcolor);
            glBegin(GL_POLYGON);
            glVertex3f(x  ,y,z+0.1);
            glVertex3f(x+w,y,z+0.1);
            glVertex3f(x+w,y-border_width2,z+0.1);
            glVertex3f(x  ,y-border_width2,z+0.1);
            glEnd();
        }
    }
}

static void nodeDrawFn(
        manetNode *us, 
        NodeDisplayType dispType,
        NodeDisplayStatus const *dispStat, 
        void (*drawFn)(GLdouble, GLdouble, GLdouble, GLdouble, WatcherPropertyData *prop),
        WatcherPropertyData *prop)
{
    const GLfloat antennaAlpha=0.5;
    GLfloat root[]={0.0,1.0,0.0,1.0};
    const GLfloat black[]={0.0,0.0,0.0,1.0};
    GLfloat leaf[]={1.0,0.0,0.0,1.0};
    const GLfloat aradius[]={1.0,0.0,0.0,0.15};
    static const GLfloat nodelabel[]={0.0,0.0,1.0,antennaAlpha};
    int i,j;
    char buff[1024];
    GLfloat tmp[4];

    // normal color
    if (us->color)
        for (i=0;i<4;i++)
            tmp[i]=us->color[i]/255.0;

    // flashing color
    if (prop && prop->flash && prop->isFlashed)
    {
        if (us->color)
            for (i=0;i<3;i++)
                tmp[i]=1-(us->color[i]/255.0);
        else
        {
            for (i=0;i<3;i++)
                leaf[i]=1-leaf[i];
            for (i=0;i<3;i++)
                root[i]=1-root[i];
        }
    }


    if (dispStat->monochromeMode)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    }
    else
    {
        if (us->color)
        {
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tmp);
        }
        else
        {
            if ((us->rootflag) && (dispStat->familyBitmap & (1<<COMMUNICATIONS_LABEL_FAMILY_HIERARCHY)))
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,root);
            else
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,leaf);
        }
    }

    drawFn(us->x,us->y,us->z,4, prop);

    if (dispStat->familyBitmap & (1<<COMMUNICATIONS_LABEL_FAMILY_HIERARCHY))
    {
        for(j=0;j<us->level;j++)
        {
            drawFn(us->x, us->y, us->z, HIERARCHY_RADIUS(j), prop);
        }
    }

    if (dispStat->familyBitmap & (1<<COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS))
    {
        if (!dispStat->monochromeMode)            /* in mono-mode, just leave the material black...   */
        {
            if (us->color)
            {
                GLfloat tmp[4];
                tmp[0]=us->color[0]/255.0;
                tmp[1]=us->color[1]/255.0;
                tmp[2]=us->color[2]/255.0;
                tmp[3]=antennaAlpha; 
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tmp);
            }
            else
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, aradius);
        }
        drawWireframeSphere(us->x,us->y,us->z,us->aradius, prop);
    }

    if (!dispStat->monochromeMode)            /* in mono-mode, just leave the material black...   */
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, nodelabel);

    if (!prop)     
    {
        sprintf(buff,"%d.%d",(us->addr>>8)&0xFF, us->addr & 0xFF);
        drawText(us->x,us->y+6,us->z+5,dispStat->scaleText[dispType], buff);
    }
    else
    {
        drawText(us->x,us->y+6,us->z+5,dispStat->scaleText[dispType], prop->guiLabel);
    }

    nodeDrawLabel(us, dispType, dispStat, us->x, us->y, us->z);
    nodeLabelTimeout(us);

} /* nodeDrawFn */

void nodeDraw(
        manetNode *us, 
        NodeDisplayType dispType,
        NodeDisplayStatus const *dispStat,
        WatcherPropertyData *prop)
{
    if (!prop)       // default is to draw a circle
        nodeDrawFn(us, dispType, dispStat, drawSphere, prop);
    else
    {
        switch(prop->shape)
        {
            case WATCHER_SHAPE_CIRCLE:
                nodeDrawFn(us, dispType, dispStat, drawSphere, prop);
                break;
            case WATCHER_SHAPE_SQUARE:
                nodeDrawFn(us, dispType, dispStat, drawCube, prop);
                break;
            case WATCHER_SHAPE_TRIANGLE:
                nodeDrawFn(us, dispType, dispStat, drawPyramid, prop);
                break;
            case WATCHER_SHAPE_TORUS:
                nodeDrawFn(us, dispType, dispStat, drawTorus, prop);
                break;
            case WATCHER_SHAPE_TEAPOT:
                nodeDrawFn(us, dispType, dispStat, drawTeapot, prop);
                break;
        }
    }
}

void nodeDrawFrowny(
        manetNode *us, 
        NodeDisplayType dispType,
        NodeDisplayStatus const *dispStat, 
        WatcherPropertyData *prop)
{
    nodeDrawFn(us, dispType, dispStat, drawFrownyCircle, prop);
}

void drawNodes(
        NodeDisplayType dispType,
        manet *m)
{
    int i;
    NodeDisplayStatus dispStat;

    dispStat.minPriority=COMMUNICATIONS_LABEL_PRIORITY_INFO;
    dispStat.familyBitmap=COMMUNICATIONS_LABEL_FAMILY_ALL & ~(1 << COMMUNICATIONS_LABEL_FAMILY_ANTENNARADIUS);
    dispStat.monochromeMode=globalDispStat.monochromeMode;
    dispStat.scaleText[dispType]=0.08;
    dispStat.scaleLine[dispType]=1.0;

    

    for(i=0;i<m->numnodes;i++)
    {
        WatcherPropertyData *propData=findWatcherPropertyData(i, GlobalWatcherPropertiesList);
        nodeDraw(&(m->nlist[i]), dispType, &dispStat, propData);
    }
}


/* Given a distance or connectivity graph from nodeGetPhysicalGraph
** and friends, draw the connecting edges.
*/
void drawGraph(manet *m, int *graph,GLdouble scale, int drawlens)
{
	int i,j;
	int n=m->numnodes;
	char buff[128];

	double a;
	double cx,cy;
	double  th=10.0;

	for(i=0;i<n;i++)
		for(j=0;j<n;j++)
		{
			if (graph[i*n+j]>0)
			{
				drawHeavyArrow(m->nlist[i].x,m->nlist[i].y,m->nlist[i].z,m->nlist[j].x,m->nlist[j].y,m->nlist[j].z,1.5);

				if ((drawlens) && (graph[i*n+j]>1))
				{
					sprintf(buff,"%d",graph[i*n+j]);
					cx=(m->nlist[i].x+m->nlist[j].x) /2.0;
					cy=(m->nlist[i].y+m->nlist[j].y) /2.0;
					a=atan2(m->nlist[i].x-m->nlist[j].x , m->nlist[i].y-m->nlist[j].y);
					drawText( cx+sin(a-M_PI_2),cy+cos(a-M_PI_2)*th , (m->nlist[i].z+m->nlist[j].z)/2, scale ,buff);
				}
			}
		}
}

#define HIERARCHY_LEVEL_SPACING 50

static void getWidth(manet *m, ManetAddr nod, int level, int *levels);
static int drawHierarchyrecurse(manet *m, int x, int width, ManetAddr node, int level,int xcoord[], int depth, int *levels, NodeDisplayStatus const *dispStat);

/* This draws the hierarchy view of the MANET on the bottom of the display
*/
void drawHierarchy(manet *m,NodeDisplayStatus const *dispStat)
{
#ifdef GRAPHICS
	int xcoord[m->numnodes];
	int i;
	int maxlevel;
	int levelhistogram[MAXLEVEL];
	char buff[64];
	int *levels;
	int *hierarchygraph;
	static const GLfloat blue[]={0.0,0.0,1.0,0.6};

#if 1
	hierarchygraph=manetGetHierarchyGraph(m);
	levels=graphComputeLevels(m,hierarchygraph);
	free(hierarchygraph);
#else
	levels=manetGetHierarchyLevels(m);
#endif

	for(i=0;i<MAXLEVEL;i++)
	{
		levelhistogram[i]=0;
	}

	for(i=0;i<m->numnodes;i++)
	{
		levelhistogram[levels[i]]++;
	}

	for(i=0;i<m->numnodes;i++)
		xcoord[i]=0;

	/* so, walk the list of nodes, and find the top most occupied level  */

	maxlevel=levels[0];
	for(i=1;i<m->numnodes;i++)
		if (levels[i] > maxlevel)
			maxlevel=levels[i];

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);

	glPushMatrix();
	glTranslatef(22.0, -400.0, 0.0);

    int width = 0;
	for(i=0;i<=maxlevel;i++)
	{
        int levelwidth = levelhistogram[i]*22;
        if(levelwidth > width)
        {
            width = levelwidth;
        }
		sprintf(buff,"%d",levelhistogram[i]);
		drawText(-20, i*HIERARCHY_LEVEL_SPACING-5, 0, dispStat->scaleText[NODE_DISPLAY_HIERARCHY], buff);
	}

	drawHierarchyrecurse(m,0,width,NODE_BROADCAST,maxlevel,xcoord,0,levels,dispStat);

	glPopMatrix();

	free(levels);
#endif
}

/* Returns the number of nodes per level under the node specified
** used to compute the width of a sub-tree
*/

static void getWidth(manet *m, ManetAddr nod, int level, int *levels)
{
	int i;

	if (level<0)     /* recursion failsafe */
		return;

	for(i=0;i<m->numnodes;i++)
		if ((m->nlist[i].clusterhead) && (m->nlist[i].clusterhead->addr==nod))
		{
			levels[level]++;
			getWidth(m,m->nlist[i].addr,level-1,levels);
		}
}


//
// returns the number of kids
//
static int drawHierarchyrecurse(
        manet *m, 
        int x,      // how far from the left to start placing nodes
        int width,  // width to place nodes into
        ManetAddr node,  // node to draw the children of, NODE_BROADCAST gets roots
        int level,
        int xcoord[],
        int depth,  // just used to track recursion
        int *levels,  // m->numnodes long
        NodeDisplayStatus const *dispStat)
{
    int topnum=0;
    int botnum[200];
    int nodeidx[200];
    int bottot=0;
    char buff[64];
    int nwidth;
    int nkids = 1;

    if (level < 0)
        return 1;

    if (depth > 50)
        return 1;

    memset(botnum,0,sizeof(botnum));

    /* node < 0 is a special case, means draw all the nodes at level level  */

    if (node==NODE_BROADCAST)
    {
        for(int i = 0; i < m->numnodes; i++)         /* we want to draw this many nodes at this level, width pixels wide */
        {
            if ((levels[i]<=level) && (m->nlist[i].rootflag))
                nodeidx[topnum++]=i;
        }
    }
    else
    {
        for(int i = 0; i < m->numnodes; i++)         /* we want to draw this many nodes at this level, width pixels wide */
        {
            if (((m->nlist[i].clusterhead) && (m->nlist[i].clusterhead->addr==node)))
                nodeidx[topnum++] = i;
        }
    }

    /* the array nodeidx[] now contains the list of indexes of nodes we want to draw */

    for(int i = 0; i < topnum; i++)
    {
        int widths[200];

        memset(widths,0,sizeof(widths));
        getWidth(m, m->nlist[nodeidx[i]].addr, level, widths);

        botnum[i]=widths[level];               /* find max width below this node */
        int j = level-1;
        while((j>=0) && (widths[j]>0))
        {
            if (widths[j]>botnum[i])
                botnum[i]=widths[j];
            j--;
        }
        botnum[i]++;
    }

    /* botnum[i] now contains the max width of the subtree under node
     * i in the nodeidx array
     */

    bottot=0;
    for(int i = 0; i < topnum; i++)             /* compute total of max widths...  */
    {
        bottot += botnum[i];
    }

    for(int i = 0; i < topnum; i++)
    {
        nwidth = (int)(botnum[i]*(width/(float)bottot));
        xcoord[m->nlist[nodeidx[i]].index] = x + nwidth/2;

        int j = drawHierarchyrecurse(
                m, 
                x,
                nwidth,
                m->nlist[nodeidx[i]].addr,
                levels[nodeidx[i]],
                xcoord,
                depth+1,
                levels,
                dispStat);

        x += nwidth;

        //		glColor3f(neighborcolors[levels[nodeidx[i]]][0],neighborcolors[levels[nodeidx[i]]][1],neighborcolors[levels[nodeidx[i]]][2]);
        sprintf(buff,"%d",m->nlist[nodeidx[i]].addr & 0xFF);
        if (j > 1)    /* only display recursive child count if >1  */
        {
            char b2[16];
            sprintf(b2," %d",j);
            strcat(buff,b2);
        }

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, neighborcolors[(levels[nodeidx[i]])%NBR_COLOR_NUM(neighborcolors)]);
        drawCircle(xcoord[m->nlist[nodeidx[i]].index], levels[nodeidx[i]]*HIERARCHY_LEVEL_SPACING, 0, 12, NULL);

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, neighborcolorsDark[(levels[nodeidx[i]])%NBR_COLOR_NUM(neighborcolorsDark)]);
        drawText(xcoord[m->nlist[nodeidx[i]].index]-6, levels[nodeidx[i]]*HIERARCHY_LEVEL_SPACING-5, 0, dispStat->scaleText[NODE_DISPLAY_HIERARCHY], buff);

        if (m->nlist[nodeidx[i]].clusterhead)
        {
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, neighborcolorsFade[(levels[nodeidx[i]]%NBR_COLOR_NUM(neighborcolorsFade))]);
            drawArrow(
                    xcoord[m->nlist[nodeidx[i]].index],
                    levels[nodeidx[i]]*HIERARCHY_LEVEL_SPACING+10,
                    xcoord[manetGetNodeNum(m,m->nlist[nodeidx[i]].clusterhead->addr)],
                    levels[manetGetNodeNum(m,m->nlist[nodeidx[i]].clusterhead->addr)]*HIERARCHY_LEVEL_SPACING-10,
                    9.0);
        }

        nodeDrawLabel(&m->nlist[nodeidx[i]], NODE_DISPLAY_HIERARCHY, dispStat, xcoord[m->nlist[nodeidx[i]].index],levels[nodeidx[i]]*HIERARCHY_LEVEL_SPACING-10,0);

        nkids += j;
    }

    return nkids;
} // drawHierarchyrecurse

#endif

#ifdef GRAPHICS
void manetDraw(manet *m)
{
    int i;
    int levelhistogram[MAXLEVEL];
    int *phy;
    static const GLfloat arena[]={0.0,1.0,1.0,1.0};
    static const GLfloat hierarchy[]={0.0,0.0,1.0,0.5};
    static const GLfloat physical[]={1.0,0.0,0.0,1.0};
    // static const NodeDisplayStatus dispstat =
    // {
    //     COMMUNICATIONS_LABEL_PRIORITY_INFO, 0xFFFFFFFF, { 0.08, 0.08 }, { 1.0, 1.0 } , 0, 0
    // };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, arena);
    // glBegin(GL_LINE_STRIP);
    glBegin(GL_QUADS);
    glVertex3f(0,0,0);
    glVertex3f(m->mobility->maxx,0,0);
    glVertex3f(m->mobility->maxx,m->mobility->maxy,0);
    glVertex3f(0,m->mobility->maxy,0);
    glVertex3f(0,0,0);
    glEnd();

    MobilityConstraint *con;
    for(con=m->mobility->constraintList; con!=NULL; con=con->next)
    {
        switch(con->type)
        {
            case MOBILITYCONSTRAINT_NOFLY:

                glBegin(GL_LINE_STRIP);
                glVertex3f(con->data.nofly.x,con->data.nofly.y,0);
                glVertex3f(con->data.nofly.x+con->data.nofly.width,con->data.nofly.y,0);
                glVertex3f(con->data.nofly.x+con->data.nofly.width,con->data.nofly.y+con->data.nofly.height,0);
                glVertex3f(con->data.nofly.x,con->data.nofly.y+con->data.nofly.height,0);
                glVertex3f(con->data.nofly.x,con->data.nofly.y,0);
                glEnd();
                break;
            case MOBILITYCONSTRAINT_FLYONLY:
                glBegin(GL_LINE_STRIP);
                glVertex3f(con->data.flyonly.minx,con->data.flyonly.miny,0);
                glVertex3f(con->data.flyonly.minx,con->data.flyonly.maxy,0);
                glVertex3f(con->data.flyonly.maxx,con->data.flyonly.maxy,0);
                glVertex3f(con->data.flyonly.maxx,con->data.flyonly.miny,0);
                glVertex3f(con->data.flyonly.minx,con->data.flyonly.miny,0);
                glEnd();
                break;
            case MOBILITYCONSTRAINT_REPEL:
                break;
        }
    }

    for(i=0;i<MAXLEVEL;i++)
    {
        levelhistogram[i]=0;
    }

    for(i=0;i<m->numnodes;i++)
    {
        levelhistogram[m->nlist[i].level]++;
    }

#if 0
    static const GLfloat histogram[]={0.0,0.0,1.0,1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, histogram);
    for(i=0;i<MAXLEVEL;i++)
    {
        char buff[1024];
        sprintf(buff,"%d",levelhistogram[i]);
        drawText(-20,-i*HIERARCHY_LEVEL_SPACING-5,0,0.08, buff);
    }
#endif

    // drawHierarchy(m, &dispstat);
    drawHierarchy(m, &globalDispStat);

    /* Physical links are on plane Z=0  */
#if 1
    phy=manetGetPhysicalGraph(m);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, physical);
        // glColor4f(1.0,0.0,0.0,0.5);
        drawGraph(m,phy,0.0, 0);
     glPopAttrib();
    free(phy);
#endif

    phy=manetGetHierarchyGraph(m);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, hierarchy);
    glPushMatrix();
    glTranslatef(0.0,0.0,+0.1);
    drawGraph(m,phy,0.0, 0);
    glPopMatrix();
    free(phy);

    glPushMatrix();
    glTranslatef(0.0,0.0,+0.2);
    drawNodes(NODE_DISPLAY_MANET, m);
    glPopMatrix();
}
#endif

#if 0
void graphicsMakeBitmap(manet *m)
{
	GLuint texname;
	int width=m->maxx;
	int height=m->maxy;
	unsigned char *p;

	unsigned char *imgdata=malloc(width*height*sizeof(char)*4);

	for(x=0;x<width;x++)
		for(y=0;y<hieght;y++)
		{
			p=imgdata+(x+(y*width))*4;
			p[0]=x;
			p[1]=y;
			p[2]=0;
			p[3]=255;
		}
	

/* Init code:
 */

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1,&texname);
	glBindTexture(GL_TEXTURE_2D,texname);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,imgdata);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);



/* Draw code:
 */
	glBindTexture(GL_TEXTURE_2D,texname);
	glBegin(GL_QUADS);

	// glColor4f(1.0,1.0,1.0,1.0);
	glTexCoord2f(0.0,1.0);
	glVertex3f(0.0,0.0,0.0);

	glTexCoord2f(1.0,1.0);
	glVertex3f(width,0.0,0.0);

	glTexCoord2f(1.0,0.0);
	glVertex3f(width,height,0.0);

	glTexCoord2f(0.0,0.0);
	glVertex3f(0.0,height,0.0);

#if 0
	// glColor4f(1.0,1.0,1.0,1.0);
	glTexCoord2f(0.0,fs->texheit);
	glVertex3f(0.0,0.0,0.0);

	glTexCoord2f(0.0,0.0);
	glVertex3f(0.0,fs->heit,0.0);

	glTexCoord2f(fs->texwid,0.0);
	glVertex3f(fs->wid,fs->heit,0.0);

	glTexCoord2f(fs->texwid,fs->texheit);
	glVertex3f(fs->wid,0.0,0.0);
#endif

	glEnd();

}

#endif
