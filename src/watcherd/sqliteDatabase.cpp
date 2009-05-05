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

SqliteDatabase::SqliteDatabase(const std::string& path)
    : conn_(new sqlite_wrapper::Connection(path))
{
    TRACE_ENTER();

    TRACE_EXIT();
}

namespace {
    /* Provide a conversion for MessageType */
    Statement& operator<< (Statement& s, MessageType t) { return s << static_cast<int>(t); }
}

void SqliteDatabase::storeEvent(const std::string& addr, MessagePtr msg)
{
    TRACE_ENTER();

    //FIXME resuse the prepared statement somehow
    Statement st = conn_->prepare("INSERT INTO events VALUES (?,?,?,?)");

    // serialize event
    std::ostringstream os;
    msg->pack(os);

    LOG_DEBUG("serialized event: " << os.str());

    // bind values
    st << msg->timestamp << msg->type << addr << os.str();

    // execute the statement
    for (Row r = st.rows(); r != st.end(); ++r) { }

    TRACE_EXIT();
}
