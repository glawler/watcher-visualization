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
 * @file layerPreviewWindow.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-07-13
 */
#include "layerPreviewWindow.h"
#include "libwatcher/labelMessage.h"

#define UNUSED(e) do { (void)(e); } while (0)

namespace watcher
{
    using namespace event; 

    INIT_LOGGER(LayerPreviewWindow, "LayerPreviewWindow"); 

    // lives in manetGLView.cpp
    extern float fast_arctan2( float y, float x ); 

    LayerPreviewWindow::LayerPreviewWindow(QWidget *parent) : 
        QGLWidget(parent), node1(), node2(), layer(NULL)
    {
    }

    LayerPreviewWindow::~LayerPreviewWindow()
    {
    }

    void LayerPreviewWindow::setLayerData(WatcherLayerData *l) 
    {
        UNUSED(l);
        // layer=l;
        // layer->clear(); 

        // node1.loadConfiguration(event::PHYSICAL_LAYER, NodeIdentifier::from_string("192.168.1.101")); 
        // node2.loadConfiguration(event::PHYSICAL_LAYER, NodeIdentifier::from_string("192.168.1.102")); 

        // // add test node label, and test floating label 
        // LabelMessagePtr mess=LabelMessagePtr(new LabelMessage("Floating Label")); 
        // mess->lat=1.0;
        // mess->lng=1.0;
        // mess->alt=1.0; 
        // mess->addLabel=true;
        // layer->addRemoveFloatingLabel(mess, true); 

        // mess->lat=0.0;
        // mess->lng=0.0;
        // mess->alt=0.0; 
        // mess->label="Node Label"; 
        // mess->fromNodeID=node1.nodeId;
        // layer->addRemoveFloatingLabel(mess, true); 
    }

