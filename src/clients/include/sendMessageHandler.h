/** @file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-30
 */

#include "libwatcher/messageHandler.h"

namespace watcher {
    /** A handler to use in the case where a single message is sent to the server
     * and the connection should be closed after transmission.
     */
    class SendMessageHandler : public MessageHandler {
        public:
            /** Callback to cause the connection to terminate after sending a single
             * message to the server.
             * @param message the message that was sent
             * @retval true always returns true
             */
            bool handleMessageSent(const event::MessagePtr &message) { return true; }

            static MessageHandlerPtr create();
    };

    inline MessageHandlerPtr SendMessageHandler::create() {
        return MessageHandlerPtr(new SendMessageHandler());
    }
}
