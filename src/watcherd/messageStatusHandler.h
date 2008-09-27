#ifndef MESSAGE_STATUS_HANDLER_H
#define MESSAGE_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class MessageStatusHandler : public MessageHandler
    {
        public:
            MessageStatusHandler();
            ~MessageStatusHandler();

            bool produceReply(const MessagePtr &request, MessagePtr &reply);
            bool produceRequest(MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // MESSAGE_STATUS_HANDLER_H
