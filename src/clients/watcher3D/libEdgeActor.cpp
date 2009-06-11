
// Watcher3D includes
#include "libEdgeActor.h"
#include "edgeActorProxy.h"

extern "C" DT_PLUGIN_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
{
    return new LibEdgeActor;
}

extern "C" DT_PLUGIN_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
{
    if(registry)
        delete registry;
}

LibEdgeActor::LibEdgeActor() : dtDAL::ActorPluginRegistry("Edge Actor Library")
{
    mDescription = "This library exports the edge actor";
}

void LibEdgeActor::RegisterActorTypes()
{
    dtDAL::ActorType *mEdgeActorType = new dtDAL::ActorType("Edge", "Watcher3D Actors", 
        "This actor represents an edge.");
    mActorFactory->RegisterType<EdgeActorProxy> (mEdgeActorType);
}

