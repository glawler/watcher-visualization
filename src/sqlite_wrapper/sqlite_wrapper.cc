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
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-04-24
 */

#include <cassert>
#include <sstream>
#include "sqlite_wrapper.h"
#include "sqlite_thread.h"

using namespace sqlite_wrapper;
using namespace std;
using namespace boost;

// void sqlite_wrapper::config(int mode)
//{
//    int res = sqlite3_config(mode);
//    if (res != SQLITE_OK)
//        throw Exception("sqlite3_config error");
//}

Connection::Connection(const std::string& path, flags f) : db_(0)
{
    int res;
    if (f) {
        int sqflags = 0;
        if (f & readonly) sqflags |= SQLITE_OPEN_READONLY;
        if (f & readwrite) sqflags |= SQLITE_OPEN_READWRITE;
        if (f & create) sqflags |= SQLITE_OPEN_CREATE;
        if (f & nomutex) sqflags |= SQLITE_OPEN_NOMUTEX;
        if (f & fullmutex) sqflags |= SQLITE_OPEN_FULLMUTEX;
        res = sqlite3_open_v2(path.c_str(), &db_, sqflags, 0);
    } else
        res = sqlite3_open(path.c_str(), &db_);
    error_check(res);
    
    /*
     * Install a callback that is invoked when sqlite detects that database
     * access would be blocked.  This will make blocked threads wait on pthread_cond_wait()
     * to be awoken when the currently executing thread is done.
     */
    sqlite3_busy_handler(db_, busy_handler, 0);
}

void Connection::close(bool nothrow)
{
    if (db_) {
        // explicitly clear the prepared statements to force the deleters on the
        // shared pointers to execute
        statements_.clear();

        int res = sqlite3_close(db_);
        if (!nothrow)
            error_check(res);
        db_ = 0;
    }
}

ImplPtr Connection::prepare(const std::string& s)
{
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v2(db_, s.c_str(), -1, &stmt, 0);
    error_check(res);
    statements_.push_front(shared_ptr<sqlite3_stmt>(stmt, sqlite3_finalize) );
    return ImplPtr( new Impl(*this, weak_ptr<sqlite3_stmt>( statements_.front() )));
}

Statement::Statement(Connection& conn, const std::string& s)
    : pos_(1), impl_(conn.prepare(s))
{
}

/** Execute a prepared statement which has been bound with parameters and ignore
 * any returned rows.  Useful for SQL commands which return no data (eg. INSERT).
 *
 * This is a non-member function because it only utilizes the public interface of
 * class Statement.
 */
void sqlite_wrapper::execute(Statement& s)
{
    Row r = s.rows();
    while (*r) ++r;
    s.reset();
}

/** Execute SQL statement(s).
 * @param[in] s UTF-8 encoded SQL statements to execute.
 * */
void Connection::execute(const std::string& s)
{
    char *err = 0;
    int res = sqlite3_exec(db_, s.c_str(), 0, 0, &err);

    busy_done(); //wake up waiting threads

    if (err) {
        std::string errstr(err);
        sqlite3_free(err);
        throw Exception(errstr);
    }

    error_check(res);
}

Statement& Statement::reset()
{
    shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
    error_check(sqlite3_reset(p.get()));
    pos_ = 1;
    impl_->nrows = 0;
    return *this;
}

Statement::~Statement()
{
    try {
        // will throw bad_weak_ptr if the db connection has gone away
        shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        impl_->conn.statements_.remove(p);
    } catch (bad_weak_ptr) {
        //ignore
    }
}

void Statement::error_check(int res)
{
    if (res != SQLITE_OK)
        throw sqlite_wrapper::Exception(sqlite3_errmsg(impl_->conn.db_));
}

void Row::step()
{
    if (flags_ & row_fail)
        throw Exception("object is invalid");

    shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();

    int res = sqlite3_step(p.get());

    busy_done(); // wake up any waiting threads

    switch(res) {
        case SQLITE_ROW:
            ++impl_->nrows;
            flags_ &= ~row_eof;
            break;

        case SQLITE_DONE:
            flags_ |= row_eof;

            break;

        case SQLITE_ERROR:
            {
                flags_ |= row_fail;
                throw Exception( sqlite3_errmsg(impl_->conn.db_) );
            } break;

        case SQLITE_CORRUPT:
            { 
                flags_ |= row_fail;
                throw Exception("database is corrupt"); 
            } 
            break;

        default:
            {
                flags_ |= row_fail;
                std::ostringstream err_mess;
                err_mess << "Unhandled ret code from sqlite3_step: " << res;
                throw Exception(err_mess.str());
            } break;
    }
}

Connection::~Connection()
{
    // can't throw on error
    Connection::close(true);
}


/** read the next column as a TEXT */
Column& Column::operator>> (std::string& s)
{
    if (!flags_)
    {
        boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        s = reinterpret_cast<const char*>(sqlite3_column_text(p.get(), pos_));
        gcount_ = sqlite3_column_bytes(p.get(), pos_);
        return ++*this;
    }
    return *this;
}

/** read the next column as an INTEGER */
Column& Column::operator>> (int& i)
{
    if (!flags_) {
        boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        i = sqlite3_column_int(p.get(), pos_);
        return ++*this;
    }
    return *this;
}

/** read the next column as an INTEGER */
Column& Column::operator>> (long long int& i)
{
    if (!flags_) {
        boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        i = sqlite3_column_int64(p.get(), pos_);
        return ++*this;
    }
    return *this;
}

/** read the next column as a DOUBLE */
Column& Column::operator>> (double& d)
{
    if (!flags_) {
        boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        d = sqlite3_column_double(p.get(), pos_++);
        return ++*this;
    }
    return *this;
}

int sqlite_wrapper::sqlite_binder(sqlite3_stmt*s, int pos, int val)
{
    return sqlite3_bind_int(s, pos, val);
}

int sqlite_wrapper::sqlite_binder(sqlite3_stmt*s, int pos, long long val)
{
    return sqlite3_bind_int64(s, pos, val);
}

int sqlite_wrapper::sqlite_binder(sqlite3_stmt*s, int pos, double val)
{
    return sqlite3_bind_double(s, pos, val);
}

/*
 * Note: use of SQLITE_TRANSIENT in the next few calls is not optimal since it
 * makes a copy.  However, we can't make assumptions here about the scope of
 * the data passed in from the caller.
 */

int sqlite_wrapper::sqlite_binder(sqlite3_stmt*s, int pos, const char* val, size_t len)
{
    return sqlite3_bind_text(s, pos, val, len, SQLITE_TRANSIENT);
}

int sqlite_wrapper::sqlite_binder(sqlite3_stmt*s, int pos, const void* val, size_t len)
{
    return sqlite3_bind_blob(s, pos, val, len, SQLITE_TRANSIENT);
}

int sqlite_wrapper::sqlite_binder(sqlite3_stmt* s, int pos, const std::string& st)
{
    return sqlite3_bind_text(s, pos, st.c_str(), st.size(), SQLITE_TRANSIENT);
}
