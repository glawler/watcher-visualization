/**
 * @file node.h 
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#ifndef YETANOTHERNODE_H
#define YETANOTHERNODE_H

#include <Ogre.h>
#include "logger.h"

namespace ogreWatcher
{
    /**
     * node
     * Keeps track of node attributes, location, etc
     */
    class node
    {
        public:

            node();
            virtual ~node();

        protected:

            /** Current Animation state of the node */
            Ogre::AnimationState *animationState;

            /** Node that is animated. */
            Ogre::Entity *entity;

            /** Scene node the node is attached to */
            Ogre::SceneNode *sceneNode;

            /** Speed the node is moving */
            Ogre::Real speed; 

        private:

            DECLARE_LOGGER();

    }; 
}

#endif //  YETANOTHERNODE_H

