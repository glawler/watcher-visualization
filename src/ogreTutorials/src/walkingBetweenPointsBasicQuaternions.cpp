#include "ExampleApplication.h"

#include <deque>
using namespace std;

class MoveDemoListener : public ExampleFrameListener
{
public:

    MoveDemoListener(RenderWindow* win, Camera* cam, SceneNode *sn, Entity *ent, deque<Vector3> &walk) : 
        ExampleFrameListener(win, cam, false, false), mNode(sn), mEntity(ent), mWalkList(walk)
    {
        // Set idle animation
        mAnimationState = ent->getAnimationState("Idle");
        mAnimationState->setLoop(true);
        mAnimationState->setEnabled(true);

        mWalkSpeed=35.0f;
        mDirection=Vector3::ZERO;

    } // MoveDemoListener

    /* This function is called to start the object moving to the next position
    in mWalkList.
    */
    bool nextLocation()
    {
        if (mWalkList.empty())
            return false;

        mDestination=mWalkList.front();
        mWalkList.pop_front();

        mDirection=mDestination-mNode->getPosition();
        mDistance=mDirection.normalise();

        return true;
    } // nextLocation()

    bool frameStarted(const FrameEvent &evt)
    {
        if (mDirection==Vector3::ZERO) {
            if (nextLocation()) {
                mAnimationState=mEntity->getAnimationState("Walk");
                mAnimationState->setLoop(true);
                mAnimationState->setEnabled(true);
            }
        }
        else {
            Real move=mWalkSpeed*evt.timeSinceLastFrame;
            mDistance-=move;
            if (mDistance<=0.0) {
                mNode->setPosition(mDestination);
                mDirection=Vector3::ZERO;
                if (!nextLocation()) {
                    mAnimationState=mEntity->getAnimationState("Die");
                    mAnimationState->setLoop(true);
                    mAnimationState->setEnabled(true);
                }
                else {
                    Vector3 src = mNode->getOrientation() * Vector3::UNIT_X;
                    if ((1.0f + src.dotProduct(mDirection)) < 0.0001f) 
                        mNode->yaw(Degree(180));
                    else {
                        Ogre::Quaternion quat = src.getRotationTo(mDirection);
                        mNode->rotate(quat);
                    } 
                }
            }
            else {
                mNode->translate(mDirection*move);
            }
        }

        mAnimationState->addTime(evt.timeSinceLastFrame);
        return ExampleFrameListener::frameStarted(evt);
    }

protected:
    Real mDistance;                  // The distance the object has left to travel
    Vector3 mDirection;              // The direction the object is moving
    Vector3 mDestination;            // The destination the object is moving towards

    AnimationState *mAnimationState; // The current animation state of the object

    Entity *mEntity;                 // The Entity we are animating
    SceneNode *mNode;                // The SceneNode that the Entity is attached to
    std::deque<Vector3> mWalkList;   // The list of points we are walking to

    Real mWalkSpeed;                 // The speed at which the object is moving
};


class MoveDemoApplication : public ExampleApplication
{
protected:
public:
    MoveDemoApplication()
    {
    }

    ~MoveDemoApplication() 
    {
    }
protected:
    Entity *mEntity;                // The entity of the object we are animating
    SceneNode *mNode;               // The SceneNode of the object we are moving
    std::deque<Vector3> mWalkList;  // A deque containing the waypoints

    void createScene(void)
    {
        // Set the default lighting.
        mSceneMgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));

        // Create the entity
        mEntity = mSceneMgr->createEntity("Robot", "robot.mesh");

        // Create the scene node
        mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("RobotNode", Vector3(0.0f, 0.0f, 25.0f));
        mNode->attachObject(mEntity);

        // Create the walking list
        mWalkList.push_back(Vector3(550.0f,  0.0f,  50.0f));
        mWalkList.push_back(Vector3(-100.0f,  0.0f, -100.0f));
        mWalkList.push_back(Vector3(-100.0f,  0.0f, 0f));
        mWalkList.push_back(Vector3(100.0f,  0.0f, -75.0f));

        // Create objects so we can see movement
        Entity *ent;
        SceneNode *node;

        char entName[]="Knot0";
        char nodeName[]="Knot0Node";
        for (std::deque<Vector3>::iterator i=mWalkList.begin(); i!=mWalkList.end(); ++i)
        {
            nodeName[5]++;
            entName[5]++;
            ent=mSceneMgr->createEntity(entName, "knot.mesh");
            Vector3 v=*i;
            node=mSceneMgr->getRootSceneNode()->createChildSceneNode(nodeName, Vector3(v.x, -v.y,  v.z));
            node->attachObject(ent);
            node->setScale(0.1f, 0.1f, 0.1f);
        }

        // Set the camera to look at our handiwork
        mCamera->setPosition(90.0f, 280.0f, 535.0f);
        mCamera->pitch(Degree(-30.0f));
        mCamera->yaw(Degree(-15.0f));
    }

    void createFrameListener(void)
    {
        mFrameListener=new MoveDemoListener(mWindow, mCamera, mNode, mEntity, mWalkList);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }

};

int main(int argc, char **argv)
{
    // Create application object
    MoveDemoApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}

