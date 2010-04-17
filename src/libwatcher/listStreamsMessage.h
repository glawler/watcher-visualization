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

/** @file listStreamsMessage.h
 */
#ifndef LIST_STREAMS_MESSAGE_H
#define LIST_STREAMS_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

	class EventStreamInfo {
	    public:
		EventStreamInfo();
		EventStreamInfo(uint32_t, const std::string&);
		uint32_t uid;			//< unique identifier for this stream
		std::string description;	//< human readable string describing this event stream
	    private:
		friend class boost::serialization::access;
		template <typename Archive> void serialize(Archive& ar, const unsigned int version);
	};
	typedef boost::shared_ptr<EventStreamInfo> EventStreamInfoPtr;

        /**
         * Request/List of Watcher event streams.
         */
        class ListStreamsMessage : public Message {
            public:
            ListStreamsMessage(); 
	    /*virtual*/ std::ostream& toStream(std::ostream&) const;

	    std::vector<EventStreamInfoPtr> evstreams;

            private:
            friend class boost::serialization::access;
            template <typename Archive> void serialize(Archive& ar, const unsigned int version);
            DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<ListStreamsMessage> ListStreamsMessagePtr;

	std::ostream& operator<< (std::ostream& os, const EventStreamInfo& p);
	std::ostream& operator<< (std::ostream& os, const ListStreamsMessage& p);
    }
}
#endif
