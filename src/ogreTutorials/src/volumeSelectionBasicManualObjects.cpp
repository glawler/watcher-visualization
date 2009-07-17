#include <CEGUI/CEGUI.h>
#include <OgreCEGUIRenderer.h>

#include "ExampleApplication.h"

class SelectionRectangle : public ManualObject
{
    public:
        SelectionRectangle(const String &name) : ManualObject(name)
        {
            setRenderQueueGroup(RENDER_QUEUE_OVERLAY);
            setUseIdentityProjection(true);
            setUseIdentityView(true);
            setQueryFlags(0);
        }

        /**
         * Sets the corners of the SelectionRectangle.  Every parameter should be in the
         * range [0, 1] representing a percentage of the screen the SelectionRectangle
         * should take up.
         */
        void setCorners(float left, float top, float right, float bottom)
        {
            left = left * 2 - 1;
            right = right * 2 - 1;
            top = 1 - top * 2;
            bottom = 1 - bottom * 2;

            clear();
            begin("", RenderOperation::OT_LINE_STRIP);
            {
                position(left, top, -1);
                position(right, top, -1);
                position(right, bottom, -1);
                position(left, bottom, -1);
                position(left, top, -1);
            }
            end();

            // The last thing we need to do is set the bounding box for this object. Many SceneManagers cull objects which are 
            // off screen. Even though we've basically created a 2D object, Ogre is still a 3D engine, and treats our 2D object 
            // as if it sits in 3D space. This means that if we create this object and attach it to a SceneNode (as we will do 
            // in the next section), it will disappear on us when we look away. To fix this we will set the bounding box of the 
            // object to be infinite, so that the camera will always be inside of it: 
            AxisAlignedBox box;
            box.setInfinite();
            setBoundingBox(box); 
            // GTL was: setBoundingBox(AxisAlignedBox::BOX_INFINITE);

            // Be sure to note that we have added this code after the clear call. Every time you call ManualObject::clear, the 
            // bounding box is reset, so be careful if you create another ManualObject which is cleared often because the bounding 
            // box will have to be set every time you recreate it.
        }

        void setCorners(const Vector2 &topLeft, const Vector2 &bottomRight)
        {
            setCorners(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
        }
};

class DemoListener : public ExampleFrameListener, public OIS::MouseListener
{
    public:
        DemoListener(RenderWindow* win, Camera* cam, SceneManager *sceneManager)
            : ExampleFrameListener(win, cam, false, true), mSceneMgr(sceneManager), mSelecting(false)
        {
            mMouse->setEventCallback(this);

            mRect = new SelectionRectangle("Selection SelectionRectangle");
            mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(mRect);

            mVolQuery = mSceneMgr->createPlaneBoundedVolumeQuery(PlaneBoundedVolumeList());

        } // DemoListener

        ~DemoListener()
        {
            mSceneMgr->destroyQuery(mVolQuery);
            delete mRect;
        }

        /* MouseListener callbacks. */
        bool mouseMoved(const OIS::MouseEvent &arg)
        {
            CEGUI::System::getSingleton().injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
            if (mSelecting)
            {
                CEGUI::MouseCursor *mouse = CEGUI::MouseCursor::getSingletonPtr();
                mStop.x = mouse->getPosition().d_x / (float)arg.state.width;
                mStop.y = mouse->getPosition().d_y / (float)arg.state.height;

                mRect->setCorners(mStart, mStop);
            }
            return true;
        }

        bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
        {
            if (id == OIS::MB_Left)
            {
                CEGUI::MouseCursor *mouse = CEGUI::MouseCursor::getSingletonPtr();
                mStart.x = mouse->getPosition().d_x / (float)arg.state.width;
                mStart.y = mouse->getPosition().d_y / (float)arg.state.height;
                mStop = mStart;

                mSelecting = true;
                mRect->clear();
                mRect->setVisible(true);
            }
            return true;
        }

        bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
        {
            if (id == OIS::MB_Left)
            {
                performSelection(mStart, mStop);
                mSelecting = false;
                mRect->setVisible(false);
            }
            return true;
        }

