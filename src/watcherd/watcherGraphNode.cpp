/**
 * @file
 * @author Geoff.Lawler@cobham.com
 * @date 2009-05-19
 */

#include <boost/foreach.hpp>

#include "watcherGraphNode.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherGraphNode, "WatcherGraphNode"); 

WatcherGraphNode::WatcherGraphNode() : 
        nodeId(), gpsData(), label(), connected(false), attachedLabels()
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

    out << " nodeId: " << nodeId << " gpsData: " << gpsData << " label: " << label << " connected: " << connected;
    out << " labels attached: ";
    BOOST_FOREACH(LabelMessagePtr lmp, attachedLabels)
        out << "[" << *lmp << "]"; 

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

