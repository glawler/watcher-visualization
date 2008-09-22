#ifndef MESSAGE_FACTORY_SDFSDFSDFDGSFLNKSADKJNSD
#define MESSAGE_FACTORY_SDFSDFSDFDGSFLNKSADKJNSD

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "messageTypesAndVersions.h"
#include "message.h"

namespace watcher
{
    class MessageFactory : private boost::noncopyable
    {
        public:

            static boost::shared_ptr<Message> makeMessage(const MessageType &type);

        protected:
        private:
            DECLARE_LOGGER();

            MessageFactory();
            ~MessageFactory();
    };
}

#endif // MESSAGE_FACTORY_SDFSDFSDFDGSFLNKSADKJNSD
