/* Copyright 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

#ifndef watcher_message_fwd_h
#define watcher_message_fwd_h

// forward decls from libwatchermsg

#include <boost/shared_ptr.hpp>

namespace watcher {

namespace event {
    class Message;
    class SeekMessage;
    class SpeedMessage;
    class PlaybackTimeRangeMessage;
    class StreamDescriptionMessage;

    typedef boost::shared_ptr<Message> MessagePtr;
    typedef boost::shared_ptr<SeekMessage> SeekMessagePtr;
    typedef boost::shared_ptr<SpeedMessage> SpeedMessagePtr;
    typedef boost::shared_ptr<PlaybackTimeRangeMessage> PlaybackTimeRangeMessagePtr;
    typedef boost::shared_ptr<StreamDescriptionMessage> StreamDescriptionMessagePtr;
} // namespace

} // namespace

#endif
