#ifndef CLIENT_MESSAGE_HANDLER_H
#define CLIENT_MESSAGE_HANDLER_H

#include "messageHandler.h"

namespace watcher 
{
    class ClientMessageHandler : 
        public MessageHandler
    {
        public:
            /// Construct with a directory containing files to be served.
            ClientMessageHandler();
            virtual ~ClientMessageHandler(); 

            /**
             * Notification of message arrival on the client. Client just checks for 
             * an ack or OK message. 
             *
             * @param[in] - message - the newly arrived message. 
             * @param[out] - response. If non-null, this message will be sent back to the entity
             * which sent 'message' as a response.
             * @return - boolean. true, if message handled, false on error or otherwise. If the return
             * value is negative, the response message will not be sent even if non-null.
             */
            virtual bool handleMessageArrive(const event::MessagePtr &message, event::MessagePtr &response);
            virtual bool handleMessagesArrive(const std::vector<event::MessagePtr> &messages, event::MessagePtr &response);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<ClientMessageHandler> ClientMessageHandlerPtr;

} // namespace 

#endif //  CLIENT_MESSAGE_HANDLER_H

