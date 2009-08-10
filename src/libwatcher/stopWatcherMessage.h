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

/** @file stopWatcherMessage.h
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#ifndef STOP_WATCHER_MESSAGE_H
#define STOP_WATCHER_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Stop playback of event stream.
         * @author Michael Elkins <michael.elkins@cobham.com>
         * @date 2009-03-20
         */
        class StopMessage : public Message {
            public:
            StopMessage(); 

            private:
            friend class boost::serialization::access;
            template <typename Archive> void serialize(Archive& ar, const unsigned int version);
            DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<StopMessage> StopMessagePtr;
    }
}
#endif
