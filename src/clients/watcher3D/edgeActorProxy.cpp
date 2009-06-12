
// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/datatype.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "edgeActorProxy.h"

INIT_LOGGER(EdgeActorProxy, "EdgeActorProxy");

EdgeActorProxy::EdgeActorProxy()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

EdgeActorProxy::~EdgeActorProxy()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void EdgeActorProxy::BuildPropertyMap()
{
    TRACE_ENTER();
    TransformableActorProxy::BuildPropertyMap();

    EdgeActor *edgeActor = dynamic_cast<EdgeActor*> (GetActor());

    // Add the "HeadPos" property
    AddProperty(new dtDAL::Vec3ActorProperty("HeadPos","Head Position",
        dtDAL::MakeFunctor(*edgeActor,&EdgeActor::SetHeadPos),
        dtDAL::MakeFunctorRet(*edgeActor,&EdgeActor::GetHeadPos),
        "Sets/gets the edge's head position as a 3-dimensional vector (x,y,z) from the origin.", "Position"));

    // Add the "TailPos" property
    AddProperty(new dtDAL::Vec3ActorProperty("TailPos","Tail Position",
        dtDAL::MakeFunctor(*edgeActor,&EdgeActor::SetTailPos),
        dtDAL::MakeFunctorRet(*edgeActor,&EdgeActor::GetTailPos),
        "Sets/gets the edge's tail position as a 3-dimensional vector (x,y,z) from the origin.", "Position"));
    TRACE_EXIT();
}

void EdgeActorProxy::CreateActor()
{
    TRACE_ENTER();
    SetActor(*new EdgeActor);
    TRACE_EXIT();
}

void SetTextureFile(const std::string &fileName)
{
    TRACE_ENTER();
    TRACE_EXIT();
}
