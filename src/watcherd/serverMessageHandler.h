#ifndef GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H
#define GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H

#include "libwatcher/messageHandler.h"

namespace watcher 
{
    /// Base class for hierarchy of classes which implement handlers for events originating from GUI or feeder clients.
    class ServerMessageHandler : public MessageHandler
    {
        public:
            ServerMessageHandler();
            virtual ~ServerMessageHandler(); 

            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);

            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages);

            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<ServerMessageHandler> ServerMessageHandlerPtr;

} // namespace 

#endif //  GOOD_EVENING_ILL_BE_YOUR_SERVER_MESSAGE_HANDLER_THIS_EVENING_WOULD_YOU_CARE_TO_START_WITH_SOME_DRINKS_H
