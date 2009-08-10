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
 * @file playbackTimeRange.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
/** @file
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-06-24
 */
#ifndef PLAYBACK_TIME_RANGE_H
#define PLAYBACK_TIME_RANGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Get time range of events in the playback database
         * @author Michael Elkins <michael.elkins@cobham.com>
         * @date 2009-06-24
         */
        class PlaybackTimeRangeMessage : public Message {
            public:
                PlaybackTimeRangeMessage(); 

                Timestamp min_;
                Timestamp max_;
            private:
                template <typename Archive> void serialize(Archive & ar, const unsigned int version);
                friend class boost::serialization::access;
                DECLARE_LOGGER();
        };

        std::ostream& operator<< (std::ostream&, const PlaybackTimeRangeMessage&);

        typedef boost::shared_ptr<PlaybackTimeRangeMessage> PlaybackTimeRangeMessagePtr;

        bool operator== (const PlaybackTimeRangeMessage& lhs, const PlaybackTimeRangeMessage& rhs);
    }
}
#endif
