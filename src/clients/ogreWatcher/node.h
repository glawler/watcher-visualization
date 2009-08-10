/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

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

