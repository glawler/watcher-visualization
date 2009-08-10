/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

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
