#ifndef EDGE_MESSAGE_STATUS_HANDLER_H
#define EDGE_MESSAGE_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class EdgeMessageHandler : public MessageHandler
    {
        public:
            EdgeMessageHandler();
            ~EdgeMessageHandler();

            bool produceReply(const boost::shared_ptr<Message> &request, boost::shared_ptr<Message> &reply);
            bool produceRequest(boost::shared_ptr<Message> &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // LABEL_MESSAGE_STATUS_HANDLER_H
