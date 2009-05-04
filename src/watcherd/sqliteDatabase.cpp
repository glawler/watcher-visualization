#include "sqliteDatabase.h"

#include "logger.h"

using namespace watcher;

INIT_LOGGER(SqliteDatabaseHandle, "DatabaseHandle.SqliteDatabaseHandle");

SqliteDatabaseHandle::SqliteDatabaseHandle(const std::string& path)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

SqliteDatabaseHandle::~SqliteDatabaseHandle()
{
    TRACE_ENTER();
    TRACE_EXIT();
}
