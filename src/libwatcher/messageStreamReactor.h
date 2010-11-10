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
#include "messageTypesAndVersions.h"
#include "messageStream.h"
#include "declareLogger.h"

namespace watcher {

    /** forward decl for private implementation details. */
    class MessageStreamReactorImpl;

    /**
     * MessageStreamReactor is a wrapper around messages stream that 
     * calls functions when there is an action on a message stream. 
     */
    class MessageStreamReactor
    {
        public: 
            /** 
             * @param ms The messageStream to monitor for events. 
             * @param reconnect if true, have this class reconnect if the message stream
             * gets disconnected. 
             */
            MessageStreamReactor(MessageStreamPtr ms, bool reconnect=true);
            virtual ~MessageStreamReactor();

            /**
             * seems like a lot of work to implement function pointers/callbacks. 
             * code sample:
             * MessageStreamReactor *msr(new MessageStreamReactor(messageStream); 
             * msr->nodeLocationUpdateFunction=
             *      boost::bind(&myClass::myFunc, this, _1, _2, _3, _4, _5); 
             */
            typedef boost::function<bool (double x, double y, double z, unsigned long nodeID)> NodeLocationUpdateFunction;
            NodeLocationUpdateFunction nodeLocationUpdateFunction;

            typedef boost::function<void (MessagePtr m)> MessageCallbackFunction;
            MessageCallbackFunction gotFeederMessage;
            MessageCallbackFunction gotDataMessage;
            MessageCallbackFunction newNodeSeen;
            MessageCallbackFunction newLayerSeen;

            /**
             * Callback for specfic message type.
             */
            void addMessageTypeCallback(MessageCallbackFunction, watcher::event::MessageType type); 

        protected:

        private:

            DECLARE_LOGGER(); 
            void getMessageLoop(); 
            boost::scoped_ptr<MessageStreamReactorImpl> impl;
            
            MessageStreamReactor(const MessageStreamReactor &); // no copies
    };
} // namespace


#endif /* WATCHER_MESSAGE_STREAM_REACTOR_H */


