#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include <boost/bind.hpp>               // for boost::bind()
#include <boost/tuple/tuple.hpp>        // for tie()
#include <boost/foreach.hpp>
#include <boost/graph/graphviz.hpp>

#include <algorithm>

#include "watcherGraph.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherGraph, "WatcherGraph");

/// 
// Helper classes (functors) to integrate the graph into STL and other functions.
//
namespace watcher {
    /** Helper class to find nodes by their nodeIds */
    class MatchNodeId
    {
        const WatcherGraph::Graph &g;
        const NodeIdentifier &id;
        public:
        MatchNodeId(const WatcherGraph::Graph &g_, const NodeIdentifier &id_) : g(g_), id(id_) {}
        bool operator()(boost::graph_traits<WatcherGraph::Graph>::vertex_descriptor const &v)
        {
            return g[v].nodeId == id;
        }
    }; // class MatchNodeId

    /** Helper class to print WatcherGraphNodes as graphviz data */
    struct WatcherNodeVertexGraphVizWriter 
    {
        const WatcherGraph::Graph &g;
        WatcherNodeVertexGraphVizWriter(const WatcherGraph::Graph &g_) : g(g_) { }

        void operator()(std::ostream &out, 
                boost::graph_traits<WatcherGraph::Graph>::vertex_descriptor const &v) const
        {
            out << "[label=\"";
            out << "nodeId:" << g[v].nodeId;
            out << "\\ngpsData: " << g[v].gpsData;
            out << "\\nlabel: " << g[v].label;
            out << "\\nconnected: "<< g[v].connected;
            out << "\"]";
        }
    };

    /** Helper class to print WatcherGraphEdges as graphviz data */
    struct WatcherNodeEdgeGraphVizWriter 
    {
        const WatcherGraph::Graph &g;
        WatcherNodeEdgeGraphVizWriter(const WatcherGraph::Graph &g_) : g(g_) { }

        void operator()(std::ostream &out, 
                boost::graph_traits<WatcherGraph::Graph>::edge_descriptor const &e) const
        {
            out << "[color=\"" << g[e].color << "\""; 
            out << " label=\"";
            out << "\\nexpiration:" << g[e].expiration;
            out << "\\nwidth:" << g[e].width;
            out << "\\nbidirectional:" << g[e].bidirectional;
            out << "\"]";
        }
    };
}



//// Node functions.
WatcherGraphNode::WatcherGraphNode() : 
    nodeId(), gpsData(), connected(false) 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherGraphNode::~WatcherGraphNode() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

//// Edge functions
WatcherGraphEdge::WatcherGraphEdge() : 
    color(Color::blue), 
    expiration(5000), 
    width(30), 
    bidirectional(false) 
{ 
    TRACE_ENTER();
    TRACE_EXIT();
}

WatcherGraphEdge::~WatcherGraphEdge() 
{
    TRACE_ENTER();
    TRACE_EXIT();
}

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

    write_graphviz(
            out, 
            theGraph, 
            WatcherNodeVertexGraphVizWriter(theGraph), 
            WatcherNodeEdgeGraphVizWriter(theGraph)); 

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

bool WatcherGraph::addNodeNeighbors(const ConnectivityMessagePtr &message)
{
    TRACE_ENTER();

    LOG_DEBUG("Adding neighbors from message: " << *message); 
    LOG_DEBUG("Graph before adding: " << *this); 

    bool retVal=true;

    // There is probably a faster way to do this than removing all
    // edges then creating all new ones. The majority of the time I 
    // would think that these edge sets would be very similar

    // remove old edges
    graph_traits<Graph>::vertex_iterator src;
    boost::graph_traits<Graph>::out_edge_iterator i, end;
    findOrCreateNode(message->fromNodeID, src);
    for(tie(i, end)=out_edges(*src, theGraph); i!=end; ++i)
         remove_edge(i, theGraph);

    // Add edges from connectivity message
    graph_traits<Graph>::vertex_iterator dest;
    BOOST_FOREACH(NodeIdentifier nid, message->neighbors)
    {
        findOrCreateNode(nid, dest);
        add_edge(*src, *dest, theGraph);
    }
    
    LOG_DEBUG("Graph after adding: " << *this); 

    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::addEdge(const EdgeMessagePtr &message)
{
    TRACE_ENTER();
    bool retVal=true;

    graph_traits<Graph>::vertex_iterator src, dest;
    findOrCreateNode(message->node1, src);
    findOrCreateNode(message->node2, dest);

    std::pair<graph_traits<Graph>::edge_descriptor, bool> edgeIter=add_edge(*src, *dest, theGraph);
   
    // The edge may already exist. Just oveerwrite the values. 
    theGraph[edgeIter.first].color=message->edgeColor;
    theGraph[edgeIter.first].expiration=message->expiration;
    theGraph[edgeIter.first].width=message->width;
    theGraph[edgeIter.first].bidirectional=message->bidirectional;

    if(message->bidirectional)
    {
        std::pair<graph_traits<Graph>::edge_descriptor, bool> edgeIter=add_edge(*dest, *src, theGraph);

        // The edge may already exist. Just oveerwrite the values. 
        theGraph[edgeIter.first].color=message->edgeColor;
        theGraph[edgeIter.first].expiration=message->expiration;
        theGraph[edgeIter.first].width=message->width;
        theGraph[edgeIter.first].bidirectional=message->bidirectional;
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::updateNodeLocation(const GPSMessagePtr &message)
{
    TRACE_ENTER();
    
    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findOrCreateNode(message->fromNodeID, nodeIter))
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
    if(findOrCreateNode(message->fromNodeID, nodeIter))
    {
        LOG_DEBUG("Updating GPS information for node " << theGraph[*nodeIter].nodeId);
        theGraph[*nodeIter].connected=message->event==NodeStatusMessage::connect ? true : false;
        retVal=true;
    }
    
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::findNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter)
{
    TRACE_ENTER();

    graph_traits<Graph>::vertex_iterator beg;
    graph_traits<Graph>::vertex_iterator end;
    tie(beg, end) = vertices(theGraph);

    // GTL - may be a way to use boost::bind() here instead
    // of the auxillary class MatchNodeId
    retIter = find_if(beg, end, MatchNodeId(theGraph, id)); 

    bool retVal=retIter != end;
    LOG_DEBUG( (retVal?"Found":"Did not find") << " node " << id << " in the current graph"); 
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::findOrCreateNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter)
{
    TRACE_ENTER();
    bool retVal=true;
    if(!findNode(id, retIter))
    {
        if(!createNode(id, retIter))
        {
            LOG_ERROR("Unable to create new node for id " << id << " in watcherGraph");
            retVal=false;
        }
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::createNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter)
{
    TRACE_ENTER();
    graph_traits<Graph>::vertex_descriptor v = add_vertex(theGraph);
    theGraph[v].nodeId=id;
    theGraph[v].label=id.to_string(); 
    bool retVal=findNode(id, retIter);
    TRACE_EXIT_RET(retVal);
    return retVal;
}

std::ostream &watcher::operator<<(std::ostream &out, const watcher::WatcherGraph &watcherGraph)
{
    TRACE_ENTER();
    watcherGraph.operator<<(out);
    TRACE_EXIT();
    return out;
}

