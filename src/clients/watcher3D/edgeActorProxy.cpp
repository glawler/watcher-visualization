
// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/messagetype.h>
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
    AddProperty(new dtDAL::Vec3ActorProperty("headPos","Head Position",
        dtDAL::MakeFunctor(*edgeActor,&EdgeActor::SetHeadPos),
        dtDAL::MakeFunctorRet(*edgeActor,&EdgeActor::GetHeadPos),
        "Sets/gets the edge's head position as a 3-dimensional vector (x,y,z) from the origin.", "Position"));

    // Add the "TailPos" property
    AddProperty(new dtDAL::Vec3ActorProperty("tailPos","Tail Position",
        dtDAL::MakeFunctor(*edgeActor,&EdgeActor::SetTailPos),
        dtDAL::MakeFunctorRet(*edgeActor,&EdgeActor::GetTailPos),
        "Sets/gets the edge's tail position as a 3-dimensional vector (x,y,z) from the origin.", "Position"));
    TRACE_EXIT();
}

void EdgeActorProxy::OnEnteredWorld()
{
   // Register for game events
   RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

   // Register for ticks
   if (IsRemote())
      RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
   else
      RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);

   dtGame::GameActorProxy::OnEnteredWorld();
}

void EdgeActorProxy::CreateActor()
{
    TRACE_ENTER();
    SetActor(*new EdgeActor(*this));
    TRACE_EXIT();
}

