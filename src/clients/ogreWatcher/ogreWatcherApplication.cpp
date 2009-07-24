/**
 * @file watcherApplication.cpp 
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#include <string>
#include "ogreWatcherApplication.h"
#include "watcherMessageFrameListener.h"
#include "singletonConfig.h"

using namespace std;
using namespace Ogre;
using namespace watcher;
using namespace libconfig;

namespace ogreWatcher {

    INIT_LOGGER(OgreWatcherApplication, "OgreApplication.OgreWatcherApplication"); 

    OgreWatcherApplication::OgreWatcherApplication()
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }

    // virtual 
    OgreWatcherApplication::~OgreWatcherApplication()
    {
        TRACE_ENTER();
        TRACE_EXIT();
    }

    // virtual 
    void OgreWatcherApplication::chooseSceneManager(void)
    {
        TRACE_ENTER();
        // Use the terrain scene manager.
        mSceneMgr = mRoot->createSceneManager(ST_EXTERIOR_CLOSE);
        TRACE_EXIT();
    }

    // virtual 
    void OgreWatcherApplication::createScene(void)
    {
        TRACE_ENTER();

        // Set ambient light
        mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

        // mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
        mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE);
        // mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
        // mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
        mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->setShadowFarDistance(1500.0);
        mSceneMgr->setShadowTextureSettings(1024, 2); 
        
        // World geometry
        mSceneMgr->setWorldGeometry("terrain.cfg");

        // Set camera look point
        mCamera->setPosition(1500, 300, 750);
        mCamera->lookAt(500, 0, 500);  // GTL todo: make dynamic, look at center of terrain
        
        // CEGUI setup
        // mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);
        // mGUISystem = new CEGUI::System(mGUIRenderer);
        // 
        // // Mouse
        // CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
        // CEGUI::MouseCursor::getSingleton().setImage("TaharezLook", "MouseArrow");

        Light *light=mSceneMgr->createLight("Light3");
        light->setType(Light::LT_DIRECTIONAL);
        light->setDiffuseColour(ColourValue(1.0, 1.0, 1.0));
        light->setSpecularColour(ColourValue(1.0, 1.0, 1.0));
        light->setDirection(Vector3(0, -1, 1));

        // light=mSceneMgr->createLight("Light1");
        // light->setType(Light::LT_POINT);
        // light->setPosition(Vector3(0, 150, 250));
        // light->setDiffuseColour(1.0, 0.0, 0.0);
        // light->setSpecularColour(1.0, 0.0, 0.0);

        // light = mSceneMgr->createLight("Light2");
        // light->setType(Light::LT_SPOTLIGHT);
        // light->setDiffuseColour(0, 0, 1.0);
        // light->setSpecularColour(0, 0, 1.0);
        // light->setDirection(-1, -1, 0);
        // light->setPosition(Vector3(300, 300, 0));
        // light->setSpotlightRange(Degree(35), Degree(35));

        TRACE_EXIT();
    }

    // virtual
    void OgreWatcherApplication::createFrameListener(void)
    {
        TRACE_ENTER();

        // Let the base class register it's keyboard/mouse listeners
        OgreApplication::createFrameListener();

        // Lookup watcherd location and connect to it.
        string serverName("localhost");
        string service("watcherd");

        SingletonConfig::lock(); 
        Config &config=SingletonConfig::instance(); 
        if (!config.lookupValue("server", serverName))
        {
            LOG_INFO("'server' not found in the configuration file, using default: " << serverName 
                    << " and adding this to the configuration file.");
            config.getRoot().add("server", Setting::TypeString) = serverName;
        }

        if (!config.lookupValue("service", service))
        {
            LOG_INFO("'service' not found in the configuration file, using default: " << service  
                    << " and adding this to the configuration file.");
            config.getRoot().add("service", Setting::TypeString)=service;
        }
        SingletonConfig::unlock(); 

        // Create listener for watcher message updates and add it to the app.
        mRoot->addFrameListener(new WatcherMessageFrameListener(mSceneMgr, mCamera, serverName, service));
    }


} // end of namespace ogreWatcher
