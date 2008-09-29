#ifndef DATA_REQUEST_STATUS_HANDLER_H
#define DATA_REQUEST_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class DataRequestMessageHandler : public MessageHandler
    {
        public:
            DataRequestMessageHandler();
            ~DataRequestMessageHandler();

            ConnectionCommand produceRequest(MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // DATA_REQUEST_STATUS_HANDLER_H
