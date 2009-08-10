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
 * @file
 * @author Geoff.Lawler@cobham.com
 * @date 2009-05-19
 */

#include <boost/foreach.hpp>
// #include "watcherSerialize.h"

#include "watcherGraphNode.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherGraphNode, "WatcherGraphNode"); 

WatcherGraphNode::WatcherGraphNode() : 
    displayInfo(new NodeDisplayInfo),
    nodeId(), 
    gpsData(new GPSMessage), 
    connected(false), 
    labels()
{
    TRACE_ENTER();
    TRACE_EXIT();
}


WatcherGraphNode::~WatcherGraphNode()
{
    TRACE_ENTER();
    TRACE_EXIT();
}


// virtual 
std::ostream &WatcherGraphNode::toStream(std::ostream &out) const
{
    TRACE_ENTER();

    out << " nodeId: " << nodeId << " gpsData: " << gpsData << " connected: " << connected;
    // GTL output the display info here. 
    out << " attached labels: " << labels.size(); 

    // GTL - when there is an outpout operator on labelDisplayInfo, uncomment this.
    // if(labels.size())
    // {
    //     BOOST_FOREACH(LabelMessagePtr lmp, labels)
    //         out << "[" << *lmp << "]"; 
    // }
    // else
    //     out << "(none)"; 

    TRACE_EXIT();
    return out; 
}

std::ostream &watcher::operator<<(std::ostream &out, const watcher::WatcherGraphNode &node)
{
    TRACE_ENTER();
    node.operator<<(out);
    TRACE_EXIT();
    return out;
}

// template <typename Archive> 
// void WatcherGraphNode::serialize(Archive & ar, const unsigned int /* file_version */)
// {
//     TRACE_ENTER();
//     ar & nodeId;
//     ar & gpsData; 
//     ar & connected;
//     // ar & labels;
//     // ar & displayInfo; 
//     TRACE_EXIT(); 
// }
// 
// BOOST_CLASS_EXPORT(watcher::WatcherGraphNode); 

