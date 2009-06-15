
#ifndef NODE_ACTOR_H
#define NODE_ACTOR_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtActors/gamemeshactor.h>
#include <dtGame/gameactor.h>
#include <osg/Geode>
#include <osg/Vec3>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class NodeActor : public dtActors::GameMeshActor
{
    public:
        NodeActor(dtGame::GameActorProxy& gameActorProxy);

        virtual void TickRemote(const dtGame::Message &tickMessage);
        virtual void TickLocal(const dtGame::Message &tickMessage);
        void DrawNode();

        osg::Vec3 GetPos();
        void SetPos(const osg::Vec3& newPos);

    protected:
        virtual ~NodeActor(void);

    private:
        DECLARE_LOGGER();
};

#endif // NODE_ACTOR_H
