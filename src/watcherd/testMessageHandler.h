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

            bool produceReply(const boost::shared_ptr<Message> &request, boost::shared_ptr<Message> &reply);
            bool produceRequest(boost::shared_ptr<Message> &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // GPS_MESSAGE_STATUS_HANDLER_H
