#include "ExampleApplication.h"

class RTTListener : public ExampleFrameListener
{ 
    public:
        RTTListener(RenderWindow *win, Camera *cam, SceneNode *sn) 
            : ExampleFrameListener(win, cam), mPlaneNode(sn)
        {
        }

        bool frameStarted(const FrameEvent& evt)
        {
            mPlaneNode->yaw(Radian(evt.timeSinceLastFrame));

            return ExampleFrameListener::frameStarted(evt);
        }

    protected:
        SceneNode	   *mPlaneNode;
};

class RTTApplication : public ExampleApplication, public RenderTargetListener
{
    protected:
        MovablePlane   *mPlane; 
        Entity	   *mPlaneEnt;
        SceneNode	   *mPlaneNode;
        Rectangle2D     *mMiniScreen;

        void createScene()
        {
            // Set ambient light
            mSceneMgr->setAmbientLight(ColourValue(0.2f, 0.2f, 0.2f));

            // Create a light
            Light* l = mSceneMgr->createLight("MainLight");
            l->setPosition(20, 80, 50);

            // Position the camera
            mCamera->setPosition(60, 200, 70);
            mCamera->lookAt(0, 0, 0);

            // Create a material for the plane (just a simple texture, here grass.jpg)
            MaterialPtr mat = MaterialManager::getSingleton().create("PlaneMat", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            TextureUnitState* t = mat->getTechnique(0)->getPass(0)->createTextureUnitState("grass_1024.jpg");

            // Create a simple plane
            mPlane = new MovablePlane("Plane");
            mPlane->d = 0;
            mPlane->normal = Vector3::UNIT_Y;
            MeshManager::getSingleton().createPlane("PlaneMesh", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                    *mPlane, 120, 120, 1, 1, true, 1, 1, 1, Vector3::UNIT_Z);
            mPlaneEnt = mSceneMgr->createEntity("PlaneEntity", "PlaneMesh");
            mPlaneEnt->setMaterialName("PlaneMat");

            // Attach the plane to a scene node
            mPlaneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
            mPlaneNode->attachObject(mPlaneEnt);

            // Create the textures
            TexturePtr texture = Ogre::TextureManager::getSingleton().createManual("RttTex", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, mWindow->getWidth(), mWindow->getHeight(), 0, PF_R8G8B8, TU_RENDERTARGET); 
            RenderTexture *renderTexture = texture->getBuffer()->getRenderTarget();
            renderTexture->addViewport(mCamera);
            renderTexture->getViewport(0)->setClearEveryFrame(true);
            renderTexture->getViewport(0)->setBackgroundColour(ColourValue::Black);
            renderTexture->getViewport(0)->setOverlaysEnabled(false);
            renderTexture->addListener(this);

            // Create the mini screen rectangle and attach it to a scene node
            mMiniScreen = new Ogre::Rectangle2D(true);
            mMiniScreen->setCorners(0.5, -0.5, 1.0, -1.0);
            // mMiniScreen->setBoundingBox(AxisAlignedBox(-100000.0*Vector3::UNIT_SCALE, 100000.0*Vector3::UNIT_SCALE)); 
            AxisAlignedBox box;
            box.setInfinite();
            mMiniScreen->setBoundingBox(box);
            Ogre::SceneNode *miniScreenNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("MiniScreenNode");
            miniScreenNode->attachObject(mMiniScreen);

            // Create the material for the mini screen
            MaterialPtr material = MaterialManager::getSingleton().create("RttMat", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            Ogre::Technique *technique = material->createTechnique();
            technique->createPass();
            material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
            material->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex");

            // Apply the material to the mini screen
            mMiniScreen->setMaterial("RttMat");
        }

        // Create a new frame listener
        void createFrameListener()
        {
            mFrameListener = new RTTListener(mWindow, mCamera, mPlaneNode);
            mRoot->addFrameListener(mFrameListener);
        }
};

int main(int argc, char **argv)
{
    // Create application object
    RTTApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        std::cerr << "An exception has occurred: " << e.getFullDescription();
    }

    return 0;
}

