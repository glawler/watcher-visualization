#include "ExampleApplication.h"


// A FrameListener that gets passed our projector node and decal frustum so they can be animated
class ProjectiveDecalListener : public ExampleFrameListener
{
    public:
        ProjectiveDecalListener(RenderWindow* win, Camera* cam, SceneNode *proj, Frustum *decal)
            : ExampleFrameListener(win, cam), mProjectorNode(proj), mDecalFrustum(decal), mAnim(0)
        {
        }

        bool frameStarted(const FrameEvent& evt)
        {
            mProjectorNode->rotate(Vector3::UNIT_Y, Degree(evt.timeSinceLastFrame * 10));
            return ExampleFrameListener::frameStarted(evt);
        }

    protected:
        SceneNode *mProjectorNode;
        Frustum *mDecalFrustum;
        float mAnim;
};

class ProjectiveDecalApplication : public ExampleApplication
{
    protected:
        SceneNode *mProjectorNode;
        Frustum *mDecalFrustum;
        Frustum *mFilterFrustum;

        void createScene()
        {
            // Set ambient light
            mSceneMgr->setAmbientLight(ColourValue(0.2, 0.2, 0.2));

            // Create a light
            Light* l = mSceneMgr->createLight("MainLight");
            l->setPosition(20,80,50);

            // Position the camera
            mCamera->setPosition(60, 200, 70);
            mCamera->lookAt(0,0,0);

            // Make 6 ogre heads (named head0, head1, etc.) arranged in a circle
            Entity *ent;
            for (int i = 0; i < 6; i++)
            {
                SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
                ent = mSceneMgr->createEntity("head" + StringConverter::toString(i), "ogrehead.mesh");
                headNode->attachObject(ent);
                Radian angle(i * Math::TWO_PI / 6);
                headNode->setPosition(75 * Math::Cos(angle), 0, 75 * Math::Sin(angle));
            }

            createProjector();
            for (unsigned int i = 0; i < ent->getNumSubEntities(); i++)
                makeMaterialReceiveDecal(ent->getSubEntity(i)->getMaterialName());



        }

        // The function to create our decal projector
        void createProjector()
        {
            mDecalFrustum = new Frustum();
            mProjectorNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("DecalProjectorNode");
            mProjectorNode->attachObject(mDecalFrustum);
            mProjectorNode->setPosition(0,5,0);

            // Remove the back projection
            mFilterFrustum = new Frustum();
            mFilterFrustum->setProjectionType(PT_ORTHOGRAPHIC);
            SceneNode *filterNode = mProjectorNode->createChildSceneNode("DecalFilterNode");
            filterNode->attachObject(mFilterFrustum);
            filterNode->setOrientation(Quaternion(Degree(90),Vector3::UNIT_Y));
        }

        // A function to take an existing material and make it receive the projected decal
        void makeMaterialReceiveDecal(const String &matName)
        {
            MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName(matName);
            Pass *pass = mat->getTechnique(0)->createPass();
            pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
            pass->setDepthBias(1);
            pass->setLightingEnabled(false);

            TextureUnitState *texState = pass->createTextureUnitState("decal.png");
            texState->setProjectiveTexturing(true, mDecalFrustum);
            texState->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            texState->setTextureFiltering(FO_POINT, FO_LINEAR, FO_NONE);

            texState = pass->createTextureUnitState("decal_filter.png");
            texState->setProjectiveTexturing(true, mFilterFrustum);
            texState->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            texState->setTextureFiltering(TFO_NONE);
        }

        // Create new frame listener
        void createFrameListener(void)
        {
            mFrameListener= new ProjectiveDecalListener(mWindow, mCamera, mProjectorNode, mDecalFrustum);
            mRoot->addFrameListener(mFrameListener);
        }
};

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char **argv)
{
    // Create application object
    ProjectiveDecalApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        std::cerr << "An exception has occurred: " << e.getFullDescription();
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
