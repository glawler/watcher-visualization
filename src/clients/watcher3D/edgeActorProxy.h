
#ifndef EDGE_ACTOR_PROXY_H
#define EDGE_ACTOR_PROXY_H

#include <dtDAL/transformableactorproxy.h>
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
