
#ifndef NODE_ACTOR_PROXY_H
#define NODE_ACTOR_PROXY_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/transformableactorproxy.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "nodeActor.h"

class NodeActorProxy : public dtDAL::TransformableActorProxy
{
    public:
        NodeActorProxy(void);
        virtual ~NodeActorProxy(void);
        void CreateActor();

        void BuildPropertyMap();
        void SetTextureFile(const std::string &fileName);
    private:
        DECLARE_LOGGER();
};

#endif // NODE_ACTOR_PROXY_H
