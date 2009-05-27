/**@file
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-04-24
 */

#ifndef a_place_to_put_stuff
#define a_place_to_put_stuff

#include <stdexcept>
#include <string>
#include <list>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/utility.hpp>
#include <vector>
#include <cstring>

// include the forward decls to ensure consistency
#include "sqlite_wrapper_fwd.h"

#include <sqlite3.h>

namespace sqlite_wrapper {

    // wrapper around sqlite3_config
    void config(int mode);

    class Exception : public std::runtime_error {
        public:
            Exception(const std::string& what) : runtime_error(what) {};
    };

    /* Common data structure used by class Row and Column.  Used to avoid
     * issue with both classes requiring eachother to be mutually defined.
     */
    struct Impl {
        Connection& conn;
        StatementPtr stmt;
        size_t nrows; // number of rows fetched
        Impl(Connection& c, StatementPtr s) : conn(c), stmt(s), nrows(0) {}
    };
    typedef boost::shared_ptr<Impl> ImplPtr;

    /** Modes for opening the database.
     * This is modeled on iostate.
     */
    enum ConnectionFlags {
        cf_none=0, cf_readwrite=(1<<0), cf_nomutex=(1<<1), cf_fullmutex=(1<<2), cf_readonly=(1<<3), cf_create=(1<<4)
    };
    /** Define a typesafe bitwise-OR operator for flags */
    inline ConnectionFlags operator| (ConnectionFlags a, ConnectionFlags b) {
        /* The casts here are required to avoid an infinite recursion.
         * Yes, I found this out the hard way.  */
        return ConnectionFlags(static_cast<int>(a) | static_cast<int>(b));
    }

    /** A class representing a connection to a specific database. */
    class Connection : private boost::noncopyable {
        public:
            typedef ConnectionFlags flags;
            static const flags none = cf_none;
            static const flags readwrite = cf_readwrite;
            static const flags nomutex = cf_nomutex;
            static const flags fullmutex = cf_fullmutex;
            static const flags readonly = cf_readonly;
            static const flags create = cf_create;

            Connection(const std::string&, flags f = none);
            ~Connection();
            void close(bool nothrow = false);
            void execute(const std::string&);

        private:
            friend class Statement;
            friend class Row;
            friend class Column;
            sqlite3 *db_;
            std::list<boost::shared_ptr<sqlite3_stmt> > statements_;
            void error_check(int res) {
                if (res != SQLITE_OK)
                    throw Exception(sqlite3_errmsg(db_));
            }
            ImplPtr prepare(const std::string&);
    };

    /** A class for iterating over column values in a Row. */
    class Column {
        private:
            size_t pos_;
            size_t cols_;
            enum { col_eof };
            unsigned int flags_;
            size_t gcount_;
            ImplPtr impl_;

            Column(ImplPtr& i)
                : pos_(0), flags_(0), gcount_(0), impl_(i)
            {
                boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                cols_ = sqlite3_column_count(p.get());
            }

            inline bool check_col_type(sqlite3_stmt*p, int pos, int type) {
                return sqlite3_column_type(p, pos) == type;
            }

            inline void error_check(int res) { impl_->conn.error_check(res); }

            friend class Row;

        public:
            /** Returns the number of bytes read by the last get<T>() or operator>>() call */
            size_t gcount() { return gcount_; }

            /** Validity check. */
            bool operator*() { return !flags_; }

            /** Invalidity check. */
            bool operator!() { return flags_; }

            /** move to the next column */
            Column& operator++() {
                if (!flags_) {
                    if (++pos_ == cols_)
                        flags_ |= col_eof;
                }
                return *this;
            }

            /** move to the previous column */
            Column& operator--() {
                if (pos_ > 0) {
                    --pos_;
                    flags_ &= ~col_eof;
                }
                return *this;
            }

            /** Return a reference to a specific column */
            Column& operator[] (size_t j) {
                if (j < cols_) {
                    pos_ = j;
                    flags_ &= ~col_eof;
                }
                return *this;
            }

            /** manipulator to seek to a specific column. */
            struct setcol {
                size_t pos;
                explicit setcol(int n) : pos(n) {};
            };

            /** Seek to a specific column. */
            Column& operator>> (const setcol& c) { return this[c.pos]; }

            Column& operator>> (int& i);
            Column& operator>> (int64_t& i);
            Column& operator>> (double& d);
            Column& operator>> (std::string& s);
            template <typename T> Column& operator>> (std::vector<T>& v);

