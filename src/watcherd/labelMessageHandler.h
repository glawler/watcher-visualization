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

            bool produceReply(const boost::shared_ptr<Message> &request, boost::shared_ptr<Message> &reply);
            bool produceRequest(boost::shared_ptr<Message> &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // LABEL_MESSAGE_STATUS_HANDLER_H
