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
 * @file messageStreamFilterMessage.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2010-01-31
 */
#ifndef MESSAGE_STREAM_FILTER_MESSAGE
#define MESSAGE_STREAM_FILTER_MESSAGE

#include <string>
#include "message.h"
#include "messageStreamFilter.h"

namespace watcher {
    namespace event {
        /** 
         * @class MessageStreamFilterMessage
         * This class wraps a message stream filter so it can be sent
         * over the network ,presumably to a watcherd instance. 
         */
        class MessageStreamFilterMessage : public Message {
            public:

                /** The messageStreamFilter to send/recv. */
                MessageStreamFilter theFilter;

                /** If true, thent he filter is applied, else it is removed. */
                bool applyFilter;

                /** Let there be default light */
                MessageStreamFilterMessage(); 

                /** Let there be a specific light */
                MessageStreamFilterMessage(const MessageStreamFilter &filter);

                /** clone me */
                MessageStreamFilterMessage(const MessageStreamFilterMessage &other);

                /** Test of equality. Note that this includes the 
                 * "applyFilter" data. To compare the messageStreamFilters
                 * themselves, use operator== on theFilter data member.
                 */
                bool operator==(const MessageStreamFilterMessage &other) const;

                /** make me equal */
                MessageStreamFilterMessage &operator=(const MessageStreamFilterMessage &other);

                /** Drop me in the water. */
                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<MessageStreamFilterMessage> MessageStreamFilterMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const MessageStreamFilterMessage &mess);
    }
}
#endif // MESSAGE_STREAM_FILTER_MESSAGE