            /** Read part of a BLOB into an array.
             * @param val array of some type T
             * @param[in] nelems max number of elements to copy
             * @return reference to the Column object
             */
            template <typename T, int N> Column& get(T (&val)[N], size_t nelems = N) { return get(&val[0], N); }

            template <typename T> Column& get(T* val, size_t nelems);

            template <typename T, int N> Column& operator>> (T (&val)[N]) { return get(&val[0], N); }

            void reset() {
                pos_ = gcount_ = 0;
                flags_ &= ~col_eof;
            }
    };

    /** read the next column as a BLOB */
    template <typename T> inline Column& Column::operator>> (std::vector<T>& v) {
        if (!flags_) {
            boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
            const size_t valsize = sizeof(T);
            const void *vp = sqlite3_column_blob(p.get(), pos_);
            gcount_ = sqlite3_column_bytes(p.get(), pos_);
            v.resize( (gcount_ + valsize - 1) / valsize);
            memcpy(&v[0], vp, gcount_);
            return ++*this;
        }
        return *this;
    }

    /** Read part of a BLOB into an array.
     * @param val array of some type T
     * @param[in] nelems max number of elements to copy
     * @return reference to the Column object
     */
    template <typename T> inline Column& Column::get(T* val, size_t nelems) {
        if (!flags_) {
            boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
            const size_t valsize = sizeof(T) * nelems;
            const void *vp = sqlite3_column_blob(p.get(), pos_);
            const size_t blobsize = sqlite3_column_bytes(p.get(), pos_);
            gcount_ = std::min(blobsize, valsize);
            memcpy(val, vp, gcount_);
            return ++*this;
        }
        return *this;
    }

    /** Loosely modeled on istream */
    class Row {
        private:
            enum { row_eof=1, row_fail=2 };
            unsigned int flags_;
            ImplPtr impl_;

            Row(ImplPtr p) : flags_(0), impl_(p) { step(); }

            void step();

            void error_check(int res) { impl_->conn.error_check(res); }

            friend class Statement;

        public:
            //< for checking validity ala iostreams
            bool operator* () const { return !flags_; }
            //< for checking validity ala iostreams
            bool operator! () const { return flags_; }

            operator bool () const { return !flags_; }

            //pre-increment, get next row
            Row& operator++();

            // iterator-like support for fetching column data
            Column columns() { return Column(impl_); }
    };

    inline Row& Row::operator++() {
        step();
        return *this;
    }

    /* C++ to C conversion functions for use with templates. */
    int sqlite_binder(sqlite3_stmt*, int pos, int val);
    int sqlite_binder(sqlite3_stmt*, int pos, int64_t val);
    int sqlite_binder(sqlite3_stmt*, int pos, double val);
    int sqlite_binder(sqlite3_stmt*, int pos, const std::string&);
    int sqlite_binder(sqlite3_stmt*, int pos, const char* val, size_t len);
    int sqlite_binder(sqlite3_stmt*, int pos, const void* val, size_t len);

    /** A class representing a SQL prepared statment.  Every statement is
     * associated with a specific Connection object.  When the Connection
     * object is destroyed, the prepared statement becomes invalid.  An
     * attempt to use a Statement after its associated Connection has been
     * deleted will cause a boost::bad_weak_ptr exception.
     */
    class Statement : private boost::noncopyable {
        private:
            friend class Connection;

            int pos_;
            ImplPtr impl_;

            void error_check(int res);

        public:
            Statement(Connection&, const std::string&);
            ~Statement();

            template <typename T> Statement& bind(const T* val, size_t len);
            template <typename T, int N> Statement& bind(const T (&val)[N] ) { return bind(val, N); }

            // binding operators
            template <typename T> Statement& operator<< (const T& val);
            template <typename T, int N> Statement& operator<< (const T (&val) [N]) { return bind(val, N); }

            // Iterator-like functions for fetching rows
            Row rows() { return Row(impl_); }

            Statement& reset();

            /** Return the number of rows fetched by the last execution of this
             * statement.
             */
            size_t count() { return impl_->nrows; }
    };

    template <typename T> inline Statement& Statement::operator<< (const T& val) {
        boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        error_check(sqlite_binder(p.get(), pos_++, val));
        return *this;
    }

    template <typename T> inline Statement& Statement::bind(const T* val, size_t len) {
        boost::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        error_check(sqlite_binder(p.get(), pos_++, val, len * sizeof(T)));
        return *this;
    }

    void execute(Statement& s);

} //namespace

#endif
