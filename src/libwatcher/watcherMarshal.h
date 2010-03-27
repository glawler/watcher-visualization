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

#ifndef watcher_marshal_h
#define watcher_marshal_h

#include "marshal.hpp"

namespace boost {
    namespace asio {
        namespace ip {
            // manipulator for reading a NodeIdentifier
            Marshal::Input& operator>> (Marshal::Input& in, address& id)
            {
                std::string s;
                in >> s;
                id = address::from_string(s);
                return in;
            }
        }
    }
}

namespace watcher {
    namespace event {
#if 0
        // manipulator for reading a NodeIdentifier
        Marshal::Input& operator>> (Marshal::Input& in, NodeIdentifier& id)
        {
            std::string s;
            in >> s;
            id = boost::asio::ip::address::from_string(s);
            return in;
        }
#endif

        //manipulator for writing a NodeIdentifier
        Marshal::Output& operator<< (Marshal::Output& out, const NodeIdentifier& id)
        {
            out << id.to_string();
            return out;
        }
    }
}

#endif /* watcher_marshal_h */
