/**@file
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-04-24
 */

#include "sqlite_wrapper.h"
#include "sqlite_thread.h"
#include <cassert>
#include <tr1/functional>

namespace sqlite_wrapper {

using namespace std;
using namespace std::tr1;

void(*sqlite_static)(void *) = reinterpret_cast<void(*)(void*)>(SQLITE_STATIC);

void config(int mode)
{
    int res = sqlite3_config(mode);
    if (res != SQLITE_OK)
        throw Exception("sqlite3_config error");
}

Connection::Connection(const std::string& path) : db_(0)
{
    int res = sqlite3_open(path.c_str(), &db_);
    error_check(res);
    sqlite3_busy_handler(db_, busy_handler, 0);
}

void Connection::close(bool nothrow)
{
    if (db_)
    {
        // explicitly clear the prepared statements to force the deleters on the
        // shared pointers to execute
        statements_.clear();

        int res = sqlite3_close(db_);
        if (!nothrow)
            error_check(res);
        db_ = 0;
    }
}

Statement Connection::prepare(const std::string& s)
{
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v2(db_, s.c_str(), -1, &stmt, 0);
    error_check(res);
    statements_.push_front( shared_ptr<sqlite3_stmt>(stmt, sqlite3_finalize) );
    return Statement( ImplPtr( new Impl(*this, weak_ptr<sqlite3_stmt>( statements_.front() ))));
}

Statement& Statement::reset()
{
    std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
    error_check(sqlite3_reset(p.get()));
    pos_ = 1;
    impl_->nrows = 0;
    return *this;
}

Statement::~Statement()
{
    try
    {
        // will throw bad_weak_ptr if the db connection has gone away
        shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        impl_->conn.statements_.remove(p);
    }
    catch (bad_weak_ptr)
    {
        //ignore
    }
}

void Row::step()
{
    if (flags_ & row_fail)
        throw Exception("object is invalid");

    shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();

    int res = sqlite3_step(p.get());

    switch(res)
    {
        case SQLITE_ROW:
            ++impl_->nrows;
            flags_ &= ~row_eof;
            break;

        case SQLITE_DONE:
            error_check(sqlite3_reset(p.get()));
            flags_ |= row_eof;

            // wake up any waiting threads
            busy_done();
            break;

        case SQLITE_ERROR:
            {
                flags_ |= row_fail;
                throw Exception( sqlite3_errmsg(impl_->conn.db_) );
            } break;

        default:
            {
                flags_ |= row_fail;
                throw Exception("unhandled return code from sqlite3_step():");
            } break;
    }
}

Connection::~Connection()
{
        // can't throw on error
    Connection::close(true);
}

int sqlite_binder(sqlite3_stmt*s, int pos, int val)
{
    return sqlite3_bind_int(s, pos, val);
}

int sqlite_binder(sqlite3_stmt*s, int pos, double val)
{
    return sqlite3_bind_double(s, pos, val);
}

int sqlite_binder(sqlite3_stmt*s, int pos, const char* val, size_t len, void (*)(void*) )
{
    return sqlite3_bind_text(s, pos, val, len, sqlite_static);
}

int sqlite_binder(sqlite3_stmt*s, int pos, const void* val, size_t len, void (*)(void*) )
{
    return sqlite3_bind_blob(s, pos, val, len, sqlite_static);
}

int sqlite_binder(sqlite3_stmt* s, int pos, const std::string& st)
{
    return sqlite_binder(s, pos, st.c_str(), st.size(), sqlite_static);
}

}
