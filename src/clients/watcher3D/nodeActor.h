
#ifndef NODE_ACTOR_H
#define NODE_ACTOR_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtCore/transformable.h>
#include <osg/Geode>
#include <osg/Vec3>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class NodeActor : public dtCore::Transformable
{
    public:
        NodeActor();
        virtual ~NodeActor();

        void DrawNode();

        // Accessors
        inline osg::Vec3 GetPos() { return pos; }

        // Mutators
        inline void SetPos(const osg::Vec3& newPos) { pos = newPos; DrawNode(); }

    private:
        osg::Vec3 pos;
        DECLARE_LOGGER();
};

#endif // NODE_ACTOR_H
