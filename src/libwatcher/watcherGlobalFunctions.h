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
                std::string tmp=a.to_string();
                ar & tmp;
            }

        template<class Archive>
            void load(Archive & ar, watcher::NodeIdentifier &a, const unsigned int /* version */)
            {
                std::string tmp;
                ar & tmp;
                a=boost::asio::ip::address::from_string(tmp);
            }
    }
}

#endif //  WATCHER_GLOBAL_FUNCTIONS
