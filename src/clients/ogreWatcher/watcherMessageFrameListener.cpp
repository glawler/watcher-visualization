#include <message.h>  // For watcher lib message
#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include "watcherMessageFrameListener.h"
#include "seekWatcherMessage.h"

using namespace std;
using namespace Ogre;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

namespace ogreWatcher {

    INIT_LOGGER(WatcherMessageFrameListener, "WatcherMessageFrameListener"); 

    WatcherMessageFrameListener::WatcherMessageFrameListener(SceneManager *sm, Camera *cam, const std::string &host, const std::string &service) :
        gpsScale(10.0), newMessage(new Message), mSceneMgr(sm), mCamera(cam), sceneNodeTag("SceneNode")
    {
        TRACE_ENTER();

        // messageStream=MessageStream::createNewMessageStream(host, service);
        messageStream=MessageStream::createNewMessageStream(host); 
        if (messageStream)
            LOG_DEBUG("Connected to " << service << " on host " << host);
        else {
            LOG_FATAL("Unable to connect to watcher daemon service " << service << " at " << host);
            exit(EXIT_FAILURE); // Low tolerance for not being connected. Must fix this.
        }

        messageStream->setStreamTimeStart(SeekMessage::eof);
        messageStream->startStream();
        messageStream->getMessageTimeRange();

        mRaySceneQuery=mSceneMgr->createRayQuery(Ray());

        TRACE_EXIT();
    }

    WatcherMessageFrameListener::~WatcherMessageFrameListener()
    {
        TRACE_ENTER();
        if (mRaySceneQuery && mSceneMgr)
            mSceneMgr->destroyQuery(mRaySceneQuery);
        TRACE_EXIT();
    }

