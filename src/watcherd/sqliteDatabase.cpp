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

SqliteDatabase::SqliteDatabase(const std::string& path) :
    conn_(new Connection(path, Connection::readwrite | Connection::create | Connection::nomutex))
{
    TRACE_ENTER();

    /* Create database if it doesn't yet exist */
    conn_->execute("CREATE TABLE IF NOT EXISTS events ( ts INTEGER, evtype INTEGER, node TEXT, data TEXT ); "
                   "CREATE INDEX IF NOT EXISTS time ON events ( ts ASC );");

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

void SqliteDatabase::getEvents(boost::function<void(event::MessagePtr)> output,
                               Timestamp t, Direction d, unsigned int count)
{
    TRACE_ENTER();
    std::ostringstream os;
    os << "SELECT data FROM events WHERE ts" << (d == forward ? ">" : "<") << t <<
        " ORDER BY ts " << (d == forward ? "ASC" : "DESC") <<
        " LIMIT " << count;
    LOG_DEBUG(os.str());

    // read each serialized event from a row, unpack and pass to callback function
    Statement st(*conn_, os.str());
    for (Row r(st.rows()); r; ++r) {
        Column c(r.columns());
        std::string data;
        c >> data;
        LOG_DEBUG("attempting to deserialize data from db: " << data);
        std::istringstream is(data);
        output(Message::unpack(is));
    }
    TRACE_EXIT();
}
