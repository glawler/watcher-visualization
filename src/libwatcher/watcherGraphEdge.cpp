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

#include <boost/foreach.hpp>

// #include "watcherSerialize.h"
#include "watcherGraphEdge.h"

using namespace watcher; 

INIT_LOGGER(WatcherGraphEdge, "WatcherGraphEdge"); 

WatcherGraphEdge::WatcherGraphEdge() : 
    displayInfo(new EdgeDisplayInfo),
    labels(),
    expiration(Infinity) 
{ 
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherGraphEdge::~WatcherGraphEdge() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual 
std::ostream &WatcherGraphEdge::toStream(std::ostream &out) const
{
    TRACE_ENTER();

    out << " expiration: " << expiration;
    out << " num attached labels: " << labels.size(); 
   
    // When labelDisplayInfo gets oper<<, uncomment this. 
    // if (attachedLabels.size())
    // {
    //     BOOST_FOREACH(LabelMessagePtr lmp, attachedLabels)
    //         out << "[" << *lmp << "]"; 
    // }
    // else
    // {
    //     out << "(none)"; 
    // }

    TRACE_EXIT();
    return out; 
}

std::ostream &watcher::operator<<(std::ostream &out, const watcher::WatcherGraphEdge &edge)
{
    TRACE_ENTER();
    edge.operator<<(out);
    TRACE_EXIT();
    return out;
}

// template<typename Archive>
// void WatcherGraphEdge::serialize(Archive &ar, const unsigned int /* file_version */)
// {
//     TRACE_ENTER();
//     // ar & displayInfo;
//     // ar & labels; 
//     ar & expiration; 
//     
//     TRACE_EXIT();
// }
// 
// BOOST_CLASS_EXPORT(WatcherGraphEdge); 

