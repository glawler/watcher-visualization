/**@file
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-04-24
 */

#ifndef a_place_to_put_stuff
#define a_place_to_put_stuff

#include <stdexcept>
#include <string>
#include <list>
#include <tr1/memory>
#include <vector>
#include <cstring>
#include <sqlite3.h>

// include the forward decls to ensure consistency
#include "sqlite_wrapper_fwd.h"

namespace sqlite_wrapper {

    // wrapper around sqlite3_config
    void config(int mode);

    class Exception : public std::runtime_error {
        public:
            Exception(const std::string& what) : runtime_error(what) {};
    };

    class Connection {
        public:
            Connection(const std::string&);
            ~Connection();
            void close(bool nothrow = false);
            Statement prepare(const std::string&);

        private:
            friend class Statement;
            friend class Row;
            friend class Column;
            sqlite3 *db_;
            std::list<std::tr1::shared_ptr<sqlite3_stmt> > statements_;
            void error_check(int res) {
                if (res != SQLITE_OK)
                    throw Exception(sqlite3_errmsg(db_));
            }
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
    typedef std::tr1::shared_ptr<Impl> ImplPtr;

    class Column {
        private:
            size_t pos_;
            size_t cols_;
            enum { col_eof };
            unsigned int flags_;
            size_t gcount_;
            ImplPtr impl_;

            Column() : pos_(-1), cols_(0), flags_(col_eof), gcount_(0) {}
            Column(ImplPtr& i)
                : pos_(0), flags_(0), gcount_(0), impl_(i)
            {
                std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                cols_ = sqlite3_column_count(p.get());
            }

            inline bool check_col_type(sqlite3_stmt*p, int pos, int type) {
                return sqlite3_column_type(p, pos) == type;
            }

            inline void error_check(int res) { impl_->conn.error_check(res); }

            friend class Row;

        public:
            size_t gcount() { return gcount_; }

            bool operator!() { return flags_; }
            bool operator*() { return !flags_; }

            //currently only used for comparing Row::end()
            bool operator!=(const Column& rhs) { return flags_ == 0; }

            // move to the next column
            Column& operator++() {
                if (!flags_) {
                    if (++pos_ == cols_)
                        flags_ |= col_eof;
                }
                return *this;
            }

            // move to the previous column
            Column& operator--() {
                if (pos_ > 0) {
                    --pos_;
                    flags_ &= ~col_eof;
                }
                return *this;
            }

            // Return a reference to a specific column
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

            /** read the next column as an INTEGER */
            Column& operator>> (int& i) {
                if (!flags_) {
                    std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                    i = sqlite3_column_int(p.get(), pos_);
                    return ++*this;
                }
                return *this;
            }

            /** read the next column as a DOUBLE */
            Column& operator>> (double& d)
            {
                if (!flags_) {
                    std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                    d = sqlite3_column_double(p.get(), pos_++);
                    return ++*this;
                }
                return *this;
            }

            /** read the next column as a BLOB */
            template <typename T> Column& operator>> (std::vector<T>& v) {
                if (!flags_) {
                    std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                    const size_t valsize = sizeof(T);
                    const void *vp = sqlite3_column_blob(p.get(), pos_);
                    gcount_ = sqlite3_column_bytes(p.get(), pos_);
                    v.resize( (gcount_ + valsize - 1) / valsize);
                    memcpy(&v[0], vp, gcount_);
                    return ++*this;
                }
                return *this;
            }

            /** read the next column as a TEXT */
            Column& operator>> (std::string& s)
            {
                if (!flags_)
                {
                    std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                    s = reinterpret_cast<const char*>(sqlite3_column_text(p.get(), pos_));
                    gcount_ = sqlite3_column_bytes(p.get(), pos_);
                    return ++*this;
                }
                return *this;
            }

            /** Read part of a BLOB into an array.
             * @param val array of some type T
             * @param[in] nelems max number of elements to copy
             * @return reference to the Column object
             */
            template <typename T, int N> Column& get(T (&val)[N], size_t nelems = N) { return get(&val[0], N); }

