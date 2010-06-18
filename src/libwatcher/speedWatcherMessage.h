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
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef SPEED_WATCHER_MESSAGE_H
#define SPEED_WATCHER_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Set playback speed of event stream.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class SpeedMessage : public Message {
            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int version);
                DECLARE_LOGGER();

            public:
                float speed;    //< playback speed.  negative value indicates reverse direction
                SpeedMessage(float speed = 1.0);
                bool operator== (const SpeedMessage& rhs) const { return speed == rhs.speed; }
                friend std::ostream& operator<< (std::ostream& o, const SpeedMessage& rhs);

		virtual std::ostream& toStream(std::ostream&) const;
        };

        typedef boost::shared_ptr<SpeedMessage> SpeedMessagePtr;
    }
}
#endif
