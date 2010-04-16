/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

/** @file subscribeStreamMessage.h
 */
#ifndef SUBSCRIBE_STREAM_MESSAGE_H
#define SUBSCRIBE_STREAM_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {
	    /** Allows a Watcher GUI to subscribe to a specified event stream, or create a new one. */
        class SubscribeStreamMessage : public Message {
            public:
            SubscribeStreamMessage(uint32_t); 

	    /** The uid of the stream to join.  uid==-1 means "create a new stream" */
	    uint32_t uid;

            private:
            friend class boost::serialization::access;
            template <typename Archive> void serialize(Archive& ar, const unsigned int version);
            DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<SubscribeStreamMessage> SubscribeStreamMessagePtr;
    }
}
#endif
