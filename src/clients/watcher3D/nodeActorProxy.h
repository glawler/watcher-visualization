
#ifndef NODE_ACTOR_PROXY_H
#define NODE_ACTOR_PROXY_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtActors/gamemeshactor.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class NodeActorProxy : public dtActors::GameMeshActorProxy
{
    public:
        NodeActorProxy();

        virtual void BuildPropertyMap();
        virtual void CreateActor();
        virtual void OnEnteredWorld();

    protected:
        virtual ~NodeActorProxy();

    private:
        DECLARE_LOGGER();
};

#endif // NODE_ACTOR_PROXY_H
