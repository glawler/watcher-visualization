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
     * clients of the class. MSR==Message Stream Reactor.
     */
    class MSRImpl {
        public: 
            MSRImpl(MessageStreamPtr ms) : mStream(ms), ioThread(NULL) {
            }; 
            ~MSRImpl() {
                if (ioThread) {
                    ioThread->interrupt(); 
                    ioThread->join(); 
                    delete ioThread;
                    ioThread=NULL;
                }
            } 
            
            boost::thread *ioThread;

            // These need to be very fast lookup. 
            // Maybe use tr1 unordered_list
            // or a hash
            typedef std::map<std::string, bool> SeenLayerMap;
            SeenLayerMap seenLayerMap;
            typedef std::map<unsigned int, bool> SeenNodeMap;
            SeenNodeMap seenNodeMap;

            MessageStreamPtr mStream;

            // keep track of callbacks in multimap indexed by message type
            typedef std::multimap<watcher::event::MessageType, MessageStreamReactor::MessageCallbackFunction> CallbackTable;
            typedef std::pair<watcher::event::MessageType, MessageStreamReactor::MessageCallbackFunction> CallbackTablePair;
            typedef std::pair<CallbackTable::const_iterator, CallbackTable::const_iterator> CallbackRange;
            CallbackTable callbackTable; 

            // Keep track of non-generic callbacks in vectors of functions.
            typedef std::vector<MessageStreamReactor::NodeLocationUpdateFunction> NodeLocationUpdateFunctions; 
            NodeLocationUpdateFunctions nodeLocationUpdateFunctions;

            // Keep track of generic callbacks in vector as well.
            typedef std::vector<MessageStreamReactor::MessageCallbackFunction> CallbackVector;
            CallbackVector feederMessageCallbacks;
            CallbackVector controlMessageCallbacks;
            CallbackVector newNodeSeenCallbacks;
            CallbackVector newLayerSeenCallbacks;

            void doMessageCallbacks(const CallbackVector &v, MessagePtr m) { 
                MSRImpl::CallbackVector::const_iterator i;
                for (i=v.begin(); i!=v.end(); ++i)
                    (*i)(m); 
            }
    };
    MessageStreamReactor::MessageStreamReactor(MessageStreamPtr ms) : 
        impl(new MSRImpl(ms)) {
        TRACE_ENTER();
        impl->mStream=ms;
        if (!impl->ioThread) 
            impl->ioThread=new boost::thread(boost::bind(&MessageStreamReactor::getMessageLoop, this));
        TRACE_EXIT();
    }
    MessageStreamReactor::~MessageStreamReactor() {
    }
    void MessageStreamReactor::addNodeLocationUpdateFunction(NodeLocationUpdateFunction f) {
       impl->nodeLocationUpdateFunctions.push_back(f);  
    }
    void MessageStreamReactor::addNewNodeSeenCallback(MessageCallbackFunction f) {
       impl->newNodeSeenCallbacks.push_back(f);  
    }
    void MessageStreamReactor::addNewLayerSeenCallback(MessageCallbackFunction f) {
       impl->newLayerSeenCallbacks.push_back(f);  
    }
    void MessageStreamReactor::addFeederMessageCallback(MessageCallbackFunction f) {
       impl->feederMessageCallbacks.push_back(f);  
    }
    void MessageStreamReactor::addControlMessageCallback(MessageCallbackFunction f) {
       impl->controlMessageCallbacks.push_back(f);  
    }
    void MessageStreamReactor::addMessageTypeCallback(MessageCallbackFunction f, watcher::event::MessageType t) {
        impl->callbackTable.insert(MSRImpl::CallbackTablePair(t,f)); 
    }
    void MessageStreamReactor::getMessageLoop() {
        while (true) {
            this_thread::interruption_point();
            MessagePtr message;
            while(impl->mStream && impl->mStream->getNextMessage(message)) {
                LOG_DEBUG("Got message in MessageStreamReactor::getMessageLoop, type: " << message->type); 
                if (!isFeederEvent(message->type)) {
                    impl->doMessageCallbacks(impl->controlMessageCallbacks, message); 
                    LOG_DEBUG("Sent control message to " << impl->controlMessageCallbacks.size() << " subscribers"); 
                }
                else {
                    impl->doMessageCallbacks(impl->feederMessageCallbacks, message); 
                    LOG_DEBUG("Sent feeder message to " << impl->feederMessageCallbacks.size() << " subscribers"); 
                    if (impl->seenNodeMap.end()==impl->seenNodeMap.find(message->fromNodeID.to_v4().to_ulong())) {
                        impl->doMessageCallbacks(impl->newNodeSeenCallbacks, message); 
                        LOG_DEBUG("Invoked new node callback " << impl->newNodeSeenCallbacks.size() << " times."); 
                        impl->seenNodeMap[message->fromNodeID.to_v4().to_ulong()]=true;
                    }
                    if (message->type==GPS_MESSAGE_TYPE) {
                        GPSMessagePtr gm=boost::dynamic_pointer_cast<GPSMessage>(message);
                        MSRImpl::NodeLocationUpdateFunctions::const_iterator i;
                        for (i=impl->nodeLocationUpdateFunctions.begin(); i!=impl->nodeLocationUpdateFunctions.end(); ++i)
                            (*i)(gm->x, gm->y, gm->z, gm->fromNodeID.to_string()); 
                        LOG_DEBUG("Invoked node location update callback " << impl->nodeLocationUpdateFunctions.size() << " times."); 
                    }
                    GUILayer layer;
                    if (hasLayer(message, layer) && impl->seenLayerMap.end()==impl->seenLayerMap.find(layer)) {
                        impl->doMessageCallbacks(impl->newLayerSeenCallbacks, message); 
                        LOG_DEBUG("Invoked new layer callback " << impl->newLayerSeenCallbacks.size() << " times."); 
                        impl->seenLayerMap[layer]=true;
                    }
                }
                if (impl->callbackTable.size()) { 
                    MSRImpl::CallbackRange range;
                    range=impl->callbackTable.equal_range(message->type); 
                    unsigned int cnt=0;
                    for (MSRImpl::CallbackTable::const_iterator i=range.first; i!=range.second; ++i, cnt++)
                        i->second(message); 
                    LOG_DEBUG("Sent type " << message->type << " message to " << cnt << " subscribers"); 
                }
            }
        }
    }
} // namespace

