
#ifndef LIB_ACTORS_H
#define LIB_ACTORS_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/plugin_export.h>
#include <dtDAL/actorpluginregistry.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class DT_PLUGIN_EXPORT LibActors : public dtDAL::ActorPluginRegistry
{
    public:
        // Constructs our registry. Creates the actor types easy access when needed.
        LibActors();

        // Registers actor types with the actor factory in the super class.
        // virtual void RegisterActorTypes();
        void RegisterActorTypes();

    private:
        dtCore::RefPtr<dtDAL::ActorType> mNodeActorType;
        dtCore::RefPtr<dtDAL::ActorType> mEdgeActorType;
        DECLARE_LOGGER();
};

#endif // LIB_ACTORS_H
