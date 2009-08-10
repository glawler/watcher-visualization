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


#include <iostream>

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <dtGame/gameactor.h>
#include <dtCore/transform.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "edgeActor.h"

INIT_LOGGER(EdgeActor, "EdgeActor");

EdgeActor::EdgeActor(dtGame::GameActorProxy& gameActorProxy) :
dtGame::GameActor(gameActorProxy),
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

void EdgeActor::TickLocal(const dtGame::Message &tickMessage)
{
    DrawEdge();
}

void EdgeActor::TickRemote(const dtGame::Message &tickMessage)
{
    DrawEdge();
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

    v->push_back(headPos-edgeActorPos);
    v->push_back(tailPos-edgeActorPos);
    osg::DrawArrays *da = new osg::DrawArrays(osg::PrimitiveSet::LINES,0,v->size());
    g->setVertexArray(v);
    g->addPrimitiveSet(da);
    geode->addDrawable(g);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF); 

    GetMatrixNode()->addChild(geode);
    TRACE_EXIT();
}
