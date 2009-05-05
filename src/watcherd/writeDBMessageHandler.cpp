/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/thread.hpp>
#include <boost/foreach.hpp>

#include "messageHandler.h"
#include "writeDBMessageHandler.h"
#include "database.h"
#include "connection.h"

/* SQLite can support multiple connections to the same database from different
 * threads/processes.  The way you do this is have a separate DB handle for
 * every thread that is accessing the database.  Use
 * boost::thread::thread_specific_ptr to store a database connection for each
 * thread in the pool.
 */
namespace {
    /// Database handles for each thread in the pool
    boost::thread_specific_ptr<watcher::Database> dbh;
}

using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WriteDBMessageHandler, "MessageHandler.WriteDBMessageHandler");

/** Create a message handler which writes the event stream to a database
 * specified by URI.
 * @param[in] uri resource specifying the database to write
 */
WriteDBMessageHandler::WriteDBMessageHandler(const std::string& uri)
    : uri_(uri)
{
}

bool WriteDBMessageHandler::handleMessageArrive(ConnectionPtr conn, const MessagePtr& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    if (isFeederEvent(msg->type)) { // only store feeder events
        Database*db = dbh.get(); // Retrive the database handle for this thread.
        if (!db) {
            /* not yet set, create a new connection */
            db = Database::connect(uri_);
            dbh.reset(db);
        }
        db->storeEvent(conn->getPeerAddr(), msg);
    }

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
