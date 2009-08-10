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


#ifndef LIB_ACTORS_H
#define LIB_ACTORS_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/plugin_export.h>
#include <dtDAL/actorpluginregistry.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class DT_PLUGIN_EXPORT LibActors : public dtDAL::ActorPluginRegistry
{
    public:
        // Constructs our registry. Creates the actor types easy access when needed.
        LibActors();

        // Registers actor types with the actor factory in the super class.
        // virtual void RegisterActorTypes();
        void RegisterActorTypes();

    private:
        dtCore::RefPtr<dtDAL::ActorType> nodeActorType;
        dtCore::RefPtr<dtDAL::ActorType> edgeActorType;
        DECLARE_LOGGER();
};

#endif // LIB_ACTORS_H
