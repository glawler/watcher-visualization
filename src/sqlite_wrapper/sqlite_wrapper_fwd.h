/**@file
 * This file contains forward declarations for the basic classes in the
 * sqlite_wrapper namespace.  The intent is to allow users of this library to
 * include this file in lieu of sqlite_wrapper.h in header files where a
 * pointer or reference is all that is required.
 * @author Michael Elkins <Michael.Elkins@cobham.com>
 * @date 2009-05-05
 */

#ifndef sqlite_wrapper_fwd_h
#define sqlite_wrapper_fwd_h

struct sqlite3_stmt;

#ifndef HAVE_BOOST
namespace boost = tr1;
#endif

namespace sqlite_wrapper {

    class Statement;

    typedef boost::weak_ptr<sqlite3_stmt> StatementPtr;

    class Connection;
} //namespace

#endif /* sqlite_wrapper_fwd_h */
