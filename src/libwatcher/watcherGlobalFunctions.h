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
 * @file watcherGlobalFunctions.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_GLOBAL_FUNCTIONS
#define WATCHER_GLOBAL_FUNCTIONS

#include <boost/serialization/split_free.hpp>
#include "watcherTypes.h"

BOOST_SERIALIZATION_SPLIT_FREE(boost::asio::ip::address);

namespace boost 
{
    namespace serialization 
    {
        template<class Archive>
            void save(Archive & ar, const watcher::NodeIdentifier &a, const unsigned int /* version */)
            {
                unsigned long tmp=a.to_v4().to_ulong();
                ar & tmp;
            }

        template<class Archive>
            void load(Archive & ar, watcher::NodeIdentifier &a, const unsigned int /* version */)
            {
                unsigned long tmp;
                ar & tmp;
                a=watcher::NodeIdentifier(boost::asio::ip::address_v4(tmp));
            }
    }
}

#endif //  WATCHER_GLOBAL_FUNCTIONS
