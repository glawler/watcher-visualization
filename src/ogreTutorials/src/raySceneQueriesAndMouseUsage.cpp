#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <OgreCEGUIRenderer.h>

#include "ExampleApplication.h"

class MouseQueryListener : public ExampleFrameListener, public OIS::MouseListener
{
    public:

        MouseQueryListener(RenderWindow* win, Camera* cam, SceneManager *sceneManager, CEGUI::Renderer *renderer)
            : ExampleFrameListener(win, cam, false, true), mGUIRenderer(renderer)
        {
            // Setup default variables
            mLMouseDown = false;
            mRMouseDown = false;
            mSceneMgr = sceneManager;

            mRobotNode=NULL;
            mWalkSpeed=35.0f;
            mRobotMoving=false;

            // Reduce move speed
            mMoveSpeed = 50;
            mRotateSpeed /= 500;

            mMouse->setEventCallback(this);
            mRaySceneQuery=mSceneMgr->createRayQuery(Ray());

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

            { // GTL todo - only do this if the camera moves 
                // Setup the scene query
                Vector3 camPos = mCamera->getPosition();
                Ray cameraRay(Vector3(camPos.x, 5000.0f, camPos.z), Vector3::NEGATIVE_UNIT_Y);
                mRaySceneQuery->setRay(cameraRay);

                // Perform the scene query
                RaySceneQueryResult &result = mRaySceneQuery->execute();
                RaySceneQueryResult::iterator itr = result.begin();

                if (itr!=result.end() && itr->worldFragment)
                {
                    Real terrainHgt=itr->worldFragment->singleIntersection.y;
                    if ((terrainHgt+10.0)>camPos.y)
                        mCamera->setPosition(camPos.x, terrainHgt+10.0, camPos.z);
                }
            }
           
            if (mRobotNode)
            {
                if (mToLocation!=Vector3::ZERO && mDistance!=Vector3::ZERO) {
                    Real move=mWalkSpeed*evt.timeSinceLastFrame;
                    mDistance-=move;
                    if (mDistance<=0.0) {
                        mNode->setPosition(mToLocation);
                        mDirection=Vector3::ZERO;
                        mAnimationState=mEntity->getAnimationState("Die");
                        mAnimationState->setLoop(true);
                        mAnimationState->setEnabled(true);
                    }
                    else {
                        mAnimationState = ent->getAnimationState("Walk");
                        mAnimationState->setLoop(true);
                        mAnimationState->setEnabled(true);

                        // Make sure the robot remains on the ground
                        Vector3 roborPos=mRobotNode->getPosition();
                        Ray robotRay(Vector3(roborPos.x, 5000.0f, roborPos.z), Vector3::NEGATIVE_UNIT_Y);
                        mRaySceneQuery->getRay(RobotRay);
                        RaySceneQueryResult &result = mRaySceneQuery->execute();
                        RaySceneQueryResult::iterator itr = result.begin();

                        if (itr!=result.end() && itr->worldFragment)
                        {
                            Real terrainHgt=itr->worldFragment->singleIntersection.y;
                            if ((terrainHgt+10.0)>camPos.y)
                                mCamera->setPosition(robotPos.x, terrainHgt+1.0, robotPos.z);
                        }

                        Vector3 src=mRobotNode->getOrientation*Vector3::UNIT_X;
                        mRobotNode->rotate(src.getRotationTo(mToLocation));
                        mRobotNode->translate(mDirection*move);
                    }
                }
                else {
                    mAnimationState = ent->getAnimationState("Idle");
                    mAnimationState->setLoop(true);
                    mAnimationState->setEnabled(true);
                }
            }

            


            mAnimationState->addTime(evt.timeSinceLastFrame);

            return true;
        }

        /* MouseListener callbacks. */
        bool mouseMoved(const OIS::MouseEvent &arg)
        {
            // Tell CEGUI where the mouse is.
            CEGUI::System::getSingleton().injectMouseMove(arg.state.X.rel, arg.state.Y.rel);

            if (mRMouseDown) {
                mCamera->yaw(Degree(-arg.state.X.rel*mRotateSpeed));
                mCamera->pitch(Degree(-arg.state.Y.rel*mRotateSpeed));
            }

            return true;
        }

        bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
        {
            if (id==OIS::MB_Left)
            {
                CEGUI::Point mousePos=CEGUI::MouseCursor::getSingleton().getPosition();
                Ray mouseRay=mCamera->getCameraToViewportRay(mousePos.d_x/float(arg.state.width), mousePos.d_y/float(arg.state.height));
                mRaySceneQuery->setRay(mouseRay);
                RaySceneQueryResult &result=mRaySceneQuery->execute();
                RaySceneQueryResult::iterator itr=result.begin();
                if (itr!=result.end() && itr->worldFragment) {

                    if (!mRobotNode) {      // create robot on first click
                        Entity *ent=mSceneMgr->createEntity("RobotEntity", "robot.mesh");
                        mRobotNode=mSceneMgr->getRootSceneNode()->createChildSceneNode("RobotNode", itr->worldFragment->singleIntersection);
                        mRobotNode->attachObject(ent);
                        mRobotNode->setScale(.1, .1, .1);
                    }
                    else { 
                        mToLocation=itr->worldFragment->singleIntersection;
                        mDistance=mToLocation.normalise();
                    }
                }

                mLMouseDown=true;
            }
            else if (id==OIS::MB_Right) {
                CEGUI::MouseCursor::getSingleton().hide();
                mRMouseDown=true;
            }
            return true;
        }

        bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
        {
            if (id==OIS::MB_Left) {
                mLMouseDown=false;
            }
            else if (id==OIS::MB_Right) {
                CEGUI::MouseCursor::getSingleton().show();
                mRMouseDown=false;
            }
            return true;
        }


    protected:
        RaySceneQuery *mRaySceneQuery;     // The ray scene query pointer
        bool mLMouseDown, mRMouseDown;     // True if the mouse buttons are down
        SceneManager *mSceneMgr;           // A pointer to the scene manager
        SceneNode *mRobotNode;             // The one and only robot
        CEGUI::Renderer *mGUIRenderer;     // CEGUI renderer
        Vector3 mToLocation;               // Where the robot is going.
        Vector3 mDistance;                 // Distance until robot gets to where he's going.
        AnimationState *mAnimationState;   // The current animation state of the robot
};

class MouseQueryApplication : public ExampleApplication
{
    protected:
        CEGUI::OgreCEGUIRenderer *mGUIRenderer;
        CEGUI::System *mGUISystem;         // cegui system
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

