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


#ifndef EDGE_ACTOR_PROXY_H
#define EDGE_ACTOR_PROXY_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtGame/gameactor.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Watcher3D includes
#include "edgeActor.h"

class EdgeActorProxy : public dtGame::GameActorProxy
{
    public:
        EdgeActorProxy(void);

        virtual void BuildPropertyMap();
        virtual void CreateActor();
        virtual void OnEnteredWorld();

    protected:
        virtual ~EdgeActorProxy(void);

    private:
        DECLARE_LOGGER();
};

#endif // EDGE_ACTOR_PROXY_H
