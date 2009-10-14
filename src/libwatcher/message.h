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

#include "declareLogger.h"
#include "messageTypesAndVersions.h"
#include "watcherTypes.h"
#include "message_fwd.h"

namespace Marshal {
    class Input;
    class Output;
}

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
                /** de-serialize object via input stream */
                static MessagePtr unpack(std::istream&);

                /** seralize object to ostream */
                std::ostream& pack(std::ostream&) const;

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

                static MessagePtr create(MessageType);

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

            protected:
                /** write the message-specific payload to the specific ostream in the wire protocol format. */
                virtual std::ostream& packPayload(std::ostream&) const;

                virtual void readPayload(Marshal::Input&);

            private:
                DECLARE_LOGGER();
        };

        std::ostream &operator<<(std::ostream &out, const Message &mess);
    }
}
#endif // BASE_MESSAGE_H
