/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "logger.h"

#include "sqliteDatabase.h"

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
