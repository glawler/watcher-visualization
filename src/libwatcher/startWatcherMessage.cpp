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

/** @file startWatcherMessage.cpp
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#include "watcherSerialize.h"
#include "startWatcherMessage.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(StartMessage, "Message.StartMessage");

        StartMessage::StartMessage(): Message(START_MESSAGE_TYPE, START_MESSAGE_VERSION)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        std::ostream& operator<< (std::ostream& os, const StartMessage& /* m */)
        {
            return os << "StartMessage()";
        }

        bool operator== (const StartMessage& /* lhs */, const StartMessage& /* rhs */) { return true; };

        template <typename Archive> void StartMessage::serialize(Archive & ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            TRACE_EXIT();
        }

    }
}

BOOST_CLASS_EXPORT(watcher::event::StartMessage);
