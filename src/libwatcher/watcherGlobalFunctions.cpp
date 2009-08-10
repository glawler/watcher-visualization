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

#include <boost/asio.hpp>

#include "watcherSerialize.h"
#include "watcherGlobalFunctions.h"

// void boost::serialization::serialize(boost::archive::polymorphic_iarchive & ar, boost::asio::ip::address &address, const unsigned int /* file_version */)
// {
//     std::string tmp;
//     ar & tmp;
//     address=boost::asio::ip::address::from_string(tmp);
// }
// 
// void boost::serialization::serialize(boost::archive::polymorphic_oarchive & ar, boost::asio::ip::address &address, const unsigned int /* file_version */)
// {
//     std::string tmp=address.to_string();
//     ar & tmp;
// }

