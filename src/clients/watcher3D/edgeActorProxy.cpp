
// Watcher3D includes
#include "edgeActorProxy.h"

EdgeActorProxy::EdgeActorProxy()
{
}

EdgeActorProxy::~EdgeActorProxy()
{
}

void EdgeActorProxy::BuildPropertyMap()
{
    TransformableActorProxy::BuildPropertyMap();
}

void EdgeActorProxy::CreateActor()
{
    SetActor(*new EdgeActor);
}

void SetTextureFile(const std::string &fileName)
{
}
