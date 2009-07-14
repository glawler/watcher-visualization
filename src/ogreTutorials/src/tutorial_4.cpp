#include "ExampleApplication.h"

class TutorialFrameListener : public ExampleFrameListener
{
    public:
        TutorialFrameListener(RenderWindow* win, Camera* cam, SceneManager *sceneMgr) : 
            ExampleFrameListener(win, cam, false, false),
            mMouseDown(false),
            mToggle(0.0),
            mCamNode(cam->getParentSceneNode()),
            mSceneMgr(sceneMgr),
            mRotate(0.13),
            mMove(250.0)
        {
        }

        // Overriding the default processUnbufferedKeyInput so the key updates we define
        // later on work as intended.
        bool processUnbufferedKeyInput(const FrameEvent& evt)
        {
            return true;
        }

        // Overriding the default processUnbufferedMouseInput so the Mouse updates we define
        // later on work as intended. 
        bool processUnbufferedMouseInput(const FrameEvent& evt)
        {
            return true;
        }

        bool frameStarted(const FrameEvent &evt)
        {
            mMouse->capture();
            mKeyboard->capture();

            if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
                return false;   /// adios - shutdown. 

            bool currMouse=mMouse->getMouseState().buttonDown(OIS::MB_Left);
            if (currMouse && !mMouseDown)
            {
                Light *light=mSceneMgr->getLight("Light1");
                light->setVisible(!light->isVisible());
            }
            mMouseDown=currMouse;

            mToggle-=evt.timeSinceLastFrame;
            if ((mToggle < 0.0f ) && mKeyboard->isKeyDown(OIS::KC_1))
            {
                mToggle=0.5f;
                mCamera->getParentSceneNode()->detachObject(mCamera);
                mCamNode=mSceneMgr->getSceneNode("CamNode1");
                mCamNode->attachObject(mCamera);
            }
            else if ((mToggle < 0.0f) && mKeyboard->isKeyDown(OIS::KC_2))
            {
                mToggle = 0.5f;
                mCamera->getParentSceneNode()->detachObject(mCamera);
                mCamNode = mSceneMgr->getSceneNode("CamNode2");
                mCamNode->attachObject(mCamera);
            }

            Vector3 transVector=Vector3::ZERO;
            if (mKeyboard->isKeyDown(OIS::KC_UP) || mKeyboard->isKeyDown(OIS::KC_W))
                transVector.z -= mMove;
            if (mKeyboard->isKeyDown(OIS::KC_DOWN) || mKeyboard->isKeyDown(OIS::KC_S))
                transVector.z += mMove;
            if (mKeyboard->isKeyDown(OIS::KC_LEFT) || mKeyboard->isKeyDown(OIS::KC_A))
                transVector.x -= mMove;
            if (mKeyboard->isKeyDown(OIS::KC_RIGHT) || mKeyboard->isKeyDown(OIS::KC_D))
                transVector.x += mMove;
            if (mKeyboard->isKeyDown(OIS::KC_PGUP) || mKeyboard->isKeyDown(OIS::KC_Q))
                transVector.y += mMove;
            if (mKeyboard->isKeyDown(OIS::KC_PGDOWN) || mKeyboard->isKeyDown(OIS::KC_E))
                transVector.y -= mMove;

            mCamNode->translate(transVector * evt.timeSinceLastFrame, Node::TS_LOCAL);

            if (mMouse->getMouseState().buttonDown(OIS::MB_Right))
            {
                mCamNode->yaw(Degree(-mRotate * mMouse->getMouseState().X.rel), Node::TS_WORLD);
                mCamNode->pitch(Degree(-mRotate * mMouse->getMouseState().Y.rel), Node::TS_LOCAL);
            }

            return true;
        }

    protected:
        bool mMouseDown;       // Whether or not the left mouse button was down last frame
        Real mToggle;          // The time left until next toggle
        Real mRotate;          // The rotate constant
        Real mMove;            // The movement constant
        SceneManager *mSceneMgr;   // The current SceneManager
        SceneNode *mCamNode;   // The SceneNode the camera is currently attached to
};

class TutorialApplication : public ExampleApplication
{
    public:
        TutorialApplication()
        {
        }

        ~TutorialApplication() 
        {
        }

    protected:
        void createCamera(void)
        {
            // create camera, but leave at default position
            mCamera = mSceneMgr->createCamera("PlayerCam"); 
            mCamera->setNearClipDistance(5);
        }

        void createScene(void)
        {
            mSceneMgr->setAmbientLight(ColourValue(0.25, 0.25, 0.25));  // Low amb light so we can see, but also see the lights going on and off.

            Entity *ent = mSceneMgr->createEntity("Ninja", "ninja.mesh");
            SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
            node->attachObject(ent);

            Light *light = mSceneMgr->createLight("Light1");
            light->setType(Light::LT_POINT);
            light->setPosition(Vector3(250, 150, 250));
            light->setDiffuseColour(ColourValue::White);
            light->setSpecularColour(ColourValue::White);

            // Create the scene node
            node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode1", Vector3(-400, 200, 400));
            node->yaw(Degree(-45));
            node->attachObject(mCamera);

            // create the second camera node
            node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode2", Vector3(0, 200, 400));
        }

        void createFrameListener(void)
        {
            mFrameListener = new TutorialFrameListener(mWindow, mCamera, mSceneMgr);
            mRoot->addFrameListener(mFrameListener);

            // Show the frame stats overlay
            mFrameListener->showDebugOverlay(true);
        }
};

int main(int argc, char **argv)
{
    // Create application object
    TutorialApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}
