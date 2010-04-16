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

/** @file subscribeStreamMessage.cpp
 */
#include "watcherSerialize.h"
#include "subscribeStreamMessage.h"
#include "logger.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(SubscribeStreamMessage, "Message.SubscribeStreamMessage");

        SubscribeStreamMessage::SubscribeStreamMessage(uint32_t uid_ = -1) : Message(SUBSCRIBE_STREAM_MESSAGE_TYPE, SUBSCRIBE_STREAM_MESSAGE_VERSION),
	uid(uid_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        template <typename Archive> void SubscribeStreamMessage::serialize(Archive& ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
	    ar & uid;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::SubscribeStreamMessage)
