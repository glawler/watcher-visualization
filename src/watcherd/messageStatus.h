#ifndef STATUS_MESSAGE_H
#define STATUS_MESSAGE_H

#include "logger.h"
#include "message.h"
#include "messageTypesAndVersions.h"

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

namespace watcher 
{
    class MessageStatus : public Message
    {
        public:

            typedef enum 
            {
                status_ok,
                status_error,
                status_ack,
                status_nack,
                status_disconnected
            } Status;

            Status status;
 
            MessageStatus(const Status stat=status_ok);
            MessageStatus(const MessageStatus &other);

            virtual ~MessageStatus();

            bool operator==(const MessageStatus &other) const;
            MessageStatus &operator=(const MessageStatus &other);

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            static const char *statusToString(const Status &stat);

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        protected:
        private:
            DECLARE_LOGGER();

    };

    typedef boost::shared_ptr<MessageStatus> MessageStatusPtr;
    std::ostream &operator<<(std::ostream &out, const MessageStatus &mess);

}

#endif // STATUS_MESSAGE_H
