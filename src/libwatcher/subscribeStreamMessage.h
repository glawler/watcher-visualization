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

#include <yaml-cpp/yaml.h>
#include "message.h"

namespace watcher {
    namespace event {
	    /** Allows a Watcher GUI to subscribe to a specified event stream, or create a new one. */
        class SubscribeStreamMessage : public Message {
            public:
            SubscribeStreamMessage(int32_t uid=-1); 

		    /** The uid of the stream to join.  uid==-1 means "create a new stream" */
		    int32_t uid;

			/** Serialize this message using a YAML::Emitter
			 * @param e the emitter to serialize to
			 * @return the emitter emitted to.
			 */
			virtual YAML::Emitter &serialize(YAML::Emitter &e) const; 

			/** Serialize from a YAML::Parser. 
			 * @param p the Parser to read from 
			 * @return the parser read from. 
			 */
			virtual YAML::Node &serialize(YAML::Node &node); 

            private:
            DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<SubscribeStreamMessage> SubscribeStreamMessagePtr;
    }
}
#endif
