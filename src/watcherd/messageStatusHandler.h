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

            ConnectionCommand produceRequest(event::MessagePtr &request);
            ConnectionCommand produceReply(const event::MessagePtr &request, event::MessagePtr &reply);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // MESSAGE_STATUS_HANDLER_H
