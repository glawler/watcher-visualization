/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#include "database.h"
#include "sqliteDatabase.h"

#include "logger.h"

using namespace watcher;

/** Create a new connection to the specified database.
 * @param[in] uri resource name for the database to open.
 */
DatabaseHandle* DatabaseHandle::connect(const std::string& uri)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return new SqliteDatabaseHandle(uri);
}

DatabaseHandle::~DatabaseHandle()
{
    TRACE_ENTER();
    TRACE_EXIT();
}
