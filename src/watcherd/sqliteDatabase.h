#ifndef sqlite_database_h
#define sqlite_database_h

#include "database.h"

#include "logger.h"

namespace watcher
{
    class SqliteDatabaseHandle : public DatabaseHandle
    {
        public:
            SqliteDatabaseHandle(const std::string& path);
            ~SqliteDatabaseHandle();
        private:
            DECLARE_LOGGER();
    };
} //namespace

#endif /* sqlite_database_h */
