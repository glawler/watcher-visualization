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

/** @file streamDescriptionMessage.h
 */
#ifndef STREAM_DESCRIPTION_MESSAGE_H
#define STREAM_DESCRIPTION_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Get/Set the description name for a stream.
         */
        class StreamDescriptionMessage : public Message {
            public:
            StreamDescriptionMessage(); 
            StreamDescriptionMessage(const std::string&); 

	    std::string desc;

            private:
            friend class boost::serialization::access;
            template <typename Archive> void serialize(Archive& ar, const unsigned int version);
            DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<StreamDescriptionMessage> StreamDescriptionMessagePtr;

	std::ostream& operator<< (std::ostream&, const StreamDescriptionMessagePtr&);
    }
}
#endif