        void performSelection(const Vector2 &first, const Vector2 &second)
        {
            float left = first.x, right = second.x, top = first.y, bottom = second.y;

            if (left > right)
                swap(left, right);

            if (top > bottom)
                swap(top, bottom);

            // in a real application you should probably find the center of the rectangle 
            // and perform a standard RaySceneQuery instead of doing nothing:
            if ((right - left) * (bottom - top) < 0.0001)
                return;

            Ray topLeft = mCamera->getCameraToViewportRay(left, top);
            Ray topRight = mCamera->getCameraToViewportRay(right, top);
            Ray bottomLeft = mCamera->getCameraToViewportRay(left, bottom);
            Ray bottomRight = mCamera->getCameraToViewportRay(right, bottom);

            PlaneBoundedVolume vol;
            vol.planes.push_back(Plane(topLeft.getPoint(3), topRight.getPoint(3), bottomRight.getPoint(3)));         // front plane
            vol.planes.push_back(Plane(topLeft.getOrigin(), topLeft.getPoint(100), topRight.getPoint(100)));         // top plane
            vol.planes.push_back(Plane(topLeft.getOrigin(), bottomLeft.getPoint(100), topLeft.getPoint(100)));       // left plane
            vol.planes.push_back(Plane(bottomLeft.getOrigin(), bottomRight.getPoint(100), bottomLeft.getPoint(100)));   // bottom plane
            vol.planes.push_back(Plane(topRight.getOrigin(), topRight.getPoint(100), bottomRight.getPoint(100)));     // right plane

            PlaneBoundedVolumeList volList;
            volList.push_back(vol);

            mVolQuery->setVolumes(volList);
            SceneQueryResult result = mVolQuery->execute();

            deselectObjects();
            SceneQueryResultMovableList::iterator itr;
            for (itr = result.movables.begin(); itr != result.movables.end(); ++itr)
                selectObject(*itr);
        }

        void deselectObjects()
        {
            std::list<MovableObject*>::iterator itr;
            for (itr = mSelected.begin(); itr != mSelected.end(); ++itr)
                (*itr)->getParentSceneNode()->showBoundingBox(false);
        }

        void selectObject(MovableObject *obj)
        {
            obj->getParentSceneNode()->showBoundingBox(true);
            mSelected.push_back(obj);
        }

    private:
        Vector2 mStart, mStop;
        SceneManager *mSceneMgr;
        PlaneBoundedVolumeListSceneQuery *mVolQuery;
        std::list<MovableObject*> mSelected;
        SelectionRectangle *mRect;
        bool mSelecting;


        static void swap(float &x, float &y)
        {
            float tmp = x;
            x = y;
            y = tmp;
        }
};

class DemoApplication : public ExampleApplication
{
    public:
        DemoApplication()
            : mRenderer(0), mSystem(0)
        {
        }

        ~DemoApplication() 
        {
            if (mSystem)
                delete mSystem;

            if (mRenderer)
                delete mRenderer;
        }

    protected:
        CEGUI::OgreCEGUIRenderer *mRenderer;
        CEGUI::System *mSystem;

        void createScene(void)
        {
            mRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);
            mSystem = new CEGUI::System(mRenderer);

            CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
            CEGUI::MouseCursor::getSingleton().setImage((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");

            mCamera->setPosition(-60, 100, -60);
            mCamera->lookAt(60, 0, 60);

            mSceneMgr->setAmbientLight(ColourValue::White);
            for (int i = 0; i < 10; ++i)
                for (int j = 0; j < 10; ++j)
                {
                    Entity *ent = mSceneMgr->createEntity("Robot" + StringConverter::toString(i + j * 10), "robot.mesh");
                    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(i * 15, 0, j * 15));
                    node->attachObject(ent);
                    node->setScale(0.1, 0.1, 0.1);
                }
        }

        void createFrameListener(void)
        {
            mFrameListener = new DemoListener(mWindow, mCamera, mSceneMgr);
            mFrameListener->showDebugOverlay(true);
            mRoot->addFrameListener(mFrameListener);
        }
};

int main(int argc, char **argv)
{
    // Create application object
    DemoApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}

