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
 * @file seekWatcherMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef SEEK_WATCHER_MESSAGE_H
#define SEEK_WATCHER_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Seek to a particular point in time in the event stream
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class SeekMessage : public Message {
            public:
                /** constat referring to first element in the database */
                static Timestamp const epoch=0;
                /** constant refering to last event in the database, or live playback */
                static Timestamp const eof=-1;

                /** Type representing how to interpet an offset */
                enum whence {
                    start,      //< offset is relative to start of stream
                    cur,        //< offset is relative to current position in stream
                    end         //< offset is relative to end of stream
                };

                Timestamp offset;   //< offset into stream in milliseconds
                whence rel;     //< specifies how offset is interpreted

                SeekMessage(Timestamp offset = 0, whence = start);
                bool operator== (const SeekMessage &rhs) const;

                friend std::ostream& operator<< (std::ostream& o, const SeekMessage& m);

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int version);
                DECLARE_LOGGER();
        };

        inline bool SeekMessage::operator== (const SeekMessage &rhs) const { return offset == rhs.offset && rel == rhs.rel; }

        typedef boost::shared_ptr<SeekMessage> SeekMessagePtr;
    }
}
#endif
