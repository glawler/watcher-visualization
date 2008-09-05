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

#if __linux__
#define _SVID_SOURCE 1		/* pull in definition of drand48() */
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#include "graphics.h"

/*  Copyright (C) 2005  McAfee Inc. 
**  All rights reserved.
*/

static const char *rcsid __attribute__ ((unused)) = "$Id: test.c,v 1.7 2007/04/25 14:20:05 dkindred Exp $";

#define TEXT_SCALE 0.08

float angleX = 0.0, angleY = 0.0, angleZ = 0.0;
float scaleX = 1.0, scaleY = 1.0, scaleZ = 1.0;
float shiftX = 0.0, shiftY = 0.0, shiftZ = 0.0;

points somepoints[10];
int numpoints;


static void Init(void)
{

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearIndex(0.0);
}

static void Reshape(int width, int height)
{

    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 800.0, 0.0, 800.0, .0, 800.0);
    glMatrixMode(GL_MODELVIEW);
}

static void Key2(int key, int x, int y)
{

    switch (key) {
      case GLUT_KEY_LEFT:
	shiftX -= 20.0;
	break;
      case GLUT_KEY_RIGHT:
	shiftX += 20.0;
	break;
      case GLUT_KEY_UP:
	shiftY += 20.0;
	break;
      case GLUT_KEY_DOWN:
	shiftY -= 20.0;
	break;
      default:
	return;
    }

    glutPostRedisplay();
}

static void Key(unsigned char key, int x, int y)
{

    switch (key) {
      case 27:
        exit(1);

      case 'n':
	shiftZ += 20.0;
	break;
      case 'm':
	shiftZ -= 20.0;
	break;

      case 'q':
	scaleX -= 0.1;
	if (scaleX < 0.1) {
	    scaleX = 0.1;
	}
	break;
      case 'w':
	scaleX += 0.1;
	break;
      case 'a':
	scaleY -= 0.1;
	if (scaleY < 0.1) {
	    scaleY = 0.1;
	}
	break;
      case 's':
	scaleY += 0.1;
	break;
      case 'z':
	scaleZ -= 0.1;
	if (scaleZ < 0.1) {
	    scaleZ = 0.1;
	}
	break;
      case 'x':
	scaleZ += 0.1;
	break;

      case 'e':
	angleX -= 5.0;
	if (angleX < 0.0) {
	    angleX = 360.0 + angleX;
	}
	break;
      case 'r':
	angleX += 5.0;
	if (angleX > 360.0) {
	    angleX = angleX - 360.0;
	}
	break;
      case 'd':
	angleY -= 5.0;
	if (angleY < 0.0) {
	    angleY = 360.0 + angleY;
	}
	break;
      case 'f':
	angleY += 5.0;
	if (angleY > 360.0) {
	    angleY = angleY - 360.0;
	}
	break;
      case 'c':
	angleZ -= 5.0;
	if (angleZ < 0.0) {
	    angleZ = 360.0 + angleZ;
	}
	break;
      case 'v':
	angleZ += 5.0;
	if (angleZ > 360.0) {
	    angleZ = angleZ - 360.0;
	}
	break;

	case ' ':
	numpoints=drawWrap(somepoints,numpoints);
	break;

      default:
	return;
    }

    glutPostRedisplay();
}

static GLenum Args(int argc, char **argv)
{
    return GL_TRUE;
}

static void Draw(void)
{
	int j;
	char buff[1024];
    glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0,1.0,1.0);

    glPushMatrix();

    glTranslatef(shiftX, shiftY, shiftZ);
    glRotatef(angleX, 1.0, 0.0, 0.0);
    glRotatef(angleY, 0.0, 1.0, 0.0);
    glRotatef(angleZ, 0.0, 0.0, 1.0);
    glScalef(scaleX, scaleY, scaleZ);

    glPushMatrix();
                                        drawCircle(100.0,100.0,0,10);

                                glBegin(GL_TRIANGLE_FAN);
                                glVertex3f(70,600,0);
                                for(j=0;j<numpoints;j++)
                                {
                                        glVertex3f(somepoints[j].x,somepoints[j].y,0);
                                }
                                glVertex3f(somepoints[0].x,somepoints[0].y,0);
                                glEnd();

				glColor3f(1.0,0.0,0.0);

                                for(j=0;j<numpoints;j++)
                                {
                                        drawCircle(somepoints[j].x,somepoints[j].y,0,10);
					sprintf(buff,"%d",j);
					drawText(somepoints[j].x,somepoints[j].y,0,TEXT_SCALE,buff);
                                }



    glPopMatrix();

    glPopMatrix();

    glFlush();

	glutSwapBuffers();
}


int main(int argc, char **argv)
{
	int i;
    glutInit(&argc, argv);

    if (Args(argc, argv) == GL_FALSE) {
	exit(1);
    }

	srand48(42);    /* want to be deterministic */

	numpoints=8;
	for(i=0;i<numpoints;i++)
	{
		somepoints[i].x=drand48()*800;
		somepoints[i].y=drand48()*800;
	}

//  awk ' { print "somepoints[i].x=" $5 "; somepoints[i].y=" $6 "; i++; " } ' foo
	i=0;
somepoints[i].x=9; somepoints[i].y=553; i++; 
somepoints[i].x=129; somepoints[i].y=581; i++; 
somepoints[i].x=141; somepoints[i].y=577; i++; 
somepoints[i].x=99; somepoints[i].y=561; i++; 
somepoints[i].x=128; somepoints[i].y=591; i++; 
somepoints[i].x=144; somepoints[i].y=650; i++; 
somepoints[i].x=7; somepoints[i].y=635; i++; 
somepoints[i].x=11; somepoints[i].y=595; i++; 
somepoints[i].x=64; somepoints[i].y=685; i++; 




    glutInitWindowPosition(0, 0); glutInitWindowSize( 800, 800);

    glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);

    if (glutCreateWindow("MANET") == GL_FALSE) {
	exit(1);
    }

    Init();


    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutSpecialFunc(Key2);
    glutDisplayFunc(Draw);
    glutMainLoop();
	return 0;
}