    // virtual
    bool WatcherMessageFrameListener::frameStarted(const FrameEvent& evt)
    {
        TRACE_ENTER();

        bool retVal=true;
        if (messageStream) 
        {
            // LOG_DEBUG("Checking for new message from the daemon"); 
            while (messageStream->isStreamReadable())
            {
                LOG_DEBUG("Reading message from watcherd");

                if (!messageStream->getNextMessage(newMessage))
                    LOG_ERROR("Error reading message from watcher daemon message stream");
                else
                {
                    LOG_DEBUG("Processing " << newMessage->type << " message from watcherd");
                    switch (newMessage->type)
                    {
                        case COLOR_MESSAGE_TYPE:
                            break;
                        case CONNECTIVITY_MESSAGE_TYPE:
                            break;
                        case GPS_MESSAGE_TYPE:
                            updateNodeLocation(dynamic_pointer_cast<GPSMessage>(newMessage));
                            break;
                        case EDGE_MESSAGE_TYPE:
                            handleEdgeMessage(dynamic_pointer_cast<EdgeMessage>(newMessage));
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        TRACE_EXIT_RET_BOOL(retVal);
        return retVal;
    }

    bool WatcherMessageFrameListener::getEntityAndSceneNode(const string &entId, const string &sceneNodeId, Entity *&ent, SceneNode *&sNode)
    {
        TRACE_ENTER();
        if (!mSceneMgr->hasEntity(entId))
        {
            static Entity *nodeEntity=NULL;
            if (!nodeEntity)  {
                nodeEntity=mSceneMgr->createEntity("nodeEntity", "robot.mesh"); 
                // MaterialPtr nodeMaterial = MaterialManager::getSingleton().create("Hummer", "Hummer");
                // nodeMaterial->setReceiveShadows(true);
                // nodeMaterial->getTechnique(0)->setLightingEnabled(true);

            }

            ent=nodeEntity->clone(entId);
            // ent->setMaterialName("Hummer"); 
            ent->setCastShadows(true); 
            sNode=mSceneMgr->getRootSceneNode()->createChildSceneNode(sceneNodeId);
            sNode->scale(0.2, 0.2, 0.2); 
            sNode->attachObject(ent);
        }
        else
        {
            ent=mSceneMgr->getEntity(entId);
            if(!mSceneMgr->hasSceneNode(sceneNodeId))
            {
                LOG_WARN("Found a entity " << entId << " without a scene node attached " << sceneNodeId << " - should not happen"); 
                TRACE_EXIT_RET_BOOL(false);
                return false;
            }
            else
                sNode=mSceneMgr->getSceneNode(sceneNodeId);
        }
        TRACE_EXIT();
        return true;
    }

    bool WatcherMessageFrameListener::handleEdgeMessage(const EdgeMessagePtr &edgeMessage)
    {
        TRACE_ENTER();

        string node1Id=edgeMessage->node1.to_string(); 
        string node2Id=edgeMessage->node2.to_string(); 

        // If we don't have the nodes at both ends of the edge, there's not much point in continuing.
        Entity *node1Entity, *node2Entity;
        SceneNode *node1SceneNode, *node2SceneNode; 
        if (!mSceneMgr->hasEntity(node1Id) || !mSceneMgr->hasSceneNode(node1Id+sceneNodeTag) ||
            !mSceneMgr->hasEntity(node2Id) || !mSceneMgr->hasSceneNode(node2Id+sceneNodeTag))
        {
            TRACE_EXIT_RET_BOOL(false);
            return false;
        }
        else
        {
            node1Entity=mSceneMgr->getEntity(node1Id);
            node2Entity=mSceneMgr->getEntity(node2Id);
            node1SceneNode=mSceneMgr->getSceneNode(node1Id+sceneNodeTag);
            node2SceneNode=mSceneMgr->getSceneNode(node2Id+sceneNodeTag);
        }

        // We have nodes, do we have an existing edge? If not create it.
        string edgeId;
        if (node1Id>node2Id)
            edgeId=node1Id+string("-")+node2Id;
        else
            edgeId=node2Id+string("-")+node1Id;

        string edgeSceneNodeId(edgeId+sceneNodeTag);

        ManualObject *edgeEntity;
        if (mSceneMgr->hasManualObject(edgeId))
            edgeEntity=mSceneMgr->getManualObject(edgeId);
        else
        {
            edgeEntity=mSceneMgr->createManualObject(edgeId);

            SceneNode *edgeSceneNode=mSceneMgr->getRootSceneNode()->createChildSceneNode(edgeSceneNodeId);
            MaterialPtr edgeMaterial=MaterialManager::getSingleton().create(edgeId+string("Material"), "edgeMaterials", true); 

            edgeMaterial->setReceiveShadows(false); 
            edgeMaterial->getTechnique(0)->setLightingEnabled(true); 
            edgeMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0); 
            edgeMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1); 

            Color c=edgeMessage->edgeColor;
            edgeMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(c.r/255.0, c.g/255.0, c.b/255.0);

            edgeSceneNode->attachObject(edgeEntity); 
        }

        // GTL todo: replace "Vector3(0, 10, 0)" below with center of bounding boxes of the nodes
        Vector3 nodeBoundingBoxCenter(0, 10, 0); 

        // Now that we've got the edge and the positions of everyone, create a spline
        // between the nodes and contour it to the terrain. Then draw the spline.
        SimpleSpline spline;
        const int splinePoints=10;      // GTL todo: replace this with a configurable value.
        Vector3 point(node1SceneNode->getPosition()); // starts at node1
        Vector3 delta(node2SceneNode->getPosition()-node1SceneNode->getPosition()); 
        delta/=splinePoints;
        // Each point on the spline must be ground clamped
        for (unsigned int i=0; i<splinePoints; i++)
        {
            Ray pointRay(Vector3(point.x, 5000.0f, point.z), Vector3::NEGATIVE_UNIT_Y);
            mRaySceneQuery->setRay(pointRay);

            // Perform the scene query
            RaySceneQueryResult &result = mRaySceneQuery->execute();
            RaySceneQueryResult::iterator itr = result.begin();

            if (itr!=result.end() && itr->worldFragment)
            {
                if (itr->worldFragment->singleIntersection.y>point.y)
                    point.y=itr->worldFragment->singleIntersection.y;
            }
            spline.addPoint(point);
            point+=delta;
        }
        spline.addPoint(node2SceneNode->getPosition());

        edgeEntity->clear(); 
        edgeEntity->begin(edgeId+string("Material"), RenderOperation::OT_LINE_STRIP);
        const int splineSections=100;    // GTL todo: make this configurable
        for (unsigned int i=0; i<splineSections; i++)
            edgeEntity->position(spline.interpolate(Real(i)/splineSections)+nodeBoundingBoxCenter); 
        edgeEntity->end();

        TRACE_EXIT_RET_BOOL(true);
        return true;
    }

    bool WatcherMessageFrameListener::updateNodeLocation(const GPSMessagePtr &gpsMessage)
    {
        TRACE_ENTER();

        LOG_DEBUG("Creating node"); 

        string nid=gpsMessage->fromNodeID.to_string();
        string sceneNid(nid+sceneNodeTag); 

        Entity *ent;
        SceneNode *sNode;
        if (!getEntityAndSceneNode(nid, sceneNid, ent, sNode))
        {
            TRACE_EXIT_RET_BOOL(false);
            return false;
        }

        // Force the node to be in the terrain x-z boundary
        // GTL todo

        // Don't let the node go beneath the terrain
        Vector3 position;
        gps2PixelLocation(gpsMessage, position);

        Ray nodeRay(Vector3(position.x, 5000.0f, position.z), Vector3::NEGATIVE_UNIT_Y);
        mRaySceneQuery->setRay(nodeRay);
        
        // Perform the scene query
        RaySceneQueryResult &result = mRaySceneQuery->execute();
        RaySceneQueryResult::iterator itr = result.begin();
        
        if (itr!=result.end() && itr->worldFragment)
        {
            if (itr->worldFragment->singleIntersection.y>position.y)
                position.y=itr->worldFragment->singleIntersection.y;
        }

        sNode->setPosition(position); 

        TRACE_EXIT_RET_BOOL(true);
        return true;
    }

    void WatcherMessageFrameListener::gps2PixelLocation(const GPSMessagePtr &message, Vector3 &ogreLocation)
    {
        TRACE_ENTER();

        double x, y, z;
        double inx=message->x*gpsScale, iny=message->z, inz=-message->y*gpsScale; // NOTE: gpsMessage.y == latitude == z vector and gps message's z == alt == ogre's y vector

        if (message->dataFormat==GPSMessage::UTM)
        {
            // There is no UTM zone information in the UTM GPS packet, so we assume all data is in a single
            // zone. Because of this, no attempt is made to place the nodes in the correct positions on the 
            // planet surface. We just use the "lat" "long" data as pure x and y coords in a plane, offset
            // by the first coord we get. (Nodes are all centered around 0,0 where, 0,0 is defined 
            // by the first coord we receive. 
            //
            if (iny < 91 && inx > 0) 
                LOG_WARN("Received GPS data that looks like lat/long in degrees, but GPS data format mode is set to UTM in cfg file."); 

            static double utmXOffset=0.0, utmZOffset=0.0;
            static bool utmOffInit=false;
            if (utmOffInit==false)
            {
                utmOffInit=true;
                utmXOffset=inx;
                utmZOffset=inz; 

                LOG_INFO("Got first UTM coordinate. Using it for x and z offsets for all other coords. Offsets are: x=" << utmXOffset << " y=" << utmZOffset);
            }

            x=inx+utmXOffset;
            y=iny;
            z=inz+utmZOffset;    
        }
        else // default to lat/long/alt WGS84
        {
            if (inx > 180)
                LOG_WARN("Received GPS data that may be UTM (long>180), but GPS data format mode is set to lat/long degrees in cfg file."); 

            static double xOff=0.0, zOff=0.0;
            static bool xOffInit=false;
            if (xOffInit==false)
            {
                xOffInit=true;
                xOff=inx;
                zOff=inz;

                LOG_INFO("Got first Lat/Long coordinate. Using it for x and y offsets for all other coords. Offsets are: x=" 
                        << xOff << " z=" << zOff);
            }

            x=inx+xOff;
            y=iny;
            z=inz+zOff;
        }
        LOG_DEBUG("Converted GPS from " << inx << ", " << iny << ", " << inz << " to " << x << ", " << y << ", " << z); 

        static double terrainCenterX=500.0, terrainCenterZ=500.0;  // GTL todo: make this dynamic with the terrain loaded.

        ogreLocation.x=x+terrainCenterX;
        ogreLocation.y=y;
        ogreLocation.z=z+terrainCenterZ;

        TRACE_EXIT();
    }
                
}


