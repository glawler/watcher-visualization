#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>

#include "logger.h"
#include "messageTypesAndVersions.h"
#include "watcherTypes.h"
#include "message_fwd.h"

namespace watcher {
    namespace event {

        /** Base class for all messages generated from the test node daemon.
         */
        class Message {
            public:
                // de-serialize object via input stream
                static MessagePtr unpack(std::istream&);

                // seralize object to ostream
                void pack(std::ostream&) const;

                unsigned int version;
                MessageType type;
                Timestamp timestamp;  
                NodeIdentifier fromNodeID;

                Message();
                Message(const MessageType &t, const unsigned int version);
                Message(const Message &other);

                virtual ~Message();

                bool operator==(const Message &other) const;
                Message &operator=(const Message &other);

                // output the class as a stream. Derived classes 
                // should call the base classes' implementation.
                virtual std::ostream &toStream(std::ostream &out) const;
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
