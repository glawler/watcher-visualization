#include "ExampleApplication.h"

// Declare a subclass of the ExampleFrameListener class
class MyListener : public ExampleFrameListener
{
public:
    MyListener(RenderWindow* win, Camera* cam) : ExampleFrameListener(win, cam)
    {
    }

    bool frameStarted(const FrameEvent& evt)
    {
        return ExampleFrameListener::frameStarted(evt);        
    }

    bool frameEnded(const FrameEvent& evt)
    {
        return ExampleFrameListener::frameEnded(evt);        
    }
};

// Declare a subclass of the ExampleApplication class
class SampleApp : public ExampleApplication 
{
public:
   SampleApp() 
   {
   }

protected:
   // Define what is in the scene
   void createScene(void)
   {
       mSceneMgr->setAmbientLight(ColourValue( 1, 1, 1 ) );
       Entity *ent1 = mSceneMgr->createEntity("Robot", "robot.mesh");
       SceneNode *node1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("RobotNode");
       node1->attachObject(ent1);
       node1->scale(0.5, 0.5, 0.5);

       Entity *ent2 = mSceneMgr->createEntity( "Robot2", "robot.mesh" );
       SceneNode *node2 = node1->createChildSceneNode("RobotNode2", Vector3( 50, 0, 0));
       node2->attachObject( ent2 );
       node2->scale(2, 2, 2);
   }
  
   // Create new frame listener
   void createFrameListener(void)
   {
       mFrameListener = new MyListener(mWindow, mCamera);
       mRoot->addFrameListener(mFrameListener);
   }
};

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char **argv) 
{
    // Instantiate our subclass
    SampleApp myApp;

    try {
        // ExampleApplication provides a go method, which starts the rendering.
        myApp.go();
    }
    catch (Ogre::Exception& e) {
        std::cerr << "Exception:\n";
        std::cerr << e.getFullDescription().c_str() << "\n";
        return 1;
    }
}

#ifdef __cplusplus
}
#endif
