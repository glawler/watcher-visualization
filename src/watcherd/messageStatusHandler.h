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

            ConnectionCommand produceRequest(MessagePtr &request);
            ConnectionCommand produceReply(const MessagePtr &request, MessagePtr &reply);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // MESSAGE_STATUS_HANDLER_H
