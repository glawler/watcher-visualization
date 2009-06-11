
#include <iostream>

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