            /** Read part of a BLOB into an array.
             * @param val array of some type T
             * @param[in] nelems max number of elements to copy
             * @return reference to the Column object
             */
            template <typename T> Column& get(T* val, size_t nelems) {
                if (!flags_) {
                    std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
                    const size_t valsize = sizeof(T) * nelems;
                    const void *vp = sqlite3_column_blob(p.get(), pos_);
                    const size_t blobsize = sqlite3_column_bytes(p.get(), pos_);
                    gcount_ = std::min(blobsize, valsize);
                    memcpy(val, vp, gcount_);
                    return ++*this;
                }
                return *this;
            }

            template <typename T, int N> Column& operator>> (T (&val)[N]) { return get(&val[0], N); }

            void reset() {
                pos_ = 0;
                gcount_ = 0;
                flags_ &= ~col_eof;
            }
    };

    /** Loosely modeled on istream */
    class Row {
        private:
            enum { row_eof=1, row_fail=2 };
            unsigned int flags_;
            ImplPtr impl_;

            Row() : flags_(-1) {}
            Row(ImplPtr p) : flags_(0), impl_(p) { step(); }

            void step();

            inline void error_check(int res) { impl_->conn.error_check(res); }

            friend class Statement;

        public:
            // for checking validity ala iostreams
            bool operator*() const { return !flags_; }
            bool operator! () const { return flags_; }
            //currently only used for comparing Statement::end()
            bool operator!=(const Row& rhs) { return (flags_ & row_eof) == 0; }
            // deref operator
            Column operator*() { return Column(impl_); }
            //pre-increment, get next row
            inline Row& operator++()
            {
                step();
                return *this;
            }

            // iterator-like support for fetching column data
            Column columns() { return Column(impl_); }
            Column end() { return Column(); }
    };

    /* C++ to C conversion functions for use with templates. */
    int sqlite_binder(sqlite3_stmt*, int pos, int val);
    int sqlite_binder(sqlite3_stmt*, int pos, int64_t val);
    int sqlite_binder(sqlite3_stmt*, int pos, double val);
    int sqlite_binder(sqlite3_stmt*, int pos, const std::string&);
    int sqlite_binder(sqlite3_stmt*, int pos, const char* val, size_t len, void (*)(void*) );
    int sqlite_binder(sqlite3_stmt*, int pos, const void* val, size_t len, void (*)(void*) );

    extern void (*sqlite_static)(void *);

    class Statement {
        private:
            friend class Connection;

            int pos_;
            ImplPtr impl_;

            inline void error_check(int res) {
                if (res != SQLITE_OK)
                    throw sqlite_wrapper::Exception(sqlite3_errmsg(impl_->conn.db_));
            }

            Statement(ImplPtr i) : pos_(1), impl_(i) {}

        public:
            ~Statement();

            template <typename T> Statement& bind(const T& val);
            template <typename T> Statement& bind(const T* val, size_t len);
            template <typename T, int N> Statement& bind(const T (&val)[N] ) { return bind(val, N); }

            // binding operators
            template <typename T> Statement& operator<< (const T& val) { return bind(val); }
            template <typename T, int N> Statement& operator<< (const T (&val) [N]) { return bind(val, N); }

            // Iterator-like functions for fetching rows
            Row rows() { return Row(impl_); }
            Row end() { return Row(); }

            Statement& reset();

            size_t count() { return impl_->nrows; }
    };

    template <typename T> Statement& Statement::bind(const T& val)
    {
        std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        error_check(sqlite_binder(p.get(), pos_++, val));
        return *this;
    }

    template <typename T> Statement& Statement::bind(const T* val, size_t len)
    {
        std::tr1::shared_ptr<sqlite3_stmt> p = impl_->stmt.lock();
        error_check(sqlite_binder(p.get(), pos_++, val, len * sizeof(T)));
        return *this;
    }
}

#endif
