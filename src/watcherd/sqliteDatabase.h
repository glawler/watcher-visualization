/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef sqlite_database_h
#define sqlite_database_h

#include "database.h"
#include "logger.h"
#include "sqlite_wrapper_fwd.h"

namespace watcher {
    /** Implementation of a SQLite backend for the Event database abstraction. */
    class SqliteDatabase : public Database {
        public:
            /** Create a new database handle backed by the specified file.
             * @param[in] path pathname for the sqlite database
             */
            SqliteDatabase(const std::string& path);

            void storeEvent(event::MessagePtr msg);
            void getEvents( boost::function<void(event::MessagePtr)> output, Timestamp t, Direction d, unsigned int count );
            TimeRange eventRange();

        private:
            /** Pointer to the sqlite implementation backing this connection. */
            boost::scoped_ptr<sqlite_wrapper::Connection> conn_;

            /** Prepared statement for resuse for storing events in storeEvent().
             * This allows * reuse across calls to storeEvent().
             */
            boost::scoped_ptr<sqlite_wrapper::Statement> insert_stmt_;

            DECLARE_LOGGER();
    };
} //namespace

#endif /* sqlite_database_h */
