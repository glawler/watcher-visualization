#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>

#include "logger.h"
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
                /** de-serialize object via input stream */
                static MessagePtr unpack(std::istream&);

                /** seralize object to ostream */
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
                 * @arg t, the type of the message
                 * @arg version, the message's version
                 */
                Message(const MessageType &t, const unsigned int version);

                /** Create a copy of a message */
                Message(const Message &other);

                /** And this too shall pass */
                virtual ~Message();

                /** Compare this message against another to see if they are equal.
                 * @arg other, the other message
                 * @returns bool, true is equal, false otherwise.
                 */
                bool operator==(const Message &other) const;

                /** Set this message equal to another
                 * @arg other, the message to set this message equal to
                 * @ret Message, a reference to this instance
                 */
                Message &operator=(const Message &other);

                /** Write this message to <b>out</b> in human readable format 
                 * @arg out, the stream to write to
                 * @return the stream that was written to
                 */
                virtual std::ostream &toStream(std::ostream &out) const;

                /** Write this message to <b>out</b> in human readable format 
                 * @arg out, the stream to write to
                 * @return the stream that was written to
                 */
                inline std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            protected:
            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };


        std::ostream &operator<<(std::ostream &out, const Message &mess);
    }
}
#endif // BASE_MESSAGE_H
