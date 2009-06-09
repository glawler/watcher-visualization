#ifndef GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H
#define GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H

#include "libwatcher/messageHandler.h"

namespace watcher 
{
    class ServerMessageHandler : public MessageHandler
    {
        public:
            /// Construct with a directory containing files to be served.
            ServerMessageHandler();
            virtual ~ServerMessageHandler(); 

            /**
             * Notification of message arrival on the server. 
             *
             * @param[in] - message - the newly arrived message. 
             * @param[out] - response. If non-null, this message will be sent back to the entity
             * which sent 'message' as a response.
             * @return - boolean. true, if message handled, false on error or otherwise. If the return
             * value is negative, the response message will not be sent even if non-null.
             */
            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);
            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages);

            /**
             * Notification that a message has been successfully sent. ServerMessageHandler
             * generally does not expect a response, but does want the connection to stay open, 
             * so it returns true. (The client generally is the one that closes the connection.)
             *
             * @param[in] - the message that was sent
             * @return - boolean. If true, expect a response, else, close connection.
             */
            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<ServerMessageHandler> ServerMessageHandlerPtr;

} // namespace 

#endif //  GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H
