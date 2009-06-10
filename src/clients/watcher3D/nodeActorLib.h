
#ifndef NODE_ACTOR_LIB_H
#define NODE_ACTOR_LIB_H

class DT_PLUGIN_EXPORT LibNodeActor : public dtDAL::ActorPluginRegistry
{
    public:
        // Constructs our registry. Creates the actor types easy access when needed.
        LibNodeActor();

        // Registers actor types with the actor factory in the super class.
        // virtual void RegisterActorTypes();
        void RegisterActorTypes();

   private:
       dtCore::RefPtr<dtDAL::ActorType> mNodeActorType;
};

#endif // NODE_ACTOR_LIB_H
