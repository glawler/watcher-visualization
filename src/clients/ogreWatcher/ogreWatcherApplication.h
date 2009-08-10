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
 * @file watcherApplication.h 
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#ifndef OGREWATCHER_APPLICATION_H
#define OGREWATCHER_APPLICATION_H

#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <OgreCEGUIRenderer.h>
#include <boost/smart_ptr.hpp>
#include <logger.h>
#include "ogreApplication.h"

namespace ogreWatcher
{
    /**
     * The "main" ogrewatcher application class. Stores OGRE stuff and 
     * handles basic OGRE application functionality.
     */
    class OgreWatcherApplication : public OgreApplication
    {
        public:
            /** Let there be light! (And possibly cameras.) */
            OgreWatcherApplication();

            /** And this too shall pass */
            virtual ~OgreWatcherApplication();

            /** Implement pure virtual from baseclass @ref ogreApplication */
            virtual void createScene(void);

        protected:

            virtual void chooseSceneManager(void);

            /** Override to add our messageStream listener */
            virtual void createFrameListener(void);

            CEGUI::OgreCEGUIRenderer *mGUIRenderer;
            CEGUI::System *mGUISystem;    

        private:

            DECLARE_LOGGER();
    };
}

#endif // OGREWATCHER_APPLICATION_H
