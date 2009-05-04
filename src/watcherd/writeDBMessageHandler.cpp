/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include "messageHandler.h"
#include "writeDBMessageHandler.h"

namespace watcher
{
    INIT_LOGGER(WriteDBMessageHandler, "MessageHandler.WriteDBMessageHandler");

    bool WriteDBMessageHandler::handleMessageArrive(ConnectionPtr conn, const event::MessagePtr& msg)
    {
        bool ret = false;
        TRACE_ENTER();
        TRACE_EXIT_RET(ret);
        return ret;
    }

    bool WriteDBMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr>& msg)
    {
        bool ret = false;
        TRACE_ENTER();
        TRACE_EXIT_RET(ret);
        return ret;
    }

} //namespace
