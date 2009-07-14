#include "ExampleApplication.h"

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
        void chooseSceneManager(void)
        {
            mSceneMgr=mRoot->createSceneManager(ST_EXTERIOR_CLOSE);
            ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        }

        void createScene(void)
        {
            // Fog code
            // ColourValue fadeColour(0.9, 0.9, 0.9);
            // mWindow->getViewport(0)->setBackgroundColour(fadeColour);

            // mSceneMgr->setFog(FOG_LINEAR, fadeColour, 0.0, 50, 500); // must come before setWorldGeometry() when using terrain scene manager // linear fog starting at 50 and going to 500
            // // mSceneMgr->setFog(FOG_EXP, fadeColour, 0.005); 
            // // mSceneMgr->setFog(FOG_EXP2, fadeColour, 0.003); 

            // mSceneMgr->setWorldGeometry("terrain.cfg");

            // Terrain code
            // mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");
            // mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 10, true);     // skybox very close to camera
            // mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000, false);     // skybox normal distance, but drawn last
            // mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000, false);     // skybox normal distance, but drawn last and too close

            // mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

            // Plane plane;
            // plane.d = 1000;
            // plane.normal = Vector3::NEGATIVE_UNIT_Y;
            // // mSceneMgr->setSkyPlane(true, plane, "Examples/SpaceSkyPlane", 1500, 75);
            // // mSceneMgr->setSkyPlane(true, plane, "Examples/SpaceSkyPlane", 1500, 50, true, 1.5f, 150, 150);  // using curvature and x y segments.
            // mSceneMgr->setSkyPlane(true, plane, "Examples/CloudySky", 1500, 40, true, 1.5f, 150, 150);  // using curvature and x y segments - again but with sky/clouds texture.


            // Problem with sky box and fog code
            ColourValue fadeColour(0.9, 0.9, 0.9);
            mSceneMgr->setFog(FOG_LINEAR, fadeColour, 0.0, 50, 515);
            mWindow->getViewport(0)->setBackgroundColour(fadeColour);
            mSceneMgr->setWorldGeometry("terrain.cfg");
            mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8, 500);


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
