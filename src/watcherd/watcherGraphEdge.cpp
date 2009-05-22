#include <boost/foreach.hpp>
#include "watcherGraphEdge.h"

using namespace watcher; 

INIT_LOGGER(WatcherGraphEdge, "WatcherGraphEdge"); 

WatcherGraphEdge::WatcherGraphEdge() : 
    label(), color(Color::blue), expiration(-1), width(30), attachedLabels(), layer()
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

    out << " label: " << label; 
    out << " layer: " << layer;
    out << " labels attached: ";

    BOOST_FOREACH(LabelMessagePtr lmp, attachedLabels)
        out << "[" << *lmp << "]"; 

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

