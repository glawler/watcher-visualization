#ifndef COLOR_MESSAGE_STATUS_HANDLER_H
#define COLOR_MESSAGE_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class ColorMessageHandler : public MessageHandler
    {
        public:
            ColorMessageHandler();
            ~ColorMessageHandler();

            ConnectionCommand produceRequest(MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // COLOR_MESSAGE_STATUS_HANDLER_H
