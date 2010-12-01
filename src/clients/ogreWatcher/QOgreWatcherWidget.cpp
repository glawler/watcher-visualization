#include <QTimerEvent>
#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include <libwatcher/message.h>
#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/edgeMessage.h>
#include "QOgreWatcherWidget.h"
#include "logger.h"

namespace watcher 
{
    static const std::string entityNodeIdPrefix("node_"); 
    static const std::string sceneNodeIdPrefix("scenenode_"); 
    static const std::string layerSceneNodeIdPrefix("layer_"); 

    static const std::string groundSceneNodeId("theGroundSceneNode"); 
    static const Ogre::Real groundWidth=2500; 
    static const Ogre::Real groundHeight=2500; 

    static const std::string skyMaterialName("Examples/CloudySky"); 

    using namespace event; 
    INIT_LOGGER(QOgreWatcherWidget, "QOgreWidget.QOgreWatcherWidget"); 

    QOgreWatcherWidget::QOgreWatcherWidget(QWidget *parent) : 
        QOgreWidget(parent),
        lastKnownMessageTime(SeekMessage::eof),
        currentPlaybackTimerId(0),
        m_physicalLayerSceneNode(NULL) 
    { 
        currentPlaybackTimerId=startTimer(1000);  // once a sec.
    }
    // virtual
    QOgreWatcherWidget::~QOgreWatcherWidget() {
    }
    void QOgreWatcherWidget::resetPosition() {
        mCamera->lookAt(m_mainNode->getPosition()); 
        mCamera->setPosition(Ogre::Vector3(0, 200, 200)); 
    }
    void QOgreWatcherWidget::timerEvent(QTimerEvent *te) {
        QOgreWidget::timerEvent(te);  // let base class handle this as well. 
        if (te->timerId()==currentPlaybackTimerId) { 
            static watcher::Timestamp prevTimeStamp=lastKnownMessageTime; 
            if (prevTimeStamp!=lastKnownMessageTime) 
                emit currentPlaybackTime(lastKnownMessageTime); 
            prevTimeStamp=lastKnownMessageTime;
        }
    }
    bool QOgreWatcherWidget::nodeLocationUpdate(double x, double y, double z, const std::string &nodeId) {
        LOG_DEBUG("Got location update for node " << nodeId << ": " << x << "," << y << "," << z); 
        std::string nodeSceneId(sceneNodeIdPrefix + nodeId); 
        if (mSceneMgr->hasSceneNode(nodeSceneId)) {     
            const Ogre::Real gpsScale=100000; 
            Ogre::Real ox=x*gpsScale, oy=z, oz=y*gpsScale;   // OGRE XZ plane is ground, GPS it's XY, so swap y anx z.
            oy+=m_physicalLayerSceneNode->getPosition().y;

            Ogre::SceneNode *sNode=mSceneMgr->getSceneNode(nodeSceneId); 
            sNode->setPosition(ox, oy, oz); 

            // first scene node seen's location becomes the center of the playing field.
            static bool firstNode=true;
            if (firstNode)  {
                firstNode=false;
                m_mainNode->setPosition(ox, oy, oz); 
                mCameraMgr->setTarget(sNode);  // GTL look at this guy for now. 
            }
        }
        return true;
    }
    void QOgreWatcherWidget::newNodeSeen(watcher::MessagePtr m) {
        LOG_DEBUG("Seen a node for the first time: " << m->fromNodeID); 
        std::string nodeId(m->fromNodeID.to_string()); 
        Ogre::Entity *ent = mSceneMgr->createEntity(entityNodeIdPrefix + nodeId, "ogrehead.mesh"); 
        ent->setCastShadows(true); 
        Ogre::SceneNode *sNode = m_physicalLayerSceneNode->createChildSceneNode(sceneNodeIdPrefix + nodeId); 
        sNode->setScale(Ogre::Vector3(0.4, 0.4, 0.4)); 
        sNode->attachObject(ent);
    }
    void QOgreWatcherWidget::newLayerSeen(watcher::event::MessagePtr m) {
        std::string layer;
        if (!hasLayer(m, layer)) { 
            LOG_ERROR("Got a non-layer'd message in " << __FUNCTION__); 
            return;
        }
        LOG_DEBUG("Seen a layer, " << layer << ", for the first time. Creating scene node for it."); 
        m_mainNode->createChildSceneNode(layerSceneNodeIdPrefix + layer); 
    }
    void QOgreWatcherWidget::handleFeederMessage(watcher::event::MessagePtr m) {
        LOG_DEBUG("Got a feeder message in " << __FUNCTION__); 
        lastKnownMessageTime=m->timestamp; 
    }
    void QOgreWatcherWidget::handleEdgeMessage(watcher::event::MessagePtr m) {
        LOG_DEBUG("Got a edge message in " << __FUNCTION__); 
        lastKnownMessageTime=m->timestamp; 
    }
    void QOgreWatcherWidget::messageStreamConnected(bool) {
    }
    void QOgreWatcherWidget::createScene() {
        mSceneMgr->setAmbientLight(Ogre::ColourValue(0.25, 0.25, 0.25)); 

        // mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
        mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);

        // GTL - get s segfault if this is removed, don't know why. 
        Ogre::Entity* mesh = mSceneMgr->createEntity("Head", "ogrehead.mesh");
        m_mainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        m_mainNode->attachObject(mesh);
        mesh->setVisible(false); 

        // create physical layer, a little bit above the main scene node
        m_physicalLayerSceneNode=m_mainNode->createChildSceneNode(
                layerSceneNodeIdPrefix + watcher::event::PHYSICAL_LAYER, 
                Ogre::Vector3(0.0, 5.0, 0.0)); 

        // All of this taken from basic tutorial 2.
        Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
        Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                plane, groundWidth, groundHeight, 20, 20, true, 1, 15, 15, Ogre::Vector3::UNIT_Z);
        Ogre::Entity *groundEntity = mSceneMgr->createEntity("groundEntity", "ground");
        Ogre::SceneNode *groundSceneNode=m_mainNode->createChildSceneNode(groundSceneNodeId); 
        groundSceneNode->attachObject(groundEntity);
        groundEntity->setMaterialName("Examples/Rockwall");
        groundEntity->setCastShadows(false);

        Ogre::Light* directionalLight = mSceneMgr->createLight("directionalLight");
        directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
        directionalLight->setDiffuseColour(Ogre::ColourValue(0.8, 0.8, 0.8)); 
        directionalLight->setSpecularColour(Ogre::ColourValue(0.8, 0.8, 0.8));
        directionalLight->setDirection(Ogre::Vector3(-1, -1, -1));
        m_mainNode->attachObject(directionalLight); 

        mSceneMgr->setSkyDome(true, skyMaterialName, 10, 8, 4000, false); 
    }
    void QOgreWatcherWidget::toggleGround(bool show) {
        mSceneMgr->getSceneNode(groundSceneNodeId)->setVisible(show); 
    }
    void QOgreWatcherWidget::toggleSky(bool show) {
        mSceneMgr->setSkyDome(show, skyMaterialName, 10, 8, 4000, false); 
    }
    void QOgreWatcherWidget::toggleFullscreen(bool fullscreen) { 
        LOG_DEBUG("Turning fullscreen mode: " << (fullscreen?"ON":"OFF")); 
        // GTL does not work - must be interaction between Qt and OGRE and X11...
        m_renderWindow->setFullscreen(fullscreen, m_renderWindow->getWidth(), m_renderWindow->getHeight()); 
    }
}
