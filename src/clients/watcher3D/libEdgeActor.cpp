
// Watcher includes
#include "logger.h"

// Watcher3D includes
#include "libEdgeActor.h"
#include "edgeActorProxy.h"

INIT_LOGGER(LibEdgeActor, "LibEdgeActor");

extern "C" DT_PLUGIN_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
{
    TRACE_ENTER();
    return new LibEdgeActor;
    TRACE_EXIT();
}

extern "C" DT_PLUGIN_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
{
    TRACE_ENTER();
    if(registry)
        delete registry;
    TRACE_EXIT();
}

LibEdgeActor::LibEdgeActor() : dtDAL::ActorPluginRegistry("Edge Actor Library")
{
    TRACE_ENTER();
    mDescription = "This library exports the edge actor";
    TRACE_EXIT();
}

void LibEdgeActor::RegisterActorTypes()
{
    TRACE_ENTER();
    dtDAL::ActorType *mEdgeActorType = new dtDAL::ActorType("Edge", "Watcher3D Actors", 
        "This actor represents an edge.");
    mActorFactory->RegisterType<EdgeActorProxy> (mEdgeActorType);
    TRACE_EXIT();
}

