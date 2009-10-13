/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file edge.h 
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#ifndef YETANOTHEREDGE_H
#define YETANOTHEREDGE_H

#include <utility>
#include <Ogre.h>
#include "declareLogger.h"

#include "node.h"

namespace ogreWatcher
{
    /**
     * edge
     * Keeps track of edge attributes, location, etc
     */
    class edge
    {
        public:

            edge();
            virtual ~edge();

        protected:

            /** The edge Entity */
            Ogre::Entity *entity;

            /** Edge is attached to a pair of nodes */
            std::pair<Ogre::Entity *, Ogre::Entity *> nodes;

            /** Scene Node the edge is attached to */
            Ogre::SceneNode *sceneNode;

        private:

            DECLARE_LOGGER();

    }; 
}

#endif //  YETANOTHEREDGE_H

