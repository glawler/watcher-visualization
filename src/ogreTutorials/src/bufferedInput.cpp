#include "ExampleApplication.h"

class TutorialFrameListener : 
    public ExampleFrameListener, 
    public OIS::MouseListener, 
    public OIS::KeyListener
{
    public:
        TutorialFrameListener(RenderWindow* win, Camera* cam, SceneManager *sceneMgr)
            : ExampleFrameListener(win, cam, true, true)
        {
            // Populate the camera and scene manager containers
            mCamNode = cam->getParentSceneNode();
            mSceneMgr = sceneMgr;

            // set the rotation and move speed
            mRotate = 0.13;
            mMove = 250;

            // continue rendering
            mContinue = true;

            mMouse->setEventCallback(this);
            mKeyboard->setEventCallback(this);

            mDirection = Vector3::ZERO;
        }

        bool frameStarted(const FrameEvent &evt)
        {
            if(mMouse)
                mMouse->capture();
            if(mKeyboard) 
                mKeyboard->capture();

            mCamNode->translate(mDirection * evt.timeSinceLastFrame, Node::TS_LOCAL);
            
            return mContinue;
        }

        // MouseListener
        bool mouseMoved(const OIS::MouseEvent &e) 
        {
            if (e.state.buttonDown(OIS::MB_Right))
            {
                mCamNode->yaw(Degree(-mRotate * e.state.X.rel), Node::TS_WORLD);
                mCamNode->pitch(Degree(-mRotate * e.state.Y.rel), Node::TS_LOCAL);
            }
            return true;
        }
        bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id) 
        {
            Light *light = mSceneMgr->getLight("Light1");
            switch (id)
            {
                case OIS::MB_Left: light->setVisible(! light->isVisible()); break;
                default:
                    break;
            }
            return true;
        }
        bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) { return true; }

        // KeyListener
        bool keyPressed(const OIS::KeyEvent &e)
        {
            switch (e.key)
            {
                case OIS::KC_ESCAPE: mContinue = false; break;
                case OIS::KC_1:
                case OIS::KC_2:
                    mCamera->getParentSceneNode()->detachObject(mCamera);
                    mCamNode=mSceneMgr->getSceneNode(e.key==OIS::KC_1?"CamNode1":"CamNode2");
                    mCamNode->attachObject(mCamera);
                    break;
                case OIS::KC_UP:
                case OIS::KC_W: mDirection.z = -mMove; break;
                case OIS::KC_DOWN:
                case OIS::KC_S: mDirection.z = mMove; break;
                case OIS::KC_LEFT:
                case OIS::KC_A: mDirection.x = -mMove; break;
                case OIS::KC_RIGHT:
                case OIS::KC_D: mDirection.x = mMove; break;
                case OIS::KC_PGDOWN:
                case OIS::KC_E: mDirection.y = -mMove; break;
                case OIS::KC_PGUP:
                case OIS::KC_Q: mDirection.y = mMove; break;

                default:
                    break;
            }
            return mContinue;
        }
        bool keyReleased(const OIS::KeyEvent &e) 
        {
            switch (e.key)
            {
                case OIS::KC_UP:
                case OIS::KC_W: mDirection.z = 0; break;
                case OIS::KC_DOWN:
                case OIS::KC_S: mDirection.z = 0; break;
                case OIS::KC_LEFT:
                case OIS::KC_A: mDirection.x = 0; break;
                case OIS::KC_RIGHT:
                case OIS::KC_D: mDirection.x = 0; break;
                case OIS::KC_PGDOWN:
                case OIS::KC_E: mDirection.y = 0; break;
                case OIS::KC_PGUP:
                case OIS::KC_Q: mDirection.y = 0; break;
                default:
                    break;
            } // switch
            return true;
        }

    protected:
        Real mRotate;          // The rotate constant
        Real mMove;            // The movement constant

        SceneManager *mSceneMgr;   // The current SceneManager
        SceneNode *mCamNode;   // The SceneNode the camera is currently attached to

        bool mContinue;        // Whether to continue rendering or not
        Vector3 mDirection;     // Value to move in the correct direction
};

class TutorialApplication : public ExampleApplication
{
    public:
        void createCamera(void)
        {
            // create camera, but leave at default position
            mCamera = mSceneMgr->createCamera("PlayerCam"); 
            mCamera->setNearClipDistance(5);
        }

        void createScene(void)
        {
            mSceneMgr->setAmbientLight(ColourValue(0.25, 0.25, 0.25));

            // add the ninja
            Entity *ent = mSceneMgr->createEntity("Ninja", "ninja.mesh");
            SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
            node->attachObject(ent);

            // create the light
            Light *light = mSceneMgr->createLight("Light1");
            light->setType(Light::LT_POINT);
            light->setPosition(Vector3(250, 150, 250));
            light->setDiffuseColour(ColourValue::White);
            light->setSpecularColour(ColourValue::White);

            // Create the scene node
            node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode1", Vector3(-400, 200, 400));

            // Make it look towards the ninja
            node->yaw(Degree(-45));

            // Create the pitch node
            node = node->createChildSceneNode("PitchNode1");
            node->attachObject(mCamera);

            // create the second camera node/pitch node
            node = mSceneMgr->getRootSceneNode()->createChildSceneNode("CamNode2", Vector3(0, 200, 400));
            node = node->createChildSceneNode("PitchNode2");
        }

        void createFrameListener(void)
        {
            // Create the FrameListener
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
