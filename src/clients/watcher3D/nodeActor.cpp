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


// Watcher includes
#include "logger.h"

// Watcher3D includes
#include "nodeActor.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtGame/gameactor.h>
#include <dtCore/isector.h>
#include <dtCore/transform.h>
#include <dtGame/gamemanager.h>
#include <osg/Vec3>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

INIT_LOGGER(NodeActor, "NodeActor");

NodeActor::NodeActor(dtGame::GameActorProxy& gameActorProxy) :
GameMeshActor(gameActorProxy)
{
    TRACE_ENTER();
    SetName("Node");
    SetMesh("models/brdm.ive");
    TRACE_EXIT();
}

NodeActor::~NodeActor()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void NodeActor::TickLocal(const dtGame::Message &tickMessage)
{
    DrawNode();
}

void NodeActor::TickRemote(const dtGame::Message &tickMessage)
{
    DrawNode();
}

void NodeActor::DrawNode()
{
    TRACE_ENTER();

    // Clamp node actor to the ground
    dtCore::Transform tx;
    GetTransform(tx);
    osg::Vec3 pos;
    tx.GetTranslation(pos);
    dtCore::RefPtr<dtCore::Isector> query = new 
          dtCore::Isector((&GetGameActorProxy().GetGameManager()->GetScene()));
    query->Reset();
    query->SetStartPosition(osg::Vec3(pos.x(),pos.y(),-10000));
    query->SetDirection(osg::Vec3(0,0,1));
    if (query.get()->Update())
    {
        osgUtil::IntersectVisitor &iv = query.get()->GetIntersectVisitor();
        osg::Vec3 p = iv.getHitList(query.get()->GetLineSegment())[0].getWorldIntersectPoint();
        pos.z() = p.z();
    }
    tx.SetTranslation(pos);
    SetTransform(tx);

    TRACE_EXIT();
}

osg::Vec3 NodeActor::GetPos()
{
    dtCore::Transform tx;
    GetTransform(tx);
    osg::Vec3 pos;
    tx.GetTranslation(pos);
    return pos;
}

void NodeActor::SetPos(const osg::Vec3& newPos)
{
    dtCore::Transform tx;
    tx.SetTranslation(newPos);
    SetTransform(tx);
    DrawNode();
}
