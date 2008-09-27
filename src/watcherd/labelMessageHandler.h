#ifndef LABEL_MESSAGE_STATUS_HANDLER_H
#define LABEL_MESSAGE_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class LabelMessageHandler : public MessageHandler
    {
        public:
            LabelMessageHandler();
            ~LabelMessageHandler();

            bool produceReply(const MessagePtr &request, MessagePtr &reply);
            bool produceRequest(MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // LABEL_MESSAGE_STATUS_HANDLER_H
