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


#ifndef EDGE_ACTOR_H
#define EDGE_ACTOR_H

// Watcher includes
#include "declareLogger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtGame/gameactor.h>
#include <osg/Geode>
#include <osg/Vec3>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class EdgeActor : public dtGame::GameActor
{
    public:
        EdgeActor(dtGame::GameActorProxy& gameActorProxy);

        virtual void TickRemote(const dtGame::Message &tickMessage);
        virtual void TickLocal(const dtGame::Message &tickMessage);
        void DrawEdge();

        inline osg::Vec3 GetHeadPos() { return headPos; }
        inline osg::Vec3 GetTailPos() { return tailPos; }
        inline void SetHeadPos(const osg::Vec3& newHeadPos) { headPos = newHeadPos; DrawEdge(); }
        inline void SetTailPos(const osg::Vec3& newTailPos) { tailPos = newTailPos; DrawEdge(); }

    protected:
        virtual ~EdgeActor();

    private:
        osg::Geode *geode;
        osg::Vec3 headPos;
        osg::Vec3 tailPos;
        DECLARE_LOGGER();
};

#endif // EDGE_ACTOR_H
