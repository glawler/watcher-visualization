#include "ExampleApplication.h"
#include <OgreSimpleSpline.h>

class TutorialApplication : public ExampleApplication
{
    protected:
    public:
        TutorialApplication()
        {
        }

        ~TutorialApplication() 
        {
        }
    protected:
        virtual void createCamera(void)
        {
            mCamera=mSceneMgr->createCamera("PlayerCam");
            mCamera->setPosition(Vector3(0,10,500));
            mCamera->lookAt(Vector3(0,0,0));
            mCamera->setNearClipDistance(5);
        }

        virtual void createViewports(void)
        {
            // Create one viewport, entire window
            Viewport* vp = mWindow->addViewport(mCamera);
            vp->setBackgroundColour(ColourValue(0,0,0));

            // Alter the camera aspect ratio to match the viewport
            mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));    
        }

        void createScene(void)
        {
            Entity *ent=NULL;
            Light *light;

            Vector3 n1pos(0,0,0), n2pos(500,0,500);

            mSceneMgr->setAmbientLight(ColourValue(0.0, 0.0, 0.0));
            mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);

            ent=mSceneMgr->createEntity("Ninja1", "ninja.mesh");
            ent->setCastShadows(true);
            SceneNode *node=mSceneMgr->getRootSceneNode()->createChildSceneNode("Ninja3Node");
            node->yaw(Math::ATan2(n1pos.x-n2pos.z, n1pos.y-n2pos.z)); 
            node->attachObject(ent);

            ent=mSceneMgr->createEntity("Ninja2", "ninja.mesh");
            ent->setCastShadows(true);
            node=mSceneMgr->getRootSceneNode()->createChildSceneNode("Ninja2Node"); 
            node->setPosition(n2pos);
            node->yaw(Math::ATan2(n2pos.x-n1pos.z, n2pos.x-n1pos.z)); 
            node->attachObject(ent); 

            // Create a glowing green line
            ManualObject* myManualObject =  mSceneMgr->createManualObject("manual1"); 
            SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node"); 

            MaterialPtr myManualObjectMaterial = MaterialManager::getSingleton().create("manual1Material","debugger"); 
            myManualObjectMaterial->setReceiveShadows(false); 
            myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true); 
            myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0); 
            myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1); 
            myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,1,0); 

            // Create spline between them and draw it.
            SimpleSpline spline;
            spline.addPoint(Vector3(0, 150, 0));
            spline.addPoint(Vector3(166, 150, 333));
            spline.addPoint(Vector3(333, 150, 166));
            spline.addPoint(Vector3(500, 150, 500));
            const int numPts=100;
            myManualObject->begin("manual1Material", Ogre::RenderOperation::OT_LINE_STRIP); 
            for (int i=0; i<numPts; i++)
                myManualObject->position(spline.interpolate(Real(i)/numPts));
            myManualObject->position(spline.getPoint(spline.getNumPoints()-1));
            myManualObject->end(); 

            myManualObjectNode->attachObject(myManualObject);

            Plane plane(Vector3::UNIT_Y, 0);
            MeshManager::getSingleton().createPlane("ground", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane, 1500,1500,20,20,true,1,5,5,Vector3::UNIT_Z);

            ent=mSceneMgr->createEntity("GroundEntity", "ground");
            ent->setMaterialName("Examples/Rockwall");
            ent->setCastShadows(false);
            mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ent);

            light=mSceneMgr->createLight("Light1");
            light->setType(Light::LT_POINT);
            light->setPosition(Vector3(0, 150, 250));
            light->setDiffuseColour(1.0, 0.0, 0.0);
            light->setSpecularColour(1.0, 0.0, 0.0);

            light=mSceneMgr->createLight("Light3");
            light->setType(Light::LT_DIRECTIONAL);
            light->setDiffuseColour(ColourValue(.25, .25, 0));
            light->setSpecularColour(ColourValue(.25, .25, 0));
            light->setDirection(Vector3(0, -1, 1));

            light = mSceneMgr->createLight("Light2");
            light->setType(Light::LT_SPOTLIGHT);
            light->setDiffuseColour(0, 0, 1.0);
            light->setSpecularColour(0, 0, 1.0);
            light->setDirection(-1, -1, 0);
            light->setPosition(Vector3(300, 300, 0));
            light->setSpotlightRange(Degree(35), Degree(35));
        }
};

int main(int argc, char **argv)
{
    // Create application object
    TutorialApplication app;

    try {
        app.go();
    } 
    catch( Exception& e ) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}
       
