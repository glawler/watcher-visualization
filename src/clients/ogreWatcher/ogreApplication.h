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
 * @file ogreApplication.h 
 * @author Torus Knot Software Ltd, modified by Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    ExampleApplication.h (renamed - GTL).
Description: Base class for all the OGRE examples
-----------------------------------------------------------------------------
*/

#ifndef __OgreApplication_H___KJSAHDKASJDHNA
#define __OgreApplication_H___KJSAHDKASJDHNA

#include <Ogre.h>
#include <OgreConfigFile.h>
#include <declareLogger.h>
#include "ogreFrameListener.h"

namespace ogreWatcher
{
    /** Base class which manages the standard startup of an Ogre application.
      Designed to be subclassed for specific examples if required.
      */
    class OgreApplication
    {
        public:
            /** Standard constructor */
            OgreApplication();

            /** Standard destructor */
            virtual ~OgreApplication();

            /** Start the OGRE engine rendering the scene, does not return until the app is exiting */
            virtual void go(void);

        protected:
            Ogre::Root *mRoot;
            Ogre::Camera* mCamera;
            Ogre::SceneManager* mSceneMgr;
            OgreFrameListener* mFrameListener;
            Ogre::RenderWindow* mWindow;
            Ogre::String mResourcePath;

            // These internal methods package up the stages in the startup process
            /** Sets up the application - returns false if the user chooses to abandon configuration. */
            virtual bool setup(void);

            /** Configures the application - returns false if the user chooses to abandon configuration. */
            virtual bool configure(void);

            virtual void chooseSceneManager(void);
            virtual void createCamera(void);
            virtual void createFrameListener(void);
            virtual void createScene(void) = 0;    // pure virtual - this has to be overridden
            virtual void destroyScene(void);    // Optional to override this
            virtual void createViewports(void);

            /// Method which will define the source of resources (other than current folder)
            virtual void setupResources(void);

            /// Optional override method where you can create resource listeners (e.g. for loading screens)
            virtual void createResourceListener(void);

            /// Optional override method where you can perform resource group loading
            /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
            virtual void loadResources(void);
         private:
            DECLARE_LOGGER();
    };

} // namespace ogreWatcher


#endif // 
