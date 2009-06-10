
#ifndef NODE_ACTOR_H
#define NODE_ACTOR_H

#include <dtCore/transformable.h>
#include <osg/Geode>
#include <osg/Vec3>

class NodeActor : public dtCore::Transformable
{
    public:
        NodeActor();
        virtual ~NodeActor();

        void DrawNode();

        // Accessors
        inline osg::Vec3 GetPos() { return pos; }

        // Mutators
        inline void SetPos(const osg::Vec3& newPos) { pos = newHeadPos; DrawNode(); }

    private:
        osg::Vec3 headPos;
        osg::Vec3 tailPos;
};

#endif // NODE_ACTOR_H
