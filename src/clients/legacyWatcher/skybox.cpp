/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file skybox.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <GL/glu.h>
#include "skybox.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(Skybox, "Skybox"); 

Skybox *Skybox::getSkybox()
{
    TRACE_ENTER();
    static Skybox theOneAndOnlySkyBox;
    TRACE_EXIT();
    return &theOneAndOnlySkyBox;
}

Skybox::Skybox() : width(100.0)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

Skybox::~Skybox()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void Skybox::drawSkybox(GLfloat camX, GLfloat camY, GLfloat camZ) 
{
    TRACE_ENTER();

    // GLfloat skyColor[4] =    { 0,   0, 1.0, 1.0 };
    // GLfloat groundColor[4] = { 0, 1.0, 1.0, 1.0 };

    // Store the current matrix
    glPushMatrix();

    // Reset and transform the matrix.
    glLoadIdentity();
    gluLookAt(
            0,0,0, 
            camX, camY, camZ,
            0,1,0);

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT | GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Just in case we set all vertices to white.
    // glColor4f(1,1,1,1);

    // Render the front quad
    // glBindTexture(GL_TEXTURE_2D, _skybox[0]);
    // glColor4fv(skyColor);
    glBegin(GL_QUADS);
    glNormal3f( 0.0f, 0.0f, 1.0f); 
    glVertex3f(  width, -width, -width );
    glVertex3f( -width, -width, -width );
    glVertex3f( -width,  width, -width );
    glVertex3f(  width,  width, -width );
    glEnd();

    // Render the left quad
    // glBindTexture(GL_TEXTURE_2D, _skybox[1]);
    glBegin(GL_QUADS);
    glVertex3f(  width, -width,  width );
    glVertex3f(  width, -width, -width );
    glVertex3f(  width,  width, -width );
    glVertex3f(  width,  width,  width );
    glEnd();

    // Render the back quad
    // glBindTexture(GL_TEXTURE_2D, _skybox[2]);
    glBegin(GL_QUADS);
    glVertex3f( -width, -width,  width );
    glVertex3f(  width, -width,  width );
    glVertex3f(  width,  width,  width );
    glVertex3f( -width,  width,  width );
    glEnd();

    // Render the right quad
    // glBindTexture(GL_TEXTURE_2D, _skybox[3]);
    glBegin(GL_QUADS);
    glVertex3f( -width, -width, -width );
    glVertex3f( -width, -width,  width );
    glVertex3f( -width,  width,  width );
    glVertex3f( -width,  width, -width );
    glEnd();

    // Render the top quad
    // glBindTexture(GL_TEXTURE_2D, _skybox[4]);
    glBegin(GL_QUADS);
    glVertex3f( -width,  width, -width );
    glVertex3f( -width,  width,  width );
    glVertex3f(  width,  width,  width );
    glVertex3f(  width,  width, -width );
    glEnd();

    // Render the bottom quad
    // glBindTexture(GL_TEXTURE_2D, _skybox[5]);
    // glColor4fv(groundColor);
    glBegin(GL_QUADS);
    glVertex3f( -width, -width, -width );
    glVertex3f( -width, -width,  width );
    glVertex3f(  width, -width,  width );
    glVertex3f(  width, -width, -width );
    glEnd();

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();

    TRACE_EXIT();
}

