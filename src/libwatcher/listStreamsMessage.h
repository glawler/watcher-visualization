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

#include <yaml.h>
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

				/** Serialize this message using a YAML::Emitter
				 * @param e the emitter to serialize to
				 * @return the emitter emitted to.
				 */
				virtual YAML::Emitter &serialize(YAML::Emitter &e) const; 

				/** Serialize from a YAML::Parser. 
				 * @param p the Parser to read from 
				 * @return the parser read from. 
				 */
				virtual YAML::Node &serialize(YAML::Node &n); 
			private:
				DECLARE_LOGGER();
		};

		typedef boost::shared_ptr<ListStreamsMessage> ListStreamsMessagePtr;

		std::ostream& operator<< (std::ostream& os, const EventStreamInfo& p);
		std::ostream& operator<< (std::ostream& os, const ListStreamsMessage& p);
	}
}
#endif
