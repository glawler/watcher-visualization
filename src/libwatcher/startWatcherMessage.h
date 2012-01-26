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

/** @file startWatcherMessage.h
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#ifndef START_WATCHER_MESSAGE_H
#define START_WATCHER_MESSAGE_H

#include <yaml.h>
#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Start playback of event stream.
         * @author Michael Elkins <michael.elkins@cobham.com>
         * @date 2009-03-20
         */
        class StartMessage : public Message {
            public:
                StartMessage(); 

				/** Serialize this message using a YAML::Emitter
				 * @param e the emitter to serialize to
				 * @return the emitter emitted to.
				 */
				virtual YAML::Emitter &serialize(YAML::Emitter &e) const; 

				/** Serialize from a YAML::Parser. 
				 * @param p the Parser to read from 
				 * @return the parser read from. 
				 */
				virtual YAML::Node &serialize(YAML::Node &node); 

            private:
                DECLARE_LOGGER();
        };

        std::ostream& operator<< (std::ostream&, const StartMessage&);

        typedef boost::shared_ptr<StartMessage> StartMessagePtr;

        bool operator== (const StartMessage& lhs, const StartMessage& rhs);
    }
}
#endif
