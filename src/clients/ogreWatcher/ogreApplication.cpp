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
 * @file ogreApplication.cpp 
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

#ifndef __ExampleApplication_H__
#define __ExampleApplication_H__

#include <Ogre.h>
#include <OgreConfigFile.h>
#include "ogreFrameListener.h"
#include "ogreApplication.h"

using namespace std;
using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

    std::string ogreWatcher::macBundlePath()
    {
        TRACE_ENTER();

        char path[1024];
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        assert(mainBundle);

        CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
        assert(mainBundleURL);

        CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
        assert(cfStringRef);

        CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);

        CFRelease(mainBundleURL);
        CFRelease(cfStringRef);

        TRACE_EXIT();
        return std::string(path);
    }
#endif

namespace ogreWatcher {

    INIT_LOGGER(OgreApplication, "OgreApplication");

    // This function will locate the path to our application on OS X,
    // unlike windows you can not rely on the curent working directory
    // for locating your configuration files and resources.
    OgreApplication::OgreApplication()
    {
        TRACE_ENTER();
        mFrameListener = 0;
        mRoot = 0;
        // Provide a nice cross platform solution for locating the configuration files
        // On windows files are searched for in the current working directory, on OS X however
        // you must provide the full path, the helper function macBundlePath does this for us.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        mResourcePath = macBundlePath() + "/Contents/Resources/";
#else
        mResourcePath = "";
#endif
        TRACE_EXIT();
    }

    /// Standard destructor
    // virtual 
    OgreApplication::~OgreApplication()
    {
        TRACE_ENTER();
        if (mFrameListener)
            delete mFrameListener;
        if (mRoot)
            delete mRoot;
        TRACE_EXIT();
    }

    /// Start the example
    // virtual 
    void OgreApplication::go(void)
    {
        TRACE_ENTER();
        if (!setup())
            return;

        mRoot->startRendering();

        // clean up
        destroyScene();

        TRACE_EXIT();
    }

    // virtual 
    bool OgreApplication::setup(void)
    {
        TRACE_ENTER();

        String pluginsPath;
        // only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
        pluginsPath = mResourcePath + "plugins.cfg";
#endif

        mRoot = new Root(pluginsPath, 
                mResourcePath + "ogre.cfg", mResourcePath + "Ogre.log");

        setupResources();

        bool carryOn = configure();
        if (!carryOn) {
            TRACE_EXIT();
            return false;
        }

        chooseSceneManager();
        createCamera();
        createViewports();

        // Set default mipmap level (NB some APIs ignore this)
        TextureManager::getSingleton().setDefaultNumMipmaps(5);

        // Create any resource listeners (for loading screens)
        createResourceListener();
        // Load resources
        loadResources();

        // Create the scene
        createScene();

        createFrameListener();

        TRACE_EXIT();
        return true;

    }
    /** Configures the application - returns false if the user chooses to abandon configuration. */
    // virtual 
    bool OgreApplication::configure(void)
    {
        TRACE_ENTER();
        // Show the configuration dialog and initialise the system
        // You can skip this and use root.restoreConfig() to load configuration
        // settings if you were sure there are valid ones saved in ogre.cfg
        if(mRoot->restoreConfig() || mRoot->showConfigDialog())
        {
            // If returned true, user clicked OK so initialise
            // Here we choose to let the system create a default rendering window by passing 'true'
            mWindow = mRoot->initialise(true);
            TRACE_EXIT();
            return true;
        }
        else
        {   
            TRACE_EXIT(); 
            return false;
        }
    }

    // virtual 
    void OgreApplication::chooseSceneManager(void)
    {
        TRACE_ENTER();
        // Create the SceneManager, in this case a generic one
        mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "OgreAppSMInstance");
        TRACE_EXIT(); 
    }
    // virtual 
    void OgreApplication::createCamera(void)
    {
        TRACE_ENTER();
        // Create the camera
        mCamera = mSceneMgr->createCamera("MainCam");

        // Position it at 500 in Z direction
        mCamera->setPosition(Vector3(0,0,500));
        // Look back along -Z
        mCamera->lookAt(Vector3(0,0,-300));
        mCamera->setNearClipDistance(5);

        TRACE_EXIT(); 
    }

    // virtual 
    void OgreApplication::createFrameListener(void)
    {
        TRACE_ENTER();
        mFrameListener=new OgreFrameListener(mWindow, mCamera, mSceneMgr);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);

        TRACE_EXIT(); 
    }

    // virtual 
    void OgreApplication::destroyScene(void)
    {
        TRACE_ENTER();
        TRACE_EXIT(); 
    }

    // virtual 
    void OgreApplication::createViewports(void)
    {
        TRACE_ENTER();
        // Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio( Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
        TRACE_EXIT();
    }

    /// Method which will define the source of resources (other than current folder)
    // virtual 
    void OgreApplication::setupResources(void)
    {
        TRACE_ENTER();
        // Load resource paths from config file
        ConfigFile cf;
        cf.load(mResourcePath + "resources.cfg");

        // Go through all sections & settings in the file
        ConfigFile::SectionIterator seci = cf.getSectionIterator();

        String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            ConfigFile::SettingsMultiMap *settings = seci.getNext();
            ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                // OS X does not set the working directory relative to the app,
                // In order to make things portable on OS X we need to provide
                // the loading with it's own bundle path location
                ResourceGroupManager::getSingleton().addResourceLocation(String(macBundlePath() + "/" + archName), typeName, secName);
#else
                ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
#endif
            }
        }
        TRACE_EXIT();
    }

    /// Optional override method where you can create resource listeners (e.g. for loading screens)
    // virtual 
    void OgreApplication::createResourceListener(void)
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }

    /// Optional override method where you can perform resource group loading
    /// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    // virtual 
    void OgreApplication::loadResources(void)
    {
        TRACE_ENTER();
        // Initialise, parse scripts etc
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        TRACE_EXIT();
    }

};  // end of namespace


#endif
