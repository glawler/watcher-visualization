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
