
#ifndef NODE_ACTOR_PROXY_H
#define NODE_ACTOR_PROXY_H

#include <dtDAL/transformableactorproxy.h>
#include "nodeActor.h"

class NodeActorProxy : public dtDAL::TransformableActorProxy
{
    public:
        NodeActorProxy(void);
        virtual ~NodeActorProxy(void);
        void CreateActor();

        void BuildPropertyMap();
        void SetTextureFile(const std::string &fileName);
};

#endif // NODE_ACTOR_PROXY_H
