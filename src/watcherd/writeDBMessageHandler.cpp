/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/thread.hpp>
#include <boost/foreach.hpp>

#include "messageHandler.h"
#include "writeDBMessageHandler.h"
#include "database.h"

namespace
{
    /// Database handles for each thread in the pool
    boost::thread_specific_ptr<watcher::DatabaseHandle> dbh;
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

bool WriteDBMessageHandler::handleMessageArrive(ConnectionPtr conn, const event::MessagePtr& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    /* Retrive the database handle for this thread. */
    DatabaseHandle *db = dbh.get();
    if (!db)
    {
        /* not yet set, create a new connection */
        db = DatabaseHandle::connect(uri_);
        dbh.reset(db);
    }

    //FIXME db insertion goes here when defined

    TRACE_EXIT_RET(ret);
    return ret;
}

bool WriteDBMessageHandler::handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr>& msg)
{
    TRACE_ENTER();

    bool ret = false; // keep connection open

    BOOST_FOREACH(MessagePtr m, msg)
    {
        ret |= handleMessageArrive(conn, m);
    }

    TRACE_EXIT_RET(ret);
    return ret;
}
