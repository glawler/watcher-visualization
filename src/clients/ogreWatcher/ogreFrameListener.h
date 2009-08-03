/**
 * @file ogreFrameListener.h 
 * @author Torus Knot Software Ltd, modified by Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
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
Filename:    ExampleFrameListener.h (renamed - GTL)
Description: Defines an example frame listener which responds to frame events.
This frame listener just moves a specified camera around based on
keyboard and mouse movements.
Mouse:    Freelook
W or Up:  Forward
S or Down:Backward
A:        Step left
D:        Step right
PgUp:     Move upwards
PgDown:   Move downwards
F:        Toggle frame rate stats on/off
R:        Render mode
T:        Cycle texture filtering
Bilinear, Trilinear, Anisotropic(8)
P:        Toggle on/off display of camera position / orientation
-----------------------------------------------------------------------------
*/

#ifndef __OgreFrameListener_H__MIKE_MYERS_AT_THE_WINDOW_QUESTION_MARK
#define __OgreFrameListener_H__MIKE_MYERS_AT_THE_WINDOW_QUESTION_MARK

#include <Ogre.h>
#include <OgreStringConverter.h>
#include <OgreException.h>
#include <logger.h>

//Use this define to signify OIS will be used as a DLL
//(so that dll import/export macros are in effect)
#define OIS_DYNAMIC_LIB
#include <OIS/OIS.h>

namespace ogreWatcher
{
    class OgreFrameListener : public Ogre::FrameListener, public Ogre::WindowEventListener
    {
        protected:
            void updateStats(void);

        public:
            /** Constructor takes a RenderWindow because it uses that to determine input context */
            OgreFrameListener(Ogre::RenderWindow* win, Ogre::Camera* cam, Ogre::SceneManager *sceneManager, bool bufferedKeys = false, bool bufferedMouse = false, bool bufferedJoy = false );

            /** Adjust mouse clipping area */
            virtual void windowResized(Ogre::RenderWindow* rw);

            /** Unattach OIS before window shutdown (very important under Linux) */
            virtual void windowClosed(Ogre::RenderWindow* rw);

            virtual ~OgreFrameListener();
            virtual bool processUnbufferedKeyInput(const Ogre::FrameEvent& evt);
            bool processUnbufferedMouseInput(const Ogre::FrameEvent& evt);
            void moveCamera();
            void showDebugOverlay(bool show);
            bool frameStarted(const Ogre::FrameEvent& evt);
            bool frameEnded(const Ogre::FrameEvent& evt);

        protected:
            Ogre::Camera* mCamera;

            Ogre::Vector3 mTranslateVector;
            Ogre::RenderWindow* mWindow;
            bool mStatsOn;

            std::string mDebugText;

            unsigned int mNumScreenShots;
            float mMoveScale;
            Ogre::Degree mRotScale;

            // just to stop toggles flipping too fast
            Ogre::Real mTimeUntilNextToggle ;
            Ogre::Radian mRotX, mRotY;
            Ogre::TextureFilterOptions mFiltering;
            int mAniso;

            int mSceneDetailIndex ;
            Ogre::Real mMoveSpeed;
            Ogre::Degree mRotateSpeed;
            Ogre::Overlay* mDebugOverlay;

            //OIS Input devices
            OIS::InputManager* mInputManager;
            OIS::Mouse*    mMouse;
            OIS::Keyboard* mKeyboard;
            OIS::JoyStick* mJoy;

            Ogre::RaySceneQuery *mRaySceneQuery;
            Ogre::SceneManager *mSceneMgr;

        private:
            DECLARE_LOGGER();
    };
}

#endif