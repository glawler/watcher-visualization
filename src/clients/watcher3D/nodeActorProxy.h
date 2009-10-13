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


#ifndef NODE_ACTOR_PROXY_H
#define NODE_ACTOR_PROXY_H

// Watcher includes
#include "declareLogger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtActors/gamemeshactor.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class NodeActorProxy : public dtActors::GameMeshActorProxy
{
    public:
        NodeActorProxy();

        virtual void BuildPropertyMap();
        virtual void CreateActor();
        virtual void OnEnteredWorld();

    protected:
        virtual ~NodeActorProxy();

    private:
        DECLARE_LOGGER();
};

#endif // NODE_ACTOR_PROXY_H
