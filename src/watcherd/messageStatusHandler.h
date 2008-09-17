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

            bool produceReply(const boost::shared_ptr<Message> &request, boost::shared_ptr<Message> &reply);
            bool produceRequest(boost::shared_ptr<Message> &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // MESSAGE_STATUS_HANDLER_H
