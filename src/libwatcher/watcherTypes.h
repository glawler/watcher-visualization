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

/**
 * @file watcherTypes.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_TYPES_HELLO_THERE_H
#define WATCHER_TYPES_HELLO_THERE_H

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>

/** Toplevel namespace for the Watcher project */
namespace watcher 
{
    /** 
     * used to hide the implementation of how to uniquely identify a node. 
     */ 
    typedef boost::asio::ip::address NodeIdentifier;

    /**
     * Give timestamp its own type - it's Unix epoch milliseconds 
     */
    typedef long long int Timestamp;    // in Epoch milliseconds

    const Timestamp Infinity = -1;

    /**
     * Return the current time in milliseconds, using typedef type.
     * @return current time in milliseconds
     */
    Timestamp getCurrentTime();
}

#endif // WATCHER_TYPES_HELLO_THERE_H
