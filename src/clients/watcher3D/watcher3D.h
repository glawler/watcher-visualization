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


#ifndef WATCHER_3D_H
#define WATCHER_3D_H

// Watcher includes
#include "declareLogger.h"

// Delta3D includes
#include <disable_watcher_logging.h> /* undef watcher logging macros */
#include <dtABC/application.h>
#include <dtGame/gameapplication.h>
#include <dtCore/refptr.h>
#include <dtGame/gamemanager.h>
#include <dtGame/defaultgroundclamper.h>
#include <enable_watcher_logging.h> /* redef watcher logging macros */

// Forward declarations to keep compile-time down
namespace dtCore
{
   class Object;
   class FlyMotionModel;
}

class Watcher3D : public dtGame::GameApplication
{
    public:
        // Watcher3D(const std::string& configFilename);
        Watcher3D(int argc, char** argv);
    protected:
        virtual ~Watcher3D();

    public:
        //virtual void Config();

    private:
        dtCore::RefPtr<dtCore::FlyMotionModel> mFlyMotionModel;
        dtCore::RefPtr<dtGame::DefaultGroundClamper> mGroundClamper;
        DECLARE_LOGGER();
};

#endif // WATCHER_3D_H
