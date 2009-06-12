
// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/datatype.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "nodeActorProxy.h"

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

    NodeActor *nodeActor = dynamic_cast<NodeActor*> (GetActor());

    // Add the "HeadPos" property
    AddProperty(new dtDAL::Vec3ActorProperty("pos","Position",
        dtDAL::MakeFunctor(*nodeActor,&NodeActor::SetPos),
        dtDAL::MakeFunctorRet(*nodeActor,&NodeActor::GetPos),
        "Sets/gets the node's position as a 3-dimensional vector (x,y,z) from the origin.", "Position"));

    TRACE_EXIT();
}

void NodeActorProxy::CreateActor()
{
    TRACE_ENTER();
    SetActor(*new NodeActor);
    TRACE_EXIT();
}

void NodeActorProxy::SetTextureFile(const std::string &fileName)
{
    TRACE_ENTER();
    TRACE_EXIT();
}
