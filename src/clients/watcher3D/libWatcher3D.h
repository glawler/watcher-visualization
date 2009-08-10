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


#ifndef LIB_WATCHER_3D_H
#define LIB_WATCHER_3D_H

// Watcher includes
#include "logger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtDAL/plugin_export.h>
#include <dtGame/gameapplication.h>
#include <dtGame/gameentrypoint.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

class DT_PLUGIN_EXPORT LibWatcher3D : public dtGame::GameEntryPoint
{
    public:
        LibWatcher3D();
        virtual ~LibWatcher3D();
        virtual void Initialize(dtGame::GameApplication& app, int argc, char** argv);
        virtual void OnStartup(dtGame::GameApplication& app);
    private:
        dtCore::RefPtr<dtCore::MotionModel> motionModel;
};

#endif // LIB_WATCHER_3D_H
