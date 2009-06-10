
#ifndef EDGE_ACTOR_H
#define EDGE_ACTOR_H

#include <dtCore/transformable.h>
#include <osg/Geode>
#include <osg/Vec3>

class EdgeActor : public dtCore::Transformable
{
    public:
        EdgeActor();
        virtual ~EdgeActor();

        void DrawEdge();

        // Accessors
        inline osg::Vec3 GetHead() { return headPos; }
        inline osg::Vec3 GetTail() { return tailPos; }

        // Mutators
        inline void SetHead(const osg::Vec3& newHeadPos) { headPos = newHeadPos; DrawEdge(); }
        inline void SetTail(const osg::Vec3& newTailPos) { tailPos = newTailPos; DrawEdge(); }

    private:
        osg::Vec3 headPos;
        osg::Vec3 tailPos;
};

#endif // EDGE_ACTOR_H
