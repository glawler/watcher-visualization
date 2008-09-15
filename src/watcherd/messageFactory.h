#ifndef MESSAGE_FACTORY_SDFSDFSDFDGSFLNKSADKJNSD
#define MESSAGE_FACTORY_SDFSDFSDFDGSFLNKSADKJNSD

#include "boost/shared_ptr.hpp"
#include "messageTypesAndVersions.h"
#include "message.h"

namespace watcher
{
    class MessageFactory 
    {
        public:

            static boost::shared_ptr<Message> makeMessage(const MessageType &type);

            DECLARE_LOGGER();

        protected:
        private:

            MessageFactory();
            ~MessageFactory();
    };
}

#endif // MESSAGE_FACTORY_SDFSDFSDFDGSFLNKSADKJNSD
