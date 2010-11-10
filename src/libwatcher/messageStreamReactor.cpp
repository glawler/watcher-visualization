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
 * @file messageStreamReactor.cpp
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2010-11-10
 */

#include "messageStreamReactor.h"
#include "logger.h"
#include "gpsMessage.h"

namespace watcher {

    using namespace boost;

    INIT_LOGGER(MessageStreamReactor, "MessageStreamReactor");

    /** 
     * Implementation class for MSR. Hides gritty details from 
     * clients of the class. 
     */
    class MessageStreamReactorImpl {
        public: 
            MessageStreamReactorImpl(MessageStreamPtr ms) {}; 
            ~MessageStreamReactorImpl() {
                if (ioThread) {
                    ioThread->interrupt(); 
                    ioThread->join(); 
                    delete ioThread;
                    ioThread=NULL;
                }
            } 
            
            MessageStreamPtr ms; 
            boost::thread *ioThread;

            // These need to be very fast lookup. 
            typedef std::map<std::string, bool> SeenLayerMap;
            SeenLayerMap seenLayerMap;
            typedef std::map<unsigned int, bool> SeenNodeMap;
            SeenNodeMap seenNodeMap;

            MessageStreamPtr mStream;
            bool reconnect;

            typedef std::map<watcher::event::MessageType, MessageStreamReactor::MessageCallbackFunction> CallbackTable;
            CallbackTable callbackTable; 
    };

    MessageStreamReactor::MessageStreamReactor(MessageStreamPtr ms, bool recon) : 
        impl(new MessageStreamReactorImpl(ms)) {
        TRACE_ENTER();
        impl->mStream=ms;
        impl->reconnect=recon;
        if (!impl->ioThread) 
            impl->ioThread=new boost::thread(boost::bind(&MessageStreamReactor::getMessageLoop, this));
        TRACE_EXIT();
    }
    MessageStreamReactor::~MessageStreamReactor() {
    }
    void MessageStreamReactor::addMessageTypeCallback(MessageCallbackFunction f, watcher::event::MessageType type) {
        impl->callbackTable[type]=f;
    }
    void MessageStreamReactor::getMessageLoop() {
        while (true) {
            while (impl->reconnect && !impl->mStream->connected()) {
                this_thread::interruption_point();
                if (!impl->mStream->connect(true)) {
                    LOG_WARN("Unable to connect to server. Trying again in 1 second");
                    sleep(1);
                }
            }
            this_thread::interruption_point();
            MessagePtr message;
            while(impl->mStream && impl->mStream->getNextMessage(message)) {
                if (isFeederEvent(message->type) && gotFeederMessage)
                    gotFeederMessage(message); 
                else {
                    if (gotDataMessage) 
                        gotDataMessage(message); 
                    if (impl->seenNodeMap.end()!=impl->seenNodeMap.find(message->fromNodeID.to_v4().to_ulong())) {
                        if (newNodeSeen) 
                            newNodeSeen(message); 
                        impl->seenNodeMap[message->fromNodeID.to_v4().to_ulong()]=true;
                    }
                    if (message->type==GPS_MESSAGE_TYPE && nodeLocationUpdateFunction) {
                        GPSMessagePtr gm=boost::dynamic_pointer_cast<GPSMessage>(message);
                        nodeLocationUpdateFunction(gm->x, gm->y, gm->z, gm->fromNodeID.to_v4().to_ulong()); 
                    }
                    GUILayer layer;
                    if (hasLayer(message, layer) && impl->seenLayerMap.end()!=impl->seenLayerMap.find(layer)) {
                        if (newLayerSeen) 
                            newLayerSeen(message); 
                        impl->seenLayerMap[layer]=true;
                    }
                }
                // GTL - holy shit this is ugly. 
                if (impl->callbackTable.size()) { 
                    MessageStreamReactorImpl::CallbackTable::iterator f=impl->callbackTable.find(message->type); 
                    if (f!=impl->callbackTable.end())
                        f->second(message); 
                }
            }
        }
    }
} // namespace

