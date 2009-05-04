#ifndef CLIENT_MESSAGE_HANDLER_H
#define CLIENT_MESSAGE_HANDLER_H

#include "messageHandler.h"

namespace watcher 
{
    class FeederAPIMessageHandler : 
        public MessageHandler
    {
        public:
            /// Construct with a directory containing files to be served.
            FeederAPIMessageHandler();
            virtual ~FeederAPIMessageHandler(); 

            /**
             * Notification of message arrival on a feeder.
             *
             * @param[in] - message - the newly arrived message. 
             * @return - boolean. If true, keep connection open, close otherwise. FeederAPI usually 
             * closes the connection.
             */
            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);
            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages); 

            /**
             * Notification that a message has been successfully sent. Feeder API
             * clients generally return false as they do not expect a response from
             * the server. 
             *
             * @param[in] - the message that was sent
             * @return - boolean. If true, expect a response, else, close connection.
             */
            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<FeederAPIMessageHandler> FeederAPIMessageHandlerPtr;

} // namespace 

#endif //  CLIENT_MESSAGE_HANDLER_H

