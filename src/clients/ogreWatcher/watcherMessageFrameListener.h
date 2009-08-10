/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file watcherMessageFrameListener.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-20
 */
#ifndef WATCHER_MESSAGE_FRAME_LISTENER_H 
#define WATCHER_MESSAGE_FRAME_LISTENER_H 

#include <string>
#include <Ogre.h>
#include <logger.h>

#include <messageStream.h>
#include <gpsMessage.h>
#include <edgeMessage.h>

namespace ogreWatcher
{
    /**
     * WatcherMessageFrameListener
     * A frame listener that checks for new messages from the watcher and processes them
     */
    class WatcherMessageFrameListener : public Ogre::FrameListener
    {
        public:

            WatcherMessageFrameListener(Ogre::SceneManager *sceneManger, Ogre::Camera *camera, const std::string &host, const std::string &service); 
            virtual ~WatcherMessageFrameListener();

            /** Use the frame started callback as an opportunity to check for 
             * new messages from the watcherd 
             */
            virtual bool frameStarted(const Ogre::FrameEvent& evt);

        protected:

            /** Update a node's location in the GUI. If not there, crreate it */
            bool updateNodeLocation(const watcher::event::GPSMessagePtr &gpsMessage);

            /** Update/create an edge in the GUI */
            bool handleEdgeMessage(const watcher::event::EdgeMessagePtr &edgeMessage);

            /** How much to scale the GPS coords by when converting to screen coords */
            double gpsScale;

        private:

            DECLARE_LOGGER();

            /** The message stream connection from the watcher daemon */
            watcher::MessageStreamPtr messageStream;

            /** The latest message gotten from the watcher daemon */
            watcher::MessagePtr newMessage;

            Ogre::SceneManager *mSceneMgr;
            Ogre::Camera *mCamera;
    
            /** Translate GPS coords into something that ogre can understand wrt scceen coordinates 
             * @param[in] message - the message that has the coords to translate
             * @param[out] ogreLocation - the GPS coods mapped to screen location
             */
            void gps2PixelLocation(const watcher::event::GPSMessagePtr &message, Ogre::Vector3 &ogreLocation);

            /** Our very own ray scene query */
            Ogre::RaySceneQuery *mRaySceneQuery;
   
            /** Helper function to get the node/edge and the associated scene node */
            bool getEntityAndSceneNode(const std::string &entId, const std::string &sceneNodeId, Ogre::Entity *&ent, Ogre::SceneNode *&sNode);
            std::string sceneNodeTag;
    };
}

#endif
