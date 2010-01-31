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

/**
 * @file watcherRegion.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef YOUDONTTOUCHTHEQUEENEVENIFYOUAREMICHELLEOBAMBA_H
#define YOUDONTTOUCHTHEQUEENEVENIFYOUAREMICHELLEOBAMBA_H

#include <ostream>
#include <boost/shared_ptr.hpp>
#include "declareLogger.h"

namespace watcher
{
    /**
     * @class WatcherRegion
     * This class defines a region that exists in the watcher environment.
     * It is meant to be used as a filter to narrow the messages that are sent to an watcherd attached GUI.
     *
     * It is currently just a place holder. At some point it'll hold gps locations or some 
     * way of choosing a specific chunk of space.
     */
    class WatcherRegion
    {
        public: 
            WatcherRegion();

            /**
             * Whatchoo talkin' 'bout Willis?
             */
            virtual ~WatcherRegion();

            /** judge me */
            bool operator==(const WatcherRegion &other) const;

            /** make me equal */
            WatcherRegion &operator=(const WatcherRegion &other);
            /**
             * Write an instance of this class as a human readable stream to the otream given
             * @param out the output stream
             * @return reference to output stream
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * Just calls toStream().
             * @param out the output stream
             * @return reference to output stream
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

            // noop

        private:
            DECLARE_LOGGER();

    }; 

    /**
     * typedef a WatcherRegion shared pointer type
     */
    typedef boost::shared_ptr<WatcherRegion> WatcherRegionPtr;

    /** write a human readable version of the WatcherRegion class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const WatcherRegion &region);
}

#endif //  YOUDONTTOUCHTHEQUEENEVENIFYOUAREMICHELLEOBAMBA_H
