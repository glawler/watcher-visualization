/**
 * @file edge.h 
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#ifndef YETANOTHEREDGE_H
#define YETANOTHEREDGE_H

#include <utility>
#include <Ogre.h>
#include "logger.h"

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

