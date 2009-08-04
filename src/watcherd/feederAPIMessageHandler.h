#ifndef CLIENT_MESSAGE_HANDLER_H
#define CLIENT_MESSAGE_HANDLER_H

#include "messageHandler.h"

namespace watcher 
{
    /// base class for hierarchy of event handlers for message sent via feeder daemons
    class FeederAPIMessageHandler : 
        public MessageHandler
    {
        public:
            FeederAPIMessageHandler();
            virtual ~FeederAPIMessageHandler(); 

            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);
            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages); 

            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<FeederAPIMessageHandler> FeederAPIMessageHandlerPtr;

} // namespace 

#endif //  CLIENT_MESSAGE_HANDLER_H

