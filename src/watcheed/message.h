#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H

#include "messageTypesAndVersions.h"
#include "logger.h"

namespace watcher 
{
    class Message 
    {
        public:

            unsigned int version;
            MessageType type;

            Message();
            Message(const MessageType &t, const unsigned int version);
            Message(const Message &other);

            virtual ~Message();

            bool operator==(const Message &other) const;
            Message &operator=(const Message &other);

            std::ostream &operator<<(std::ostream &out) const;

            template<class Archive>
                void serialize(Archive& ar, const unsigned int /*version*/)
                {
                    ar & version & type;
                }

            DECLARE_LOGGER();

        protected:
        private:
    };

    std::ostream &operator<<(std::ostream &out, const Message &mess);
}

#endif // BASE_MESSAGE_H