    void LayerPreviewWindow::initializeGL()
    {
        // TRACE_ENTER();
        // GLfloat matShine=0.6;
        // GLfloat specReflection[] = { 0.05, 0.05, 0.05, 1.0f };
        // GLfloat globalAmbientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        // GLfloat posLight0[]={ 50.0f, 50.0f, 000.0f, 1.0f };
        // GLfloat ambLight0[]={ 0.25, 0.25, 0.25, 1.0f };
        // GLfloat specLight0[]={ 0.1f, 0.1f, 0.1f, 1.0f };
        // GLfloat diffLight0[]={ 0.05, 0.05, 0.05, 1.0f };

        // glEnable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LESS); 

        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // // glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE); 

        // glEnable(GL_TEXTURE_2D);

        // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

        // glShadeModel(GL_SMOOTH); 
        // // glShadeModel(GL_FLAT); 

        // // LIGHT0
        // glLightfv(GL_LIGHT0, GL_POSITION, posLight0);
        // glLightfv(GL_LIGHT0, GL_AMBIENT, ambLight0); 
        // glLightfv(GL_LIGHT0, GL_SPECULAR, specLight0);
        // glLightfv(GL_LIGHT0, GL_DIFFUSE, diffLight0);

        // glEnable(GL_LIGHTING); 
        // glEnable(GL_LIGHT0); 

        // glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
        // glEnable(GL_COLOR_MATERIAL);

        // glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
        // glMaterialf(GL_FRONT, GL_SHININESS, matShine);

        // TRACE_EXIT();
    }
    void LayerPreviewWindow::resizeGL(int w, int h)
    {
        UNUSED(w);
        UNUSED(h);
        // glViewport(0, 0, w, h);
        // glMatrixMode(GL_PROJECTION);
        // glLoadIdentity();
        // gluPerspective(40.0, GLfloat(w)/GLfloat(h), 1.0, 50.0);
    }
    void LayerPreviewWindow::paintGL()
    {

    }
    void LayerPreviewWindow::drawGraph() 
    {
        // drawNode(node1); 
        // drawNode(node2); 
        // if (!layer)
        //     return; 
        // drawEdge(layer->edgeDisplayInfo, node1, node2); 
        // {
        //     WatcherLayerData::ReadLock readLock(layer->floatingLabelsMutex); 
        //     drawLabel(layer->floatingLabels.begin()->lat, layer->floatingLabels.begin()->lng, layer->floatingLabels.begin()->alt, 
        //             *layer->floatingLabels.begin()); 
        // }
        // {
        //     WatcherLayerData::ReadLock readLock(layer->nodeLabelsMutexes[0]); 
        //     drawLabel(node1.x, node1.y, node1.z, *layer->nodeLabels[0].begin()); 
        // }
    }
    void LayerPreviewWindow::drawNode(const NodeDisplayInfo &node) 
    {
        UNUSED(node);
        // GLdouble x=node.x;
        // GLdouble y=node.y;
        // GLdouble z=node.z;
        // GLfloat nodeColor[]={
        //     node.color.r/255.0, 
        //     node.color.g/255.0, 
        //     node.color.b/255.0, 
        //     node.color.a/255.0
        // };

        // glPushMatrix();
        // glTranslated(x, y, z);
        // glColor4fv(nodeColor);
        // GLUquadricObj *quadric=gluNewQuadric();
        // gluQuadricNormals(quadric, GLU_SMOOTH);
        // gluSphere(quadric, 4, 15, 15);
        // gluDeleteQuadric(quadric);

        // renderText(0, 6, 3, QString(node.get_label().c_str()),
        //         QFont(node.labelFont.c_str(), static_cast<int>(node.labelPointSize))); 

        // glPopMatrix(); 
    }
    void LayerPreviewWindow::drawEdge(const EdgeDisplayInfo &edge, const NodeDisplayInfo &node1, const NodeDisplayInfo &node2)
    {
        UNUSED(edge);
        UNUSED(node1);
        UNUSED(node2);
        // GLdouble x1=node1.x;
        // GLdouble y1=node1.y;
        // GLdouble z1=node1.z;
        // GLdouble x2=node2.x;
        // GLdouble y2=node2.y;
        // GLdouble z2=node2.z;

        // double width=edge.width;

        // GLfloat edgeColor[]={
        //     edge.color.r/255.0, 
        //     edge.color.g/255.0, 
        //     edge.color.b/255.0, 
        //     edge.color.a/255.0, 
        // };

        // glColor4fv(edgeColor);

        // GLUquadricObj *quadric=gluNewQuadric();
        // gluQuadricNormals(quadric, GLU_SMOOTH);

        // float vx = x2-x1;
        // float vy = y2-y1;
        // float vz = z2-z1;

        // //handle the degenerate case of z1 == z2 with an approximation
        // if(vz == 0)
        //     vz = .0001;

        // float v = sqrt( vx*vx + vy*vy + vz*vz );
        // float ax = 57.2957795*acos( vz/v );
        // if ( vz < 0.0 )
        //     ax = -ax;
        // float rx = -vy*vz;
        // float ry = vx*vz;

        // glPushMatrix();

        // //draw the cylinder body
        // glTranslatef( x1,y1,z1 );
        // glRotatef(ax, rx, ry, 0.0);
        // gluQuadricOrientation(quadric,GLU_OUTSIDE);
        // gluCylinder(quadric, width, 0, v, 15, 15);

        // glPopMatrix();

        // gluDeleteQuadric(quadric);

        // // draw the edge's label, if there is one.
        // if (edge.label!="none") {
        //     const GLfloat clr[]={
        //         edge.labelColor.r/255.0, 
        //         edge.labelColor.g/255.0, 
        //         edge.labelColor.b/255.0, 
        //         edge.labelColor.a/255.0
        //     };
        //     glColor4fv(clr);

        //     GLdouble lx=(x1+x2)/2.0; 
        //     GLdouble ly=(y1+y2)/2.0; 
        //     GLdouble lz=(z1+z2)/2.0; 
        //     GLdouble a=fast_arctan2(x1-x2 , y1-y2);
        //     GLdouble th=10.0;
        //     renderText(lx+sin(a-M_PI_2),ly+cos(a-M_PI_2)*th, lz, 
        //             QString(edge.label.c_str()),
        //             QFont(QString(edge.labelFont.c_str()), (int)(edge.labelPointSize)));
        // }
    }

    void LayerPreviewWindow::drawLabel(const GLfloat &x, const GLfloat &y, const GLfloat &z, const LabelDisplayInfo &l) 
    {
        UNUSED(x);
        UNUSED(y);
        UNUSED(z);
        UNUSED(l);
        // int fgColor[]={
        //     l.foregroundColor.r, 
        //     l.foregroundColor.g, 
        //     l.foregroundColor.b, 
        //     l.foregroundColor.a
        // };
        // float offset=4.0;

        // QFont f(l.fontName.c_str(), (int)(l.pointSize)); 
        // qglColor(QColor(fgColor[0], fgColor[1], fgColor[2], fgColor[3])); 
        // renderText(x+offset, y+offset, z+2.0, l.labelText.c_str(), f); 
    }

} // namespace watcher

