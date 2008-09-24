#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include <boost/shared_ptr.hpp>

#include "logger.h"
#include "messageTypesAndVersions.h"

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

namespace watcher 
{
    typedef long long int Timestamp;    // in Epoch milliseconds

    class Message 
    {
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

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        protected:
        private:
            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<Message> MessagePtr; 
    std::ostream &operator<<(std::ostream &out, const Message &mess);
}

#endif // BASE_MESSAGE_H
