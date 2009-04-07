#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>

#include "logger.h"
#include "messageTypesAndVersions.h"
#include "watcherTypes.h"

namespace watcher {
    namespace event {

        /** Base class for all messages generated from the test node daemon.
         */
        class Message {
            public:
                unsigned int version;
                MessageType type;
                Timestamp timestamp;  

                Message();
                Message(const MessageType &t, const unsigned int version);
                Message(const Message &other);

                virtual ~Message();

                bool operator==(const Message &other) const;
                Message &operator=(const Message &other);

                // output the class as a stream. Derived classes 
                // should call the base classes' implementation.
                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            protected:
            private:
                friend class boost::serialization::access;
                template <typename Archive>
                void serialize(Archive & ar, const unsigned int file_version)
                {
                    TRACE_ENTER();
                    ar & version;
                    ar & type;
                    ar & timestamp;
                    TRACE_EXIT();
                }

                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<Message> MessagePtr; 
        std::ostream &operator<<(std::ostream &out, const Message &mess);
    }
}
#endif // BASE_MESSAGE_H
