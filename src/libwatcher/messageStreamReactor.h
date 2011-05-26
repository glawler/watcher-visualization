/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @file messageStreamReactor.h
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2010-11-10
 */
#ifndef WATCHER_MESSAGE_STREAM_REACTOR_H
#define WATCHER_MESSAGE_STREAM_REACTOR_H

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include "libwatcher/messageTypesAndVersions.h"
#include "libwatcher/messageStream.h"
#include "declareLogger.h"

namespace watcher {

    /** forward decl for private implementation details, please ignore. */
    class MSRImpl;

    /**
     * MessageStreamReactor is a wrapper around messages stream that 
     * calls functions when there is an action on a message stream. 
     */
    class MessageStreamReactor
    {
        public: 
            /** 
             * @param ms The messageStream to monitor for events. 
             */
            MessageStreamReactor(MessageStreamPtr ms); 
            virtual ~MessageStreamReactor();

            /**
             * seems like a lot of work to implement function pointers/callbacks. 
             * code sample that adds a class instance's member function as a callback
             * looks much like this ugly ugly code:
             * MessageStreamReactor *msr(new MessageStreamReactor(messageStream); 
             * msr->addNodeLocationUpdateFunction(
             *      boost::bind(&myClass::myFunc, this, _1, _2, _3, _4, _5)); 
             */
            typedef boost::function<bool (double x, double y, double z, const std::string &nodeID)> NodeLocationUpdateFunction;

            /** Add a function callback when a node's GPS data is updated. */
            void addNodeLocationUpdateFunction(NodeLocationUpdateFunction); 

            /**
             * Basic function callback is a function called with the relevant message.
             */
            typedef boost::function<void (MessagePtr m)> MessageCallbackFunction;

            /** Called when a feeder message arrives. */
            void addFeederMessageCallback(MessageCallbackFunction); 

            /** Called when a control message arrives. */
            void addControlMessageCallback(MessageCallbackFunction); 

            /** Called when a new node is seen for the first time. */
            void addNewNodeSeenCallback(MessageCallbackFunction); 

            /** Called when a new layer is seen for the first time. */
            void addNewLayerSeenCallback(MessageCallbackFunction); 

            /**
             * Callback for specfic message type. A generic "subscription" API. 
             */
            void addMessageTypeCallback(MessageCallbackFunction, watcher::event::MessageType type); 

        protected:

        private:

            DECLARE_LOGGER(); 
            void getMessageLoop(); 
            MessageStreamReactor(const MessageStreamReactor &); // no copies

            // most of the private stuff for this class is hidden in the .cpp 
            boost::scoped_ptr<MSRImpl> impl;
    };
} // namespace


#endif /* WATCHER_MESSAGE_STREAM_REACTOR_H */


