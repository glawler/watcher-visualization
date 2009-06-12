
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
    TRACE_ENTER();
    if(registry)
        delete registry;
    TRACE_EXIT();
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
    dtDAL::ActorType *mNodeActorType = new dtDAL::ActorType("Node", "Watcher3D Actors", 
        "This actor represents a node.");
    mActorFactory->RegisterType<NodeActorProxy> (mNodeActorType);
    dtDAL::ActorType *mEdgeActorType = new dtDAL::ActorType("Edge", "Watcher3D Actors", 
        "This actor represents an edge.");
    mActorFactory->RegisterType<EdgeActorProxy> (mEdgeActorType);
    TRACE_EXIT();
}

