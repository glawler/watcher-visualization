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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "rng.h"

#include "des.h"
#include "config.h"
#include "graphics.h"
#include "mobility.h"
#include "packetapi.h"

#include "apisupport.h"
#include "node.h"

static const char *rcsid __attribute__ ((unused)) = "$Id: main.cpp,v 1.38 2007/07/23 16:59:50 dkindred Exp $";

typedef enum
{
	MANETLOSS_NONE,         /* do no loss at the simulator level  */
	MANETLOSS_PER,          /* do a packet error rate loss, in the simulator  */
	MANETLOSS_BER           /* do a bit error rate, with some FEC parameters  (unimplemented) */
} manetLossMode;

typedef struct ManetRFModelState
{
	manetLossMode lossMode;
	double lossPER;
	RNG *rfRNG;
} ManetRFModelState;

ManetRFModelState *manetRFModelInit(manet *m)
{
	ManetRFModelState *st;
	char const *mode;

	mode=configSearchStr(m->conf,"rf_mode");
	st=(ManetRFModelState*)malloc(sizeof(*st));

	if (mode==NULL)
		st->lossMode=MANETLOSS_NONE;
	else if (strcasecmp(mode,"per")==0)
		st->lossMode=MANETLOSS_PER;
	else if (strcasecmp(mode,"ber")==0)
		st->lossMode=MANETLOSS_BER;
	
	st->lossPER=configSetDouble(m->conf,"rf_per",0.0);
	st->rfRNG=new RNG(configSearchInt(m->conf,"rf_seed"));
	return st;
}

/* returns true if the packet in question should be dropped.
 */
int manetRFModelDrop(manet *m, packet *p)
{
	ManetRFModelState *st=m->rfState;
	int drop=0;
	
	switch(st->lossMode)
	{
		case MANETLOSS_NONE:
		break;
		case MANETLOSS_PER:
			if (st->rfRNG->rand_u01() < st->lossPER)
				drop=1;
		break;
		case MANETLOSS_BER:
			fprintf(stderr,"bit error rate loss model is unimplemented!\n");
			abort();
		break;
	}
	return drop;

}

/********************* end RF model stuff   ***********************************/

manet *themanet;
void checkIO(int arg);

#ifdef GRAPHICS

#include <GL/glut.h>

float angleX = 0.0, angleY = 0.0, angleZ = 0.0;
float scaleX = 0.02, scaleY = 0.02, scaleZ = 0.10;
float shiftX = 0.0, shiftY = 0.0, shiftZ = 0.0;


static void Reshape(int awidth, int aheight)
{
	glViewport(0, 0, awidth, aheight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, GLfloat(awidth)/GLfloat(aheight), 1.0, 50.0);
}

