/**
 * @file watcherdAPIMessageHandler.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef CLIENT_MESSAGE_HANDLER_H
#define CLIENT_MESSAGE_HANDLER_H

#include "messageHandler.h"

namespace watcher 
{
    /// Base class for hierachy of event handlers for messages arriving from the watcher daemon.
    class WatcherdAPIMessageHandler : 
        public MessageHandler
    {
        public:
            WatcherdAPIMessageHandler();
            virtual ~WatcherdAPIMessageHandler(); 

            virtual bool handleMessageArrive(ConnectionPtr, const event::MessagePtr &message);
            virtual bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr> &messages); 

            virtual bool handleMessageSent(const event::MessagePtr &message); 
            virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<WatcherdAPIMessageHandler> WatcherdAPIMessageHandlerPtr;

} // namespace 

#endif //  CLIENT_MESSAGE_HANDLER_H

