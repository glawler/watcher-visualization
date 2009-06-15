
#ifndef EDGE_ACTOR_PROXY_H
#define EDGE_ACTOR_PROXY_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtGame/gameactor.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "edgeActor.h"

class EdgeActorProxy : public dtGame::GameActorProxy
{
    public:
        EdgeActorProxy(void);

        virtual void BuildPropertyMap();
        virtual void CreateActor();
        virtual void OnEnteredWorld();

    protected:
        virtual ~EdgeActorProxy(void);

    private:
        DECLARE_LOGGER();
};

#endif // EDGE_ACTOR_PROXY_H
