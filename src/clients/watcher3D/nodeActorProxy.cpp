
#include <iostream>

// Watcher3D includes
#include "nodeActorProxy.h"

NodeActorProxy::NodeActorProxy()
{
}

NodeActorProxy::~NodeActorProxy()
{
}

void NodeActorProxy::BuildPropertyMap()
{
    TransformableActorProxy::BuildPropertyMap();
}

void NodeActorProxy::CreateActor()
{
    SetActor(*new NodeActor);
}

void NodeActorProxy::SetTextureFile(const std::string &fileName)
{
}
