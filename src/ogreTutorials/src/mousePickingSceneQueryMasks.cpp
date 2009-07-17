#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <OgreCEGUIRenderer.h>

#include "ExampleApplication.h"

class MouseQueryListener : public ExampleFrameListener, public OIS::MouseListener
{
    public:

        enum QueryFlags
        {
            NINJA_MASK = 1<<0,
            ROBOT_MASK = 1<<1
        };


        MouseQueryListener(RenderWindow* win, Camera* cam, SceneManager *sceneManager, CEGUI::Renderer *renderer)
            : ExampleFrameListener(win, cam, false, true), mGUIRenderer(renderer)
        {
            // Setup default variables
            mCount = 0;
            mCurrentObject = NULL;
            mLMouseDown = false;
            mRMouseDown = false;
            mSceneMgr = sceneManager;

            // Reduce move speed
            mMoveSpeed = 50;
            mRotateSpeed /= 500;

            // Register this so that we get mouse events.
            mMouse->setEventCallback(this);

            // Create RaySceneQuery
            mRaySceneQuery = mSceneMgr->createRayQuery(Ray());

            // Set result text, and default state
            mRobotMode = true;
            mDebugText = "Robot Mode Enabled - Press Space to Toggle";

        } // MouseQueryListener

        ~MouseQueryListener()
        {
            mSceneMgr->destroyQuery(mRaySceneQuery);
        }

        bool frameStarted(const FrameEvent &evt)
        {
            // Process the base frame listener code.  Since we are going to be
            // manipulating the translate vector, we need this to happen first.
            if (!ExampleFrameListener::frameStarted(evt))
                return false;

            // Swap modes
            if(mKeyboard->isKeyDown(OIS::KC_SPACE) && mTimeUntilNextToggle <= 0)
            {
                mRobotMode = !mRobotMode;
                mTimeUntilNextToggle = 1;
                mDebugText = (mRobotMode ? String("Robot") : String("Ninja")) + " Mode Enabled - Press Space to Toggle";
            }

            // Setup the scene query
            Vector3 camPos = mCamera->getPosition();
            Ray cameraRay(Vector3(camPos.x, 5000.0f, camPos.z), Vector3::NEGATIVE_UNIT_Y);
            mRaySceneQuery->setRay(cameraRay);

            // Perform the scene query
            mRaySceneQuery->setSortByDistance(false);
            RaySceneQueryResult &result = mRaySceneQuery->execute();
            RaySceneQueryResult::iterator itr;

            // Get the results, set the camera height
            for (itr = result.begin(); itr != result.end(); itr++)
            {
                if (itr->worldFragment)
                {
                    Real terrainHeight = itr->worldFragment->singleIntersection.y;
                    if ((terrainHeight + 10.0f) > camPos.y)
                        mCamera->setPosition(camPos.x, terrainHeight + 10.0f, camPos.z);
                    break;
                } // if
            } // for

            return true;
        }

        /* MouseListener callbacks. */
        bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
        {
            // Left mouse button up
            if (id == OIS::MB_Left)
            {
                onLeftReleased(arg);
                mLMouseDown = false;
            } // if

            // Right mouse button up
            else if (id == OIS::MB_Right)
            {
                onRightReleased(arg);
                mRMouseDown = false;
            } // else if

            return true;
        }

        bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
        {
            // Left mouse button down
            if (id == OIS::MB_Left)
            {
                onLeftPressed(arg);
                mLMouseDown = true;
            } // if

            // Right mouse button down
            else if (id == OIS::MB_Right)
            {
                onRightPressed(arg);
                mRMouseDown = true;
            } // else if

            return true;
        }

        bool mouseMoved(const OIS::MouseEvent &arg)
        {
            // Update CEGUI with the mouse motion
            CEGUI::System::getSingleton().injectMouseMove(arg.state.X.rel, arg.state.Y.rel);

            // If we are dragging the left mouse button.
            if (mLMouseDown)
            {
                CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
                Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.d_x/float(arg.state.width),mousePos.d_y/float(arg.state.height));
                mRaySceneQuery->setRay(mouseRay);
                mRaySceneQuery->setSortByDistance(false);

                RaySceneQueryResult &result = mRaySceneQuery->execute();
                RaySceneQueryResult::iterator itr;

                for (itr = result.begin(); itr != result.end(); itr++)
                    if (itr->worldFragment)
                    {
                        mCurrentObject->setPosition(itr->worldFragment->singleIntersection);
                        break;
                    } // if
            } // if

