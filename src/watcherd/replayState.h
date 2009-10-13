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

/** @file
 * @author Michael.Elkins@cobham.com
 * @date 2009-05-13
 */

#ifndef replay_state_h
#define replay_state_h

#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "libwatcher/watcherTypes.h" //for Timestamp
#include "declareLogger.h"

// forward decls
namespace boost {
    namespace system {
        class error_code;
    }
}

namespace watcher {

    // forward decls
    class ServerConnection;
    typedef boost::shared_ptr<ServerConnection> ServerConnectionPtr;

    /** Implements replay of events from the database to a specific client
     * connected to watcherd.
     *
     * The ServerConnection will hold a weak_ptr to an instance of this class
     * when the client is a GUI and replaying the event stream from the
     * database.
     *
     * The playback rate, event buffer size and time step may all be
     * reconfigured at runtime.  NOTE: altering the playback speed may
     * not take effect immediately because the pending timer is not
     * rescheduled to take into account the new value.
     */
    class ReplayState : public boost::enable_shared_from_this<ReplayState> {
        public:
            /// invalid argument exception
            struct Bad_arg {};

            /** Create an object for replaying events from the database to a specific client
             * connection of watcherd.
             *
             * @param[in] ptr the server connection to send events
             * @param[in] ts the time at which to begin event playback (milliseconds)
             * @param[in] speed event playback speed
             */
            ReplayState(ServerConnectionPtr ptr, Timestamp ts = 0, float speed = 1.0f);

            /** Start event playback. */
            void run();

            /** Pause event playback.
             *
             * NOTE: the object will be destroyed if no shared_ptr exists.
             */
            ReplayState& pause();
            
            /** Return the current position in the event stream. */
            Timestamp tell() const;

            /** Seek to an absolute position in the event stream.
             * @param[in] t time offset in milliseconds
             * @return reference to object
             */
            ReplayState& seek(Timestamp t);

            /** Adjust the event playback speed.
             *
             * @param[in] f a floating point value representing the speed multiplication factor
             * @return reference to object
             */
            ReplayState& speed(float f);

            /** Return the current playback speed. */
            float speed() const;

            /** Adjust the number of events prefetched from the database.
             *
             * @param[in] n positive integer representing the number of events to prefetch
             * @return reference to object
             */
            ReplayState& buffer_size(unsigned int n);

            /** Adjust the granularity of the timer used to bundle events.
             *
             * When a timer expires, all events that occur in the next time
             * step are bundled up and sent out in bulk.  The smaller the
             * time_step, the more accurate playback will be.
             *
             * @param[in] n positive integer representing the number of
             * milliseconds
             * @return reference to object
             */
            ReplayState& time_step(unsigned int n);

            ~ReplayState();

        private:
            struct impl;
            //impl* impl_;
            boost::scoped_ptr<impl> impl_;
            void timer_handler(const boost::system::error_code& error);
            DECLARE_LOGGER();
    };

}

#endif /* replay_state_h */
