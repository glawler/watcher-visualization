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


// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/datatype.h>
#include <dtGame/messagetype.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "nodeActorProxy.h"
#include "nodeActor.h"

INIT_LOGGER(NodeActorProxy, "NodeActorProxy");

NodeActorProxy::NodeActorProxy()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

NodeActorProxy::~NodeActorProxy()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void NodeActorProxy::BuildPropertyMap()
{
    TRACE_ENTER();
    TransformableActorProxy::BuildPropertyMap();

    NodeActor *nodeActor = dynamic_cast<NodeActor*>(GetActor());

    // Add the "pos" property
    AddProperty(new dtDAL::Vec3ActorProperty("pos", "Position",
        dtDAL::MakeFunctor(*nodeActor,&NodeActor::SetPos),
        dtDAL::MakeFunctorRet(*nodeActor,&NodeActor::GetPos),
        "Sets/gets the node's position as a 3-dimensional vector (x,y,z) from the origin.", "Position"));

    TRACE_EXIT();
}

void NodeActorProxy::OnEnteredWorld()
{
   // Register for game events
   RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

   // Register for ticks
   if (IsRemote())
      RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
   else
      RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);

   dtActors::GameMeshActorProxy::OnEnteredWorld();
}

void NodeActorProxy::CreateActor()
{
    TRACE_ENTER();
    SetActor(*new NodeActor(*this));
    TRACE_EXIT();
}

