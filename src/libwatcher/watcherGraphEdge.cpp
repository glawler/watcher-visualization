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
    out << " layer: " << layer; 
   
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

