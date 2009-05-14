/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/foreach.hpp>

#include "messageHandler.h"
#include "writeDBMessageHandler.h"
#include "database.h"
#include "connection.h"

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WriteDBMessageHandler, "MessageHandler.WriteDBMessageHandler");

bool WriteDBMessageHandler::handleMessageArrive(ConnectionPtr conn, const MessagePtr& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    assert(isFeederEvent(msg->type)); // only store feeder events
    Database& db = get_db_handle(); // Retrive the database handle for this thread.
    db.storeEvent(conn->getPeerAddr(), msg);

    TRACE_EXIT_RET(ret);
    return ret;
}

bool WriteDBMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<MessagePtr>& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    BOOST_FOREACH(MessagePtr m, msg) {
        ret |= handleMessageArrive(conn, m);
    }

    TRACE_EXIT_RET(ret);
    return ret;
}
