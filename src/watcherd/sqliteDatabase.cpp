/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <sstream>

//external deps
#include "sqlite_wrapper.h"
#include "logger.h"

// class declaration
#include "sqliteDatabase.h"

#include "libwatcher/message.h"

using namespace watcher;
using namespace watcher::event;
using namespace sqlite_wrapper;

INIT_LOGGER(SqliteDatabase, "Database.SqliteDatabase");

namespace {
    bool sqlite_init = false;
}

SqliteDatabase::SqliteDatabase(const std::string& path)
{
    TRACE_ENTER();

    if (!sqlite_init) {
        sqlite_init=true;
        /* disable locks on database connections (user is required to manage) */
        sqlite_wrapper::config(SQLITE_CONFIG_MULTITHREAD);
    }

    conn_.reset(new sqlite_wrapper::Connection(path));

    /*
     * Create a prepared statement for inserting events into the DB.  This allows
     * reuse across calls to storeEvent().
     */
    insert_stmt_.reset(new Statement(*conn_, "INSERT INTO events VALUES (?,?,?,?)"));

    TRACE_EXIT();
}

namespace {
    /* Provide a conversion for MessageType */
    Statement& operator<< (Statement& s, MessageType t) { return s << static_cast<int>(t); }
}

void SqliteDatabase::storeEvent(const std::string& addr, MessagePtr msg)
{
    TRACE_ENTER();

    // serialize event
    std::ostringstream os;
    msg->pack(os);

    //LOG_DEBUG("serialized event: " << os.str());

    // bind values to prepared statement
    *insert_stmt_ << msg->timestamp << msg->type << addr << os.str();

    sqlite_wrapper::execute(*insert_stmt_);

    TRACE_EXIT();
}