            // If we are dragging the right mouse button.
            else if (mRMouseDown)
            {
                mCamera->yaw(Degree(-arg.state.X.rel * mRotateSpeed));
                mCamera->pitch(Degree(-arg.state.Y.rel * mRotateSpeed));
            } // else if

            return true;
        }

        // Specific handlers
        void onLeftPressed(const OIS::MouseEvent &arg)
        {
            // Turn off bounding box.
            if (mCurrentObject)
                mCurrentObject->showBoundingBox(false);

            // Setup the ray scene query
            CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
            Ray mouseRay = mCamera->getCameraToViewportRay(mousePos.d_x/float(arg.state.width), mousePos.d_y/float(arg.state.height));
            mRaySceneQuery->setRay(mouseRay);
            mRaySceneQuery->setSortByDistance(true);
            mRaySceneQuery->setQueryMask(mRobotMode ? ROBOT_MASK : NINJA_MASK); 

            // Execute query
            RaySceneQueryResult &result = mRaySceneQuery->execute();
            RaySceneQueryResult::iterator itr;

            // Get results, create a node/entity on the position
            for ( itr = result.begin(); itr != result.end(); itr++ )
            {
                if (itr->movable && itr->movable->getName().substr(0, 5) != "tile[")
                {
                    mCurrentObject = itr->movable->getParentSceneNode();
                    break;
                }
                else if (itr->worldFragment)
                {
                    Entity *ent;
                    char name[16];
                    if (mRobotMode)
                    {
                        sprintf(name, "Robot%d", mCount++);
                        ent = mSceneMgr->createEntity(name, "robot.mesh");
                        ent->setQueryFlags(ROBOT_MASK);
                    } // if
                    else
                    {
                        sprintf(name, "Ninja%d", mCount++);
                        ent = mSceneMgr->createEntity(name, "ninja.mesh");
                        ent->setQueryFlags(NINJA_MASK);
                    } // else

                    mCurrentObject = mSceneMgr->getRootSceneNode()->createChildSceneNode(String(name) + "Node", itr->worldFragment->singleIntersection);
                    mCurrentObject->attachObject(ent);
                    float scale = mRobotMode?0.1:0.05;
                    mCurrentObject->setScale(scale, scale, scale);
                    break;
                } // else if
            } // for


            // Show the bounding box to highlight the selected object
            if (mCurrentObject)
                mCurrentObject->showBoundingBox(true);
        }

        void onLeftReleased(const OIS::MouseEvent &arg)
        {
        }

        void onRightPressed(const OIS::MouseEvent &arg)
        {
            CEGUI::MouseCursor::getSingleton().hide();
        }

        virtual void onRightReleased(const OIS::MouseEvent &arg)
        {
            CEGUI::MouseCursor::getSingleton().show();
        }

    protected:
        RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
        bool mLMouseDown, mRMouseDown;     // True if the mouse buttons are down
        int mCount;                        // The number of robots on the screen
        SceneManager *mSceneMgr;           // A pointer to the scene manager
        SceneNode *mCurrentObject;         // The newly created object
        CEGUI::Renderer *mGUIRenderer;     // CEGUI renderer

        bool mRobotMode;                   // The current state
};

class MouseQueryApplication : public ExampleApplication
{
    protected:
        CEGUI::OgreCEGUIRenderer *mGUIRenderer;
        CEGUI::System *mGUISystem;         // CEGUI system
    public:
        MouseQueryApplication()
        {
        }

        ~MouseQueryApplication() 
        {
        }
    protected:
        void chooseSceneManager(void)
        {
            // Use the terrain scene manager.
            mSceneMgr = mRoot->createSceneManager(ST_EXTERIOR_CLOSE);
        }

        void createScene(void)
        {
            // Set ambient light
            mSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
            mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

            // World geometry
            mSceneMgr->setWorldGeometry("terrain.cfg");

            // Set camera look point
            mCamera->setPosition(40, 100, 580);
            mCamera->pitch(Degree(-30));
            mCamera->yaw(Degree(-45));

            // CEGUI setup
            mGUIRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);
            mGUISystem = new CEGUI::System(mGUIRenderer);

            // Mouse
            CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
            CEGUI::MouseCursor::getSingleton().setImage("TaharezLook", "MouseArrow");
        }

        void createFrameListener(void)
        {
            mFrameListener = new MouseQueryListener(mWindow, mCamera, mSceneMgr, mGUIRenderer);
            mFrameListener->showDebugOverlay(true);
            mRoot->addFrameListener(mFrameListener);
        }
};


int main(int argc, char **argv)
{
    // Create application object
    MouseQueryApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}