static void Key2(int key, int x, int y)
{

    switch (key) {
      case GLUT_KEY_LEFT:
	shiftX += 20.0;
	break;
      case GLUT_KEY_RIGHT:
	shiftX -= 20.0;
	break;
      case GLUT_KEY_UP:
	shiftY -= 20.0;
	break;
      case GLUT_KEY_DOWN:
	shiftY += 20.0;
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
		lastStep(themanet);
        exit(1);

      case 'n':
	shiftZ += 20.0;
	break;
      case 'm':
	shiftZ -= 20.0;
	break;

      case 'q':
	scaleX -= 0.001;
	if (scaleX < 0.001) {
	    scaleX = 0.001;
	}
	
	scaleY=scaleX;

	break;
      case 'w':
	scaleX += 0.001;
	scaleY=scaleX;

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
		/* Take a step */
		takeStep(themanet);
	break;

	case 'j':
		{
			int i;
			/* Take 50 steps */
			for(i=0;i<50;i++)
				takeStep(themanet);
		}
	break;
	case 'k':
		{
			int i;
			/* Take 1000 steps */
			for(i=0;i<1000;i++)
				takeStep(themanet);
		}
	break;
	case 't':
		{
			/* Step to the next second */
			int t;
			t=themanet->curtime/1000;
			while((themanet->curtime/1000) == t)
				takeStep(themanet);
		}
	break;
	case 'y':
		{
			/* Step to the next minute */
			int t;
			t=themanet->curtime/(1000*60);
			while((themanet->curtime/(1000*60)) == t)
				takeStep(themanet);
		}
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
	char buff[16];
	const GLfloat light_pos[] = { 0.0, 0.0, 0.0, 1.0 };
	const GLfloat blue[] = { 0.2f, 1.0f, 0.2f, 0.5f };
	const GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
	const GLfloat polished[] = { 50.0 };

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
        
        glTranslatef(0.0,0.0,-20.0);

        glPushMatrix();
        glTranslatef(0.0,0.0,-50.0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glPopMatrix();
        
        glScalef(scaleX, scaleY, scaleZ);
        glRotatef(angleX, 1.0, 0.0, 0.0);
        glRotatef(angleY, 0.0, 1.0, 0.0);
        glRotatef(angleZ, 0.0, 0.0, 1.0);
        glTranslatef(shiftX, shiftY, shiftZ);

	glPushMatrix();

	glColor3f(0.0,0.0,0.0);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
        glMaterialfv(GL_FRONT, GL_SPECULAR, white);
        glMaterialfv(GL_FRONT, GL_SHININESS, polished);

	sprintf(buff,"time: %lld",themanet->curtime);
	drawText(-40,10,0, 0.08, buff);

	manetDraw(themanet);

	glPopMatrix();

	glFlush();

	glutSwapBuffers();
}

#endif

int main(int argc, char **argv)
{
	int i;
	int time;
	Config *conf;
	const char *posWeightFile;
	char fullpath[PATH_MAX];

#if 0
/* Evil debugging hack, so that my stderr msgs are interleaved with amroute's stdout msgs */
	fclose(stderr);
	stderr=stdout;
#endif

	fprintf(stderr, "loading config: %s\n", argv[1]);
	conf=configLoad(argv[1]);

	if (argc<=1)
	{
		fprintf(stderr,"manet [conffile [duration]]\nconffile is the config file which describes the scenario\nduration is time in seconds to run\nIf duration is not specified the GUI will be run\nMetrics are on stdout, algorithm messages are on stderr\n");
		exit(1);
	}

        /* should make this seed selectable from config file or argument */
	srand48(0); /* want to be deterministic */

	themanet=manetInit(conf,0);

	themanet->rfState=manetRFModelInit(themanet);

	for(i=0;i<themanet->numnodes;i++)
		mobilityInit(&themanet->nlist[i]);

	firstStep(themanet,1);

	nodeMetrics(&(themanet->nlist[0]),1);
	nodeMobilityCountEdges(&(themanet->nlist[0]),1);
	nodeMobilityDumpCoords(&(themanet->nlist[0]),NULL);
	nodeHierarchyCountEdges(&(themanet->nlist[0]),1);

        posWeightFile = configSearchStr(conf,"positionweightfile");
	if (posWeightFile &&
	    0 == configGetPathname(conf,
				   posWeightFile,
				   fullpath,
				   sizeof(fullpath)))
	{
		posWeightFile = fullpath;
	}

	if (posWeightFile) 
	{
		fprintf(stderr, "%s: loading position weights from %s\n",
			__func__, posWeightFile);
	}
	for(i=0;i<themanet->numnodes;i++)
		packetApiPositionWeightLoad(&(themanet->nlist[i]),posWeightFile);

	takeStep(themanet);

	if (argc>2)
	{
		sscanf(argv[2],"%d",&time);
		time*=1000;     /* the time variable is actually in milli-ticks, not ticks */
		while(themanet->curtime <time)
			takeStep(themanet);
		lastStep(themanet);

		packetDumpDebug();
		exit(0);
	}


#ifdef GRAPHICS

	if (Args(argc, argv) == GL_FALSE)
	{
		exit(1);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize( 1000, 900);
	if (glutCreateWindow("MANET") == GL_FALSE)
	{
		exit(1);
	}

	GLfloat white[]={1.0,1.0,1.0,1.0};


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	shiftX=-(themanet->mobility->maxx/2);
	shiftY=-(themanet->mobility->maxy/2);

	glClearColor(1.0,1.0,1.0,0.0);

        glutTimerFunc(100,checkIO,0);

	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(Key2);
	glutDisplayFunc(Draw);
	glutMainLoop();
#else
	fprintf(stderr,"Graphics were not enabled at compile time.\n");
#endif
	return 0;
}

/* send a packet to all nodes which are withing this node's antenna radius
**  Note that that set includes this node.
*/
void packetSend(manetNode *us, packet *p, int origflag)
{
        int i;

//	statusCount(us, PACKET_RECEIVE, p);

#ifdef DEBUG_EVENTS
        fprintf(stderr,"node %d: packetSend: sending to %d enqueued at ",us->addr & 0xFF, p->dst & 0xFF);
#endif
        for(i=0;i<us->manet->numnodes;i++)
                if (us->manet->linklayergraph[us->index*us->manet->numnodes+i])
                { 
			if ((us!=&(us->manet->nlist[i])) && (manetRFModelDrop(us->manet,p)))   /* nodes have 0 packet loss with themselves...  */
			{
#ifdef DEBUG_EVENTS
				fprintf(stderr,"node %d: dropping packet to %d\n",us->addr & 0xFF ,us->manet->nlist[i].addr & 0xFF);
#endif
			}
			else
				packetEnqueue(&us->manet->nlist[i],p,1);   /* send packet to us->manet->nlist[i] */
#ifdef DEBUG_EVENTS
                        fprintf(stderr," %d",us->manet->nlist[i].addr & 0xFF);
#endif   
                }
#ifdef DEBUG_EVENTS
        fprintf(stderr,"\n");
#endif
}


void checkIO(int arg)
{
        glutTimerFunc(100,checkIO,0);

	manetIOCheck(themanet);
}

