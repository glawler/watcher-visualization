#ifndef CLIENT_MESSAGE_HANDLER_H
#define CLIENT_MESSAGE_HANDLER_H

#include "messageHandler.h"

namespace watcher 
{
    class WatcherdAPIMessageHandler : 
        public MessageHandler
    {
        public:
            /// Construct with a directory containing files to be served.
            WatcherdAPIMessageHandler();
            virtual ~WatcherdAPIMessageHandler(); 

            /**
             * Notification of message arrival on the client. Client just checks for 
             * an ack or OK message. 
             *
             * @param[in] - message - the newly arrived message. 
             * @return - boolean. If true, keep connection open, close otherwise. Watcherd API 
             * usually leaves teh connection open to rec'v message streams.
             */
            virtual bool handleMessageArrive(const event::MessagePtr &message);
            virtual bool handleMessagesArrive(const std::vector<event::MessagePtr> &messages); 

            /**
             * Notification that a message has been successfully sent. Watcherd API clients
             * generally want a reponse to any message they send so always return true.
             *
             * @param[in] - the message that was sent
             * @return - boolean. If true, expect a response, else, close connection.
             */
            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<WatcherdAPIMessageHandler> WatcherdAPIMessageHandlerPtr;

} // namespace 

#endif //  CLIENT_MESSAGE_HANDLER_H

