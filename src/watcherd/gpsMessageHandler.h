#ifndef GPS_MESSAGE_STATUS_HANDLER_H
#define GPS_MESSAGE_STATUS_HANDLER_H

#include "messageHandler.h"

namespace watcher
{
    class GPSMessageHandler : public MessageHandler
    {
        public:
            GPSMessageHandler();
            ~GPSMessageHandler();

            bool produceReply(const MessagePtr &request, MessagePtr &reply);
            bool produceRequest(MessagePtr &request);

            DECLARE_LOGGER(); 

        protected:
        private:
    };
}
#endif // GPS_MESSAGE_STATUS_HANDLER_H
