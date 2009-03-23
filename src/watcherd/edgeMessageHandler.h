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

            ConnectionCommand produceRequest(event::MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // LABEL_MESSAGE_STATUS_HANDLER_H
