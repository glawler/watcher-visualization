#include "ExampleApplication.h"
#include <CEGUI/CEGUI.h>
#include <OIS/OIS.h>
#include <OgreCEGUIRenderer.h>

// Convert between OIS and CEGUI mouse events
CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
        case OIS::MB_Left: return CEGUI::LeftButton;
        case OIS::MB_Right: return CEGUI::RightButton;
        case OIS::MB_Middle: return CEGUI::MiddleButton;
        default: return CEGUI::LeftButton;
    }
}

class TutorialListener : public ExampleFrameListener, public OIS::MouseListener, public OIS::KeyListener
{
public:
    TutorialListener(RenderWindow* win, Camera* cam) : ExampleFrameListener(win, cam, true, true)
    {
        mContinue=true;
        mMouse->setEventCallback(this);
        mKeyboard->setEventCallback(this);

        CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();
        CEGUI::Window *quit = wmgr->getWindow((CEGUI::utf8*)"CEGUIDemo/QuitButton");
        quit->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TutorialListener::quit, this));

    } // CEGUIDemoListener

    bool frameStarted(const FrameEvent &evt)
    {
        mKeyboard->capture();
        mMouse->capture();

        return mContinue && !mKeyboard->isKeyDown(OIS::KC_ESCAPE);
    }

    bool quit(const CEGUI::EventArgs &e)
    {
        mContinue = false;
        return true;
    }

    // MouseListener
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        CEGUI::System::getSingleton().injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    }

    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));
    }

    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));
    }

    // KeyListener
    bool keyPressed(const OIS::KeyEvent &arg)
    {
        CEGUI::System *sys = CEGUI::System::getSingletonPtr();
        sys->injectKeyDown(arg.key);
        sys->injectChar(arg.text);
    }

    bool keyReleased(const OIS::KeyEvent &arg)
    {
        CEGUI::System::getSingleton().injectKeyUp(arg.key);
    }

private:
    bool mContinue;
};


class CEGUIDemoApplication : public ExampleApplication
{
public:
    CEGUIDemoApplication()
        : mSystem(0), mRenderer(0)
    {
    }

    ~CEGUIDemoApplication() 
    {
        if (mSystem)
            delete mSystem;

        if (mRenderer)
            delete mRenderer;
    }
protected:
   CEGUI::System *mSystem;
   CEGUI::OgreCEGUIRenderer *mRenderer;

    void createScene(void)
    {
        mRenderer = new CEGUI::OgreCEGUIRenderer(mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000, mSceneMgr);
        mSystem = new CEGUI::System(mRenderer);
        CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"TaharezLookSkin.scheme");
        mSystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
        mSystem->setDefaultFont((CEGUI::utf8*)"BlueHighway-12");

        CEGUI::WindowManager *win = CEGUI::WindowManager::getSingletonPtr();
        CEGUI::Window *sheet = win->createWindow("DefaultGUISheet", "CEGUIDemo/Sheet");
        CEGUI::Window *quit = win->createWindow("TaharezLook/Button", "CEGUIDemo/QuitButton");
        quit->setText("Quit");
        quit->setSize(CEGUI::UVector2(CEGUI::UDim(0.15, 0), CEGUI::UDim(0.05, 0)));
        sheet->addChildWindow(quit);

        mSystem->setGUISheet(sheet);

        mSceneMgr->setAmbientLight(ColourValue(1, 1, 1));
        mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);
        Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0, 0, -300));
        headNode->attachObject(ogreHead);

        // Create render window in CEGUI
        RenderTexture *tex=mRoot->getTextureManager()->createManual(
                "R2TTex", "Default", TEX_TYPE_2D, 512, 512, 0, PF_R8G8B8, TU_RENDERTARGET
                )->getBuffer()->getRenderTarget();

        Camera *cam = mSceneMgr->createCamera("R2TCam");
        cam->setPosition(100, -100, -400);
        cam->lookAt(0, 0, -300);

        Viewport *v = tex->addViewport(cam);
        v->setOverlaysEnabled(false);
        v->setClearEveryFrame(true);
        v->setBackgroundColour(ColourValue::Black);

        CEGUI::Texture *cTex = mRenderer->createTexture((CEGUI::utf8*)"R2TTex");

        CEGUI::Imageset *imageSet = CEGUI::ImagesetManager::getSingleton().createImageset((CEGUI::utf8*)"R2TImageset", cTex);
        imageSet->defineImage((CEGUI::utf8*)"R2TImage", 
                CEGUI::Point(0.0f, 0.0f),
                CEGUI::Size(cTex->getWidth(), cTex->getHeight()),
                CEGUI::Point(0.0f,0.0f));

        CEGUI::Window *si = win->createWindow((CEGUI::utf8*)"TaharezLook/StaticImage", "R2TWindow");
        si->setSize(CEGUI::UVector2(CEGUI::UDim(0.5f, 0), CEGUI::UDim(0.4f, 0)));
        si->setPosition(CEGUI::UVector2(CEGUI::UDim(0.5f, 0), CEGUI::UDim(0, 0)));
        si->setProperty("Image", CEGUI::PropertyHelper::imageToString(&imageSet->getImage((CEGUI::utf8*)"R2TImage")));
        sheet->addChildWindow(si);
    }

    void createFrameListener(void)
    {
        mFrameListener= new TutorialListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }

};

int main(int argc, char **argv)
{
    // Create application object
    CEGUIDemoApplication app;

    try {
        app.go();
    } 
    catch(Exception& e) {
        fprintf(stderr, "An exception has occurred: %s\n", e.getFullDescription().c_str());
    }

    return 0;
}

