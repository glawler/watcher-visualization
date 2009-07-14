#include "ExampleApplication.h"
#define CAMERA_NAME "SceneCamera"

void setupViewport(RenderWindow *win, SceneManager *curr)
{
    win->removeAllViewports();

    Camera *cam = curr->getCamera(CAMERA_NAME);
    Viewport *vp = win->addViewport(cam);

    vp->setBackgroundColour(ColourValue(0,0,0));
    cam->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
}

void dualViewport(RenderWindow *win, SceneManager *primary, SceneManager *secondary)
{
    win->removeAllViewports();

    Viewport *vp = 0;
    Camera *cam = primary->getCamera(CAMERA_NAME);
    vp = win->addViewport(cam, 0, 0, 0, 0.5, 1);
    vp->setBackgroundColour(ColourValue(0,0,0));
    cam->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));

    cam = secondary->getCamera(CAMERA_NAME);
    vp = win->addViewport(cam, 1, 0.5, 0, 0.5, 1);
    vp->setBackgroundColour(ColourValue(0,0,0));
    cam->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));

}

class SMTutorialListener : public ExampleFrameListener, public OIS::KeyListener
{
public:
    SMTutorialListener(RenderWindow* win, SceneManager *primary, SceneManager *secondary)
        : ExampleFrameListener(win, primary->getCamera(CAMERA_NAME), true, false),
          mPrimary(primary), mSecondary(secondary), mDual(false), mContinue(true)
    {
        mKeyboard->setEventCallback(this);
    }

    bool frameStarted(const FrameEvent& evt)
    {
        mKeyboard->capture();
        return mContinue;
    }

    bool keyPressed(const OIS::KeyEvent &arg)
    {
       switch (arg.key)
       {
           case OIS::KC_ESCAPE: mContinue = false; break;
           case OIS::KC_V: mDual = !mDual;
                           if (mDual) dualViewport(mWindow, mPrimary, mSecondary);
                           else setupViewport(mWindow, mPrimary);
                           break;
           case OIS::KC_C:
                           swap(mPrimary, mSecondary);
                           if (mDual) dualViewport(mWindow, mPrimary, mSecondary);
                           else setupViewport(mWindow, mPrimary);
                           break;
           default:
                           break;
       }

       return true;
    }

    bool keyReleased(const OIS::KeyEvent &) {return true;}

private:
    SceneManager *mPrimary, *mSecondary;
    bool mDual, mContinue;

    static void swap(SceneManager *&first, SceneManager *&second)
    {
        SceneManager *tmp = first;
        first = second;
        second = tmp;
    }
};

class SMTutorialApplication : public ExampleApplication
{
public:
    SMTutorialApplication()
    {
    }

    ~SMTutorialApplication() 
    {
    }
protected:
    SceneManager *mPrimary, *mSecondary;

    void chooseSceneManager(void)
    {
        mPrimary = mRoot->createSceneManager(ST_GENERIC, "primary");
        mSecondary = mRoot->createSceneManager(ST_GENERIC, "secondary");
    }

    void createCamera()
    {
        mPrimary->createCamera(CAMERA_NAME);
        mSecondary->createCamera(CAMERA_NAME);
    }

    void createViewports()
    {
        setupViewport(mWindow, mPrimary);
    }

    void createScene(void)
    {
        // Setup the TerrainSceneManager
        mPrimary->setSkyBox(true, "Examples/SpaceSkyBox");

        // Setup the Generic SceneManager
        mSecondary->setSkyDome(true, "Examples/CloudySky", 5, 8);
    }

    void createFrameListener(void)
    {
        mFrameListener = new SMTutorialListener(mWindow, mPrimary, mSecondary);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }
};

int main(int argc, char **argv)
{
    // Create application object
    SMTutorialApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}
