/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

SqliteDatabase::SqliteDatabase(const std::string& path) :
    conn_(new Connection(path, Connection::readwrite | Connection::create | Connection::nomutex))
{
    TRACE_ENTER();

    /* Create database if it doesn't yet exist */
    conn_->execute("CREATE TABLE IF NOT EXISTS events ( ts INTEGER, evtype INTEGER, node TEXT, data TEXT ); "
                   "CREATE INDEX IF NOT EXISTS time ON events ( ts ASC );");

    /* Added to enable higher insert rate.
     * More concerned with performance than crash integrity
     */
    conn_->execute("PRAGMA synchronous = OFF;");
    conn_->execute("PRAGMA journal_mode = OFF;");

    /*
     * This must come after the db creation otherwise it will fail saying that
     * table "events" does not exist.
     */
    insert_stmt_.reset(new Statement(*conn_, "INSERT INTO events VALUES (?,?,?,?)"));

    TRACE_EXIT();
}

namespace {
    /* Provide a conversion for MessageType */
    Statement& operator<< (Statement& s, MessageType t) { return s << static_cast<int>(t); }
}

void SqliteDatabase::storeEvent(MessagePtr msg)
{
    TRACE_ENTER();

    // serialize event
    std::ostringstream os;
    msg->pack(os);

    //LOG_DEBUG("serialized event: " << os.str());

    // bind values to prepared statement
    *insert_stmt_ << msg->timestamp << msg->type << msg->fromNodeID.to_string() << os.str();

    sqlite_wrapper::execute(*insert_stmt_);

    TRACE_EXIT();
}

void SqliteDatabase::getEvents(boost::function<void(event::MessagePtr)> output,
                               Timestamp t, Direction d, unsigned int count)
{
    TRACE_ENTER();

    /* the count of how many events we've processed thus far for this query */
    unsigned int nevents = 0;

    std::ostringstream os;
    os << "SELECT data FROM events WHERE ts" << (d == forward ? ">" : "<") << t <<
	" ORDER BY ts " << (d == forward ? "ASC" : "DESC");
    LOG_DEBUG(os.str());

    // read each serialized event from a row, unpack and pass to callback function
    Timestamp last_event = 0;
    Statement st(*conn_, os.str());
    for (Row r(st.rows()); r; ++r, ++nevents) {
	Column c(r.columns());
	std::string data;
	c >> data;
	LOG_DEBUG("attempting to deserialize data from db: " << data);
	std::istringstream is(data);
	event::MessagePtr msg(Message::unpack(is));

	/* In the case where more than `count` events occurred during the same
	 * millisecond, make sure all events are read, even if there are more than
	 * the user requested.
	 */
	if (msg->timestamp > last_event && nevents >= count) {
	    LOG_DEBUG("stopping at ts " << msg->timestamp << " after reading " << nevents << " events");
	    break;
	}

	output(msg);
	last_event = msg->timestamp;
    }

    TRACE_EXIT();
}

TimeRange SqliteDatabase::eventRange()
{
    Timestamp begin = 0, end = 0;

    TRACE_ENTER();

    Statement s(*conn_,
                "SELECT * FROM"
                "(SELECT ts from events ORDER BY ts ASC LIMIT 1)"
                "JOIN"
                "(SELECT ts from events ORDER BY ts DESC LIMIT 1)");
    Row r(s.rows());
    Column c(r.columns());
    c >> begin >> end;

    LOG_DEBUG("begin=" << begin << " end=" << end);

    TRACE_EXIT();

    return TimeRange(begin, end);
}

// vim:sw=4 ts=8
