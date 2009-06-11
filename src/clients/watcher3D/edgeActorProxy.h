
#ifndef EDGE_ACTOR_PROXY_H
#define EDGE_ACTOR_PROXY_H

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/transformableactorproxy.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "edgeActor.h"

class EdgeActorProxy : public dtDAL::TransformableActorProxy
{
    public:
        EdgeActorProxy(void);
        virtual ~EdgeActorProxy(void);
        void CreateActor();

        void BuildPropertyMap();
        void SetTextureFile(const std::string &fileName);
};

#endif // EDGE_ACTOR_PROXY_H
