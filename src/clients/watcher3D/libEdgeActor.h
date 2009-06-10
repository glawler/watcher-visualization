
#ifndef LIB_EDGE_ACTOR_H
#define LIB_EDGE_ACTOR_H

#include <dtDAL/plugin_export.h>
#include <dtDAL/actorpluginregistry.h>

class DT_PLUGIN_EXPORT LibEdgeActor : public dtDAL::ActorPluginRegistry
{
    public:
        // Constructs our registry. Creates the actor types easy access when needed.
        LibEdgeActor();

        // Registers actor types with the actor factory in the super class.
        // virtual void RegisterActorTypes();
        void RegisterActorTypes();

   private:
       dtCore::RefPtr<dtDAL::ActorType> mEdgeActorType;
};

#endif // LIB_EDGE_ACTOR_H
