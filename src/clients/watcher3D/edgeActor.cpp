
#include <iostream>

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <dtCore/transform.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "edgeActor.h"

INIT_LOGGER(EdgeActor, "EdgeActor");

EdgeActor::EdgeActor() :
geode(NULL)
{
    TRACE_ENTER();
    SetName("Edge");
    osg::Geometry *geometry = new osg::Geometry;
    geode = new osg::Geode;
    geode->addDrawable(geometry);
    GetMatrixNode()->addChild(geode);
    TRACE_EXIT();
}

EdgeActor::~EdgeActor()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void EdgeActor::DrawEdge()
{
    TRACE_ENTER();
    if(geode && geode->getDrawable(0))
        geode->removeDrawable(geode->getDrawable(0));

    osg::Geometry *g = new osg::Geometry;
    osg::Vec3Array *v = new osg::Vec3Array;

    // Get the position (translation) of the Edge Actor relative to the origin.
    dtCore::Transform edgeActorTrans;
    GetTransform(edgeActorTrans);
    osg::Vec3 edgeActorPos = edgeActorTrans.GetTranslation();

    // v->push_back(headPos-edgeActorPos);
    // v->push_back(tailPos-edgeActorPos);
    osg::Vec3 a(0.0f,0.0f,500.0f);
    osg::Vec3 b(0.0f,50.0f,-50.0f);
    v->push_back(a);
    v->push_back(b);
    osg::DrawArrays *da = new osg::DrawArrays(osg::PrimitiveSet::LINES,0,v->size());
    g->setVertexArray(v);
    g->addPrimitiveSet(da);
    geode->addDrawable(g);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF); 

    GetMatrixNode()->addChild(geode);
    TRACE_EXIT();
}
