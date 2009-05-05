/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef sqlite_database_h
#define sqlite_database_h

#include "database.h"

#include "logger.h"

// forward decl
namespace sqlite_wrapper
{
    class Connection;
}

namespace watcher
{
    /** Implementation of a SQLite backend for the Event database abstraction. */
    class SqliteDatabase : public Database
    {
        public:
            /** Create a new database handle backed by the specified file.
             * @param[in] path pathname for the sqlite database
             */
            SqliteDatabase(const std::string& path);

            void storeEvent(const std::string& addr, event::MessagePtr msg);

        private:
            boost::scoped_ptr<sqlite_wrapper::Connection> conn_;

            DECLARE_LOGGER();
    };
} //namespace

#endif /* sqlite_database_h */
