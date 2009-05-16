#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include <boost/bind.hpp>               // for boost::bind()
#include <boost/tuple/tuple.hpp>        // for tie()

#include <algorithm>

#include "watcherGraph.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherGraph, "WatcherGraph");

WatcherGraph::WatcherGraph()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual 
WatcherGraph::~WatcherGraph()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// virtual 
std::ostream &WatcherGraph::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    TRACE_EXIT();
    return out;
}

bool WatcherGraph::updateGraph(const MessagePtr &message)
{
    TRACE_ENTER();
    bool retVal=false;

    switch(message->type)
    {
        case CONNECTIVITY_MESSAGE_TYPE:
            retVal=addNodeNeighbors(dynamic_pointer_cast<ConnectivityMessage>(message));
            break;
        case EDGE_MESSAGE_TYPE:
            retVal=addEdge(dynamic_pointer_cast<EdgeMessage>(message));
            break;
        case GPS_MESSAGE_TYPE:
            retVal=updateNodeLocation(dynamic_pointer_cast<GPSMessage>(message));
            break;
        case MESSAGE_STATUS_TYPE:
            retVal=updateNodeStatus(dynamic_pointer_cast<NodeStatusMessage>(message));
            break;
        default:
            retVal=false;
            break;
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::addNodeNeighbors(const ConnectivityMessagePtr &)
{
    TRACE_ENTER();
    bool retVal=true;
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::addEdge(const EdgeMessagePtr &)
{
    TRACE_ENTER();
    bool retVal=true;
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::updateNodeLocation(const GPSMessagePtr &message)
{
    TRACE_ENTER();
    
    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findNode(message->fromNodeID, nodeIter))
    {
        LOG_DEBUG("Updating GPS information for node " << theGraph[*nodeIter].nodeId);
        theGraph[*nodeIter].gpsData=message; 
        retVal=true;
    }
    
    TRACE_EXIT_RET(retVal);
    return retVal;
}
bool WatcherGraph::updateNodeStatus(const NodeStatusMessagePtr &message)
{
    TRACE_ENTER();

    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findNode(message->fromNodeID, nodeIter))
    {
        LOG_DEBUG("Updating GPS information for node " << theGraph[*nodeIter].nodeId);
        theGraph[*nodeIter].connected=message->event==NodeStatusMessage::connect ? true : false;
        retVal=true;
    }
    
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::findNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retVal)
{
    TRACE_ENTER();

    graph_traits<Graph>::vertex_iterator beg;
    graph_traits<Graph>::vertex_iterator end;
    tie(beg, end) = vertices(theGraph);

    // GTL - may be a way to use boost::bind() here instead
    // of the auxillary class MatchNodeId
    retVal = find_if(beg, end, MatchNodeId(theGraph, id)); 

    TRACE_EXIT();
    return retVal != end;
}

/* global operations */

std::ostream &operator<<(std::ostream &out, const WatcherGraph &watcherGraph)
{
    TRACE_ENTER();
    watcherGraph.operator<<(out);
    TRACE_EXIT();
    return out;
}

