#ifndef MESSAGE_HANDLER_FACTORY_H
#define MESSAGE_HANDLER_FACTORY_H

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <libwatcher/messageTypesAndVersions.h>
#include "messageHandler.h"

namespace watcher 
{
    class MessageHandlerFactory : public boost::noncopyable
    {
        public:

            // static function that returns a pointer to a MessageHandler class based on the 
            // message type you want to handle. 
            static MessageHandlerPtr getMessageHandler(const event::MessageType &type);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // MESSAGE_HANDLER_FACTORY_H
