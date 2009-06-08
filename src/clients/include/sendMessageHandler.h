/** @file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-30
 */

#include "messageHandler.h"

namespace watcher {
    /** A handler to use in the case where a single message is sent to the server
     * and the connection should be closed after transmission.
     */
    class SendMessageHandler : public MessageHandler {
        public:
            bool handleMessageSent(const event::MessagePtr &) { return true; }

            static MessageHandlerPtr create();
    };

    inline MessageHandlerPtr SendMessageHandler::create() {
        return MessageHandlerPtr(new SendMessageHandler());
    }
}
