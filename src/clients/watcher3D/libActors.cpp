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


// Watcher includes
#include "logger.h"

// Watcher3D includes
#include "libActors.h"
#include "nodeActorProxy.h"
#include "edgeActorProxy.h"

INIT_LOGGER(LibActors, "LibActors");

extern "C" DT_PLUGIN_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
{
    TRACE_ENTER();
    return new LibActors;
    TRACE_EXIT();
}

extern "C" DT_PLUGIN_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
{
    // The trace macros cause a segmentation fault on program exit.
    // TRACE_ENTER();
    if(registry)
        delete registry;
    // TRACE_EXIT();
}

LibActors::LibActors() : dtDAL::ActorPluginRegistry("Watcher3D Actors Library")
{
    TRACE_ENTER();
    mDescription = "This library exports the Watcher3D actors";
    TRACE_EXIT();
}

void LibActors::RegisterActorTypes()
{
    TRACE_ENTER();
    dtDAL::ActorType *nodeActorType = new dtDAL::ActorType("Node", "Watcher3D Actors", 
        "This actor represents a node.");
    mActorFactory->RegisterType<NodeActorProxy> (nodeActorType);
    dtDAL::ActorType *edgeActorType = new dtDAL::ActorType("Edge", "Watcher3D Actors", 
        "This actor represents an edge.");
    mActorFactory->RegisterType<EdgeActorProxy> (edgeActorType);
    TRACE_EXIT();
}

