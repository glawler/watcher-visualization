#ifndef TEST_MESSAGE_STATUS_HANDLER_H
#define TEST_MESSAGE_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class TestMessageHandler : public MessageHandler
    {
        public:
            TestMessageHandler();
            ~TestMessageHandler();

            ConnectionCommand produceRequest(event::MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // GPS_MESSAGE_STATUS_HANDLER_H
