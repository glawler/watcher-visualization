#include "QOgreWidget.h"
#include <QMouseEvent>

#include "QSdkCameraMan.h"

namespace QtOgre 
{
    ///
    /// Default constructor
    ///
    QOgreWidget::QOgreWidget(QWidget *parent) : QOgreWindowWidget(parent) {
        mSceneMgr = NULL;
        m_vp = NULL;
    }
    ///
    /// Destructor
    ///
    QOgreWidget::~QOgreWidget(void) {
    }
    ///
    ///
    /// \name Protected functions
    /// Protected helper functions
    ///
    //@{
    ///
    /// Handle a resize event (pass it along to the render window)
    /// \param e The event data
    ///
    void QOgreWidget::resizeEvent(QResizeEvent *e) {
        QOgreWindowWidget::resizeEvent(e);

        if (m_renderWindow) {
            // Alter the camera aspect ratio to match the viewport
            mCamera->setAspectRatio(Ogre::Real(width()) / Ogre::Real(height()));
            m_vp->update();
        }
    }
    ///
    /// Handle a paint event (just render again, if needed create render window)
    /// \param e The event data
    ///
    void QOgreWidget::paintEvent(QPaintEvent *) {
        if(!m_renderWindow) {
            createRenderWindow();
            setupResources();
            setupScene();
        }

        update();
    }
    ///
    /// Handle a timer event
    /// \param e The event data
    ///
    void QOgreWidget::timerEvent(QTimerEvent *) {
        update();
    }
    ///
    /// The user pressed a mouse button, start tracking
    /// \param e The event data
    ///
    void QOgreWidget::mousePressEvent(QMouseEvent *e) {
        // std::cout << "got mouse press event " << e->x() << ", " << e->y() << std::endl;
        mCameraMgr->injectMouseDown(*e); 
        update();
    }
    ///
    /// The user released a mouse button, stop tracking
    /// \param e The event data
    ///
    void QOgreWidget::mouseReleaseEvent(QMouseEvent *e) {
        mCameraMgr->injectMouseUp(*e); 
        update();
    }
    ///
    /// The user moved the mouse, if tracking process it
    /// \param e The event data
    ///
    void QOgreWidget::mouseMoveEvent(QMouseEvent *e) {
        mCameraMgr->injectMouseMove(*e); 
        update();
    }
    ///
    /// Create the Ogre scene
    ///
    void QOgreWidget::createScene(void) {
        mSceneMgr->setAmbientLight(Ogre::ColourValue(0.6, 0.6, 0.6));

        // Setup the actual scene
        Ogre::Light* l = mSceneMgr->createLight("MainLight");
        l->setPosition(0, 100, 500);
        
        Ogre::Entity* mesh = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        m_mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        m_mainNode->attachObject(mesh);
        
        // mCamera->setAutoTracking(true, m_mainNode);
    }

    ///
    /// Configure the resources in Ogre
    /// \todo This should be moved somewhere else
    ///
    void QOgreWidget::setupResources(void) {
        // Load resource paths from config file
        Ogre::ConfigFile config;
        config.load("../data/resources.cfg");

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator it = config.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while (it.hasMoreElements()) {
            secName = it.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap *settings = it.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;

            for (i = settings->begin(); i != settings->end(); ++i) {
                typeName = i->first;
                archName = i->second;
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
            }
        }
    }
    //@}
    ///
    /// \name Private functions
    /// Private helper functions
    ///
    //@{
    ///
    /// Setup the scene
    ///
    void QOgreWidget::setupScene(void) {
        mSceneMgr = m_ogreRoot->createSceneManager(Ogre::ST_GENERIC);

        // Create the camera
        mCamera = mSceneMgr->createCamera("PlayerCam");
        mCamera->setPosition(Ogre::Vector3(0, 0, 200));

        // Look back along -Z
        mCamera->lookAt(Ogre::Vector3(0, 0, -300));
        mCamera->setNearClipDistance(5);

        mCameraMgr=new OgreBites::QSdkCameraMan(mCamera); 
        m_ogreRoot->addFrameListener(mCameraMgr); 

        // Create one viewport, entire window
        m_vp = m_renderWindow->addViewport(mCamera);
        m_vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
        m_vp->setClearEveryFrame(true);

        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        createScene();

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_BILINEAR);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(1);

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(Ogre::Real(m_vp->getActualWidth()) / Ogre::Real(m_vp->getActualHeight()));

        startTimer(20);
    }
    void QOgreWidget::keyPressEvent(QKeyEvent *e) {
        // std::cout << "got key press event " << e->text().toStdString() << std::endl;
        if (e->key()==Qt::Key_Space) { 
            switch(mCameraMgr->getStyle()) { 
                case OgreBites::CS_FREELOOK: 
                    mCameraMgr->setTarget(m_mainNode); 
                    mCameraMgr->setStyle(OgreBites::CS_ORBIT); 
                    break;
                case OgreBites::CS_MANUAL: 
                case OgreBites::CS_ORBIT: 
                    mCameraMgr->setTarget(NULL); 
                    mCameraMgr->setStyle(OgreBites::CS_FREELOOK); 
                    break;
            }
        }
        mCameraMgr->injectKeyDown(*e); 
        update(); 
    }
    void QOgreWidget::keyReleaseEvent(QKeyEvent *e) {
        mCameraMgr->injectKeyUp(*e); 
        update(); 
    }

    //@}
    ///
    /// \name Private constants
    /// Private constants this class uses
    ///
    //@{
    //@}
    //
    //
} // namespace QtOgre
