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
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-04
 */

#ifndef database_h
#define database_h

#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <string>

#include "libwatcher/message_fwd.h"
#include "libwatcher/watcherTypes.h"

namespace watcher {
    typedef std::pair<Timestamp, Timestamp> TimeRange;

    /** Abstract class used to provide an interface to a database backend for
     * storing event streams. */
    class Database : private boost::noncopyable {
        public:
            static Database* connect(const std::string&);

            /** Store an event received from a specified host into the database.
             *
             * @param[in] msg the Event to store
             */
            virtual void storeEvent(event::MessagePtr msg) = 0;


            enum Direction { forward, reverse };

            /** Retreive events from the database.
             * @param output a function which accepts the individual events returned from the DB
             * @param[in] t time offset at which to start retrieving events
             * @param[in] d direction of the event stream
             * @param[in] count the max number of events to retrieve
             */
            virtual void getEvents( boost::function<void(event::MessagePtr)> output, Timestamp t, Direction d, unsigned int count ) = 0;

            virtual TimeRange eventRange() = 0;

            virtual ~Database() = 0;
    };

    /** Get the thread-local database handle for the current thread.  If one
     * does not exist, it is created.  The URI for the database is found from
     * the SingletonConfig instance for the process.
     *
     * @return handle to thread-local database handle.
     */
    Database& get_db_handle();

    /** Put an event into the database. */
    void store_event(event::MessagePtr);

    
    /** Return the Timestamps for the first and last event in the database. */
    TimeRange event_range();

} //namespace

#endif /* database_h */
