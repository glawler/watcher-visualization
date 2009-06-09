/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "logger.h"

#include "sqliteDatabase.h"
#include "singletonConfig.h"
#include "watcherdConfig.h"

using namespace watcher;

/** Create a new connection to the specified database.
 * @param[in] uri resource name for the database to open.
 */
Database* Database::connect(const std::string& uri)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return new SqliteDatabase(uri);
}

Database::~Database()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

/* SQLite can support multiple connections to the same database from different
 * threads/processes.  The way you do this is have a separate DB handle for
 * every thread that is accessing the database.  Use
 * boost::thread::thread_specific_ptr to store a database connection for each
 * thread in the pool.
 */
namespace {
    /// Database handles for each thread in the pool
    boost::thread_specific_ptr<Database> dbh;
}

Database& watcher::get_db_handle()
{
    Database* db = dbh.get(); // Retrive the database handle for this thread.
    if (!db) {
        /* not yet set, create a new connection */

        /* look up the database URI in the global config settings */
        std::string uri;
        SingletonConfig::instance().lookupValue(dbPath, uri);

        db = Database::connect(uri);
        dbh.reset(db);
    }
    return *db;
}

void watcher::store_event(event::MessagePtr m)
{
    get_db_handle().storeEvent(m);
}
