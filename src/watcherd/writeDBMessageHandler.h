/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef writedb_message_handler_h
#define writedb_message_handler_h

#include <string>

#include "libwatcher/messageHandler.h"

namespace watcher
{
    /** Class implementing the interface to the event database.
     * @author Michael.Elkins@cobham.com
     * @date 2009-05-04
     */
    class WriteDBMessageHandler : public MessageHandler
    {
        public:
            bool handleMessageArrive(ConnectionPtr, const event::MessagePtr&);
            bool handleMessagesArrive(ConnectionPtr, const std::vector<event::MessagePtr>&);

        private:
            DECLARE_LOGGER();
    };

} //namespace

#endif /* writedb_message_handler_h */
