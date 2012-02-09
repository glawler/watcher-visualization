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
 * @file message.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include <boost/shared_ptr.hpp>
#include <yaml-cpp/yaml.h>

#include "declareLogger.h"
#include "messageTypesAndVersions.h"
#include "watcherTypes.h"
#include "message_fwd.h"

namespace watcher {
    /** 
     * @namespace watcher::event
     * This namespace holds the messages that make up the API between the test node daemons and the GUIs and the watcher daemon 
     */
    namespace event {

        /** 
         * Base class for all messages generated from the test node daemon.
         */
		class Message {
			public:
				/** de-serialize object via input stream.
				 * Due to limitation on underlying parse library, 
				 * each message must be in a single stream. If you pass
				 * a stream that contains more than one message, the others will be 
				 * silently ignored. There is a change request in with the 
				 * author of the parser library to fix this. 
				 * (yaml-cpp issue #148). 
				 */
				static MessagePtr unpack(std::istream &);

				/** Create and unpack a message directly from a YAML node */
				static MessagePtr unpack(YAML::Node &node); 

				/** seralize object to ostream. Please be aware that ::pack
				 * will generate a single YAML document containing the message
				 * data. 
				 */
				void pack(std::ostream&) const;

				/** The version of this message. All versions are defined in \ref messageTypesAndVersions.h */
				unsigned int version;

				/** The unique type of this message. All types are defined in \ref messageTypesAndVersions.h */
				MessageType type;

				/** When the message waas sent. Specified in Unix epoch milliseconds */
				Timestamp timestamp;  

				/** The node that the message was sent from. If not set, the daemon will add it from the ip header when 
				 * it gets the message */
				NodeIdentifier fromNodeID;

				/** Create a message. Should not be done directly */
				Message();

				/** Create a Message 
				 * @param t the type of the message
				 * @param version the message's version
				 */
				Message(const MessageType &t, const unsigned int version);

				/** Create a copy of a message */
				Message(const Message &other);

				/** And this too shall pass */
				virtual ~Message();

				/** Compare this message against another to see if they are equal.
				 * @param other the other message
				 * @return bool true is equal, false otherwise.
				 */
				bool operator==(const Message &other) const;

				/** Set this message equal to another
				 * @param other the message to set this message equal to
				 * @return a reference to this instance
				 */
				Message &operator=(const Message &other);

				/** Write this message to <b>out</b> in human readable format 
				 * @param out the stream to write to
				 * @return the stream that was written to
				 */
				virtual std::ostream &toStream(std::ostream &out) const;

				/** Write this message to <b>out</b> in human readable format 
				 * @param out the stream to write to
				 * @return the stream that was written to
				 */
				inline std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

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

			protected:
			private:
				void operator>>(const YAML::Node& in); 

				DECLARE_LOGGER();
		};


        std::ostream &operator<<(std::ostream &out, const Message &mess);
    }
}
#endif // BASE_MESSAGE_H
