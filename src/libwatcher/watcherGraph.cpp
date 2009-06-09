#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include <boost/bind.hpp>               // for boost::bind()
#include <boost/tuple/tuple.hpp>        // for tie()
#include <boost/foreach.hpp>
#include <boost/graph/graphviz.hpp>

#include <algorithm>

#include "messageStreamFilter.h"

#include "connectivityMessage.h"
#include "gpsMessage.h"
#include "nodeStatusMessage.h"
#include "edgeMessage.h"
#include "colorMessage.h"

#include "watcherGraph.h"

// #include "watcherSerialize.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherGraph, "WatcherGraph");

/// 
// Helper classes (functors) to integrate the graph into STL and other functions.
//
namespace watcher {

    namespace GraphFunctors
    {
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

        struct MatchMessageLabelPtr
        {
            MatchMessageLabelPtr(const LabelMessagePtr &l_) : l(l_) {} 
            const LabelMessagePtr &l;
            bool operator()(const LabelDisplayInfoPtr &lhs)
            {
                return 
                    l->label==lhs->labelText && 
                    l->layer==lhs->layer;       // GTL anything else? 
            }
        };
        struct MatchLabelLayer
        {
            MatchLabelLayer(const GUILayer &l_) : l(l_) {} 
            const GUILayer &l;
            bool operator()(const LabelDisplayInfoPtr &lhs)
            {
                return l == lhs->layer;
            }
        };
        struct LabelExpired
        {
            LabelExpired(const Timestamp &t) : now(t) {}
            const Timestamp now;
            bool operator()(const LabelDisplayInfoPtr &lhs)
            {
                return (lhs->expiration == Infinity) ? false : (now > lhs->expiration); 
            }
        };
        struct EdgeExpired
        {
            EdgeExpired(const WatcherGraph::Graph &g_, const Timestamp &t_) : g(g_), t(t_) {}
            const WatcherGraph::Graph &g;
            const Timestamp &t;
            bool operator()(boost::graph_traits<WatcherGraph::Graph>::edge_descriptor const &e)
            {
                return (g[e].expiration==Infinity) ? false : (t > g[e].expiration); 
            }
        };
        struct MatchEdgeLayer
        {
            const WatcherGraph::Graph &g;
            const GUILayer &layer;
            MatchEdgeLayer(const WatcherGraph::Graph &g_, const GUILayer &l_) : g(g_), layer(l_) {}
            bool operator()(const boost::graph_traits<WatcherGraph::Graph>::edge_descriptor &e)
            {
                return g[e].displayInfo->layer==layer;
            }
        };

        /** Helper class to print WatcherGraphNodes as graphviz data */
        struct WatcherNodeVertexGraphVizWriter 
        {
            const WatcherGraph::Graph &g;
            WatcherNodeVertexGraphVizWriter(const WatcherGraph::Graph &g_) : g(g_) { }

            void operator()(std::ostream &out, 
                    boost::graph_traits<WatcherGraph::Graph>::vertex_descriptor const &v) const
            {
                out << "[";
                stringstream label;
                label << "label=\"nodeId: " << g[v].nodeId;
                if (g[v].labels.size())
                    label << "\\n numlabels: " << g[v].labels.size(); 
                if (g[v].gpsData) 
                    label << "\\ngps: " << g[v].gpsData->x << "," << g[v].gpsData->y << "," <<  g[v].gpsData->z;
                label << "\""; 

                out << label.str(); 
                out << " color=" << g[v].displayInfo->color.toString();
                
                out << "]"; 
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
                out << "[";
                stringstream label;
                label << " label=\"";
                if (!g[e].displayInfo->label.empty())
                    label << "\\nlabel:" <<  g[e].displayInfo->label;
                if (g[e].labels.size())
                    label << "\\nnumlabels: " << g[e].labels.size(); 
                label << "\""; 

                out << " color=" << g[e].displayInfo->color;
                out << "]"; 
            }
        };
    }
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

bool WatcherGraph::addNodeNeighbors(const ConnectivityMessagePtr &message)
{
    TRACE_ENTER();

    LOG_DEBUG("Adding neighbors from message: " << *message); 
    LOG_DEBUG("Graph before adding: " << *this); 

    bool retVal=true;

    // GTL - find a better way to do this than removing all the edges, then 
    // adding them back. Do: 
    //     Find the intersection of the sets of the est of existing neighbors
    //     and the set of neighbors in the message. Then make sure that 
    //     the intersection of nodes exist. 
    graph_traits<Graph>::vertex_iterator src;
    boost::graph_traits<Graph>::out_edge_iterator i, end, newEnd;
    findOrCreateNode(message->fromNodeID, src, message->layer); 

    // Remove all edges on the same layer as the message. 
    tie(i, end)=out_edges(*src, theGraph);
    newEnd=remove_if(i, end, GraphFunctors::MatchEdgeLayer(theGraph, message->layer)); 
    for( ; newEnd!=end; ++newEnd)
        remove_edge(newEnd, theGraph);

    // Add edges from connectivity message
    graph_traits<Graph>::vertex_iterator dest;
    BOOST_FOREACH(NodeIdentifier nid, message->neighbors)
    {
        findOrCreateNode(nid, dest, message->layer); 
        std::pair<graph_traits<Graph>::edge_descriptor, bool> ei=add_edge(*src, *dest, theGraph);
        theGraph[ei.first].displayInfo->loadConfiguration(message->layer); 
    }
    
    LOG_DEBUG("Graph after adding: " << *this); 

    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::addEdge(const EdgeMessagePtr &message)
{
    TRACE_ENTER();
    bool retVal=true;

    graph_traits<Graph>::vertex_iterator src, dest, tmp;
    findOrCreateNode(message->node1, src, message->layer); 
    findOrCreateNode(message->node2, dest, message->layer); 

    std::pair<graph_traits<Graph>::edge_descriptor, bool> ei=add_edge(*src, *dest, theGraph);

    if(!ei.second) 
    {
        LOG_ERROR("Unable to add edge to graph between " << message->node1 << " and " << message->node2);
        TRACE_EXIT_RET_BOOL(false); 
        return false;
    }

    Timestamp now=Timestamp(time(NULL))*1000; 

    // Set expiration on this edge if needed. 
    if (message->expiration!=Infinity)
        theGraph[ei.first].expiration=now+message->expiration;  

    theGraph[ei.first].displayInfo->loadConfiguration(message->layer); 

    // set color if asked (will change edge display info for all nodes on the same layer
    theGraph[ei.first].displayInfo->color=message->edgeColor; 
    theGraph[ei.first].displayInfo->width=message->width;

    // If labels are specified, set them up. 
    if (message->node1Label && !message->node1Label->label.empty())
    {
        LabelDisplayInfoPtr lmp(new LabelDisplayInfo);
        lmp->loadConfiguration(message->node1Label); 
        theGraph[*src].labels.push_back(lmp);
    }
    if (message->middleLabel && !message->middleLabel->label.empty())
    {
        LabelDisplayInfoPtr lmp(new LabelDisplayInfo); 
        lmp->loadConfiguration(message->middleLabel); 
        theGraph[ei.first].labels.push_back(lmp);
    }
    if (message->node2Label && !message->node2Label->label.empty())
    {
        LabelDisplayInfoPtr lmp(new LabelDisplayInfo); 
        lmp->loadConfiguration(message->node2Label); 
        theGraph[*dest].labels.push_back(lmp);
    }

    if(message->bidirectional)
    {
        ei=add_edge(*dest, *src, theGraph);
        if(!ei.second) 
            LOG_ERROR("Unable to add edge to graph between " << message->node2 << " and " << message->node1);

        if (message->expiration!=Infinity)
            theGraph[ei.first].expiration=now+message->expiration;  

        theGraph[ei.first].displayInfo->loadConfiguration(message->layer); 
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}

void WatcherGraph::doMaintanence()
{
    TRACE_ENTER();

    Timestamp now=Timestamp(time(NULL))*1000; 

    // remove edges that have expired
    remove_edge_if(GraphFunctors::EdgeExpired(theGraph, now), theGraph); 

    // Remove expired edge labels
    graph_traits<Graph>::edge_iterator ei, eEnd;
    for(tie(ei, eEnd)=edges(theGraph); ei!=eEnd; ++ei)
    {
        WatcherGraphEdge::LabelList::iterator b=theGraph[*ei].labels.begin();
        WatcherGraphEdge::LabelList::iterator e=theGraph[*ei].labels.end();
        WatcherGraphEdge::LabelList::iterator newEnd=remove_if(b, e, GraphFunctors::LabelExpired(now));
        theGraph[*ei].labels.erase(newEnd, e);
    }

    // remove expired node labels
    graph_traits<Graph>::vertex_iterator vi, vEnd;
    for(tie(vi, vEnd)=vertices(theGraph); vi!=vEnd; ++vi)
    {
        WatcherGraphNode::LabelList::iterator b=theGraph[*vi].labels.begin();
        WatcherGraphNode::LabelList::iterator e=theGraph[*vi].labels.end();
        WatcherGraphNode::LabelList::iterator newEnd=remove_if(b, e, GraphFunctors::LabelExpired(now));
        theGraph[*vi].labels.erase(newEnd, e);
    }

    TRACE_EXIT();
}

bool WatcherGraph::updateNodeLocation(const GPSMessagePtr &message)
{
    TRACE_ENTER();
    
    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findOrCreateNode(message->fromNodeID, nodeIter, message->layer)) 
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
    if(findOrCreateNode(message->fromNodeID, nodeIter, message->layer))
    {
        LOG_DEBUG("Updating connection status for node " << theGraph[*nodeIter].nodeId);
        theGraph[*nodeIter].connected=message->event==NodeStatusMessage::connect ? true : false;
        retVal=true;
    }
    
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::updateNodeColor(const ColorMessagePtr & /*message*/)
{
    TRACE_ENTER();
    TRACE_EXIT_RET_BOOL(false);
    return false; 

    // bool retVal;
    // boost::graph_traits<Graph>::vertex_iterator nodeIter;
    // if(findOrCreateNode(message->fromNodeID, nodeIter))
    // {
    //     LOG_DEBUG("Updating color information for node " << theGraph[*nodeIter].nodeId);
    //     theGraph[*nodeIter].color.reset(new ColorMessage(*message)); 
    //     retVal=true;
    // }
    // 
    // TRACE_EXIT_RET(retVal);
    // return retVal;
}

bool WatcherGraph::addRemoveAttachedLabel(const LabelMessagePtr &message)
{
    TRACE_ENTER();

    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findOrCreateNode(message->fromNodeID, nodeIter, message->layer))
    {
        LOG_DEBUG("Updating attached label information for node " << theGraph[*nodeIter].nodeId);
        if (message->addLabel)
        {
            LabelDisplayInfoPtr lmp(new LabelDisplayInfo); 
            lmp->loadConfiguration(message); 
            theGraph[*nodeIter].labels.push_back(lmp);
        }
        else
        {
            // GTL - this is some ugly ass code. Compact, but very very ugly. 
            // See http://www.sgi.com/tech/stl/remove_if.html for details
            WatcherGraphNode::LabelList::iterator b=theGraph[*nodeIter].labels.begin();
            WatcherGraphNode::LabelList::iterator e=theGraph[*nodeIter].labels.end();
            WatcherGraphNode::LabelList::iterator newEnd=remove_if(b, e, GraphFunctors::MatchMessageLabelPtr(message));
            theGraph[*nodeIter].labels.erase(newEnd, e);
        }
        retVal=true;
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}

// virtual 
std::ostream &WatcherGraph::toStream(std::ostream &out) const
{
    TRACE_ENTER();

    write_graphviz(
            out, 
            theGraph, 
            GraphFunctors::WatcherNodeVertexGraphVizWriter(theGraph), 
            GraphFunctors::WatcherNodeEdgeGraphVizWriter(theGraph)); 

    TRACE_EXIT();
    return out;
}

bool WatcherGraph::updateGraph(const MessageStreamFilter &theFilter)
{
    // GTL - there are so many contradictions/problems/issues about filtering inmemory 
    // watcherGraph, (what if you filter out the physical layer - does everything 
    // else et deleted? what if you remove an edge that has labels on a different layer?) 
    // that maybe it is best to just destroy the existing graph 
    // and let the new incoming messages just rebuild it from the server filtered
    // messages. 
    TRACE_ENTER();

    bool retVal=false;
    // handle vertices
    {  
        graph_traits<Graph>::vertex_iterator i, end, next;
        tie(i, end) = vertices(theGraph);
        for(next=i; i!=end; i=next)
        {
            ++next;  // In case we have to remove the vertex itself, get pointer to next vertex.

            // GTL - TODO handle the region filter.  
            {
                LOG_ERROR("Filtering in-memory watcher data by region is not currently supported."); 
            }

            // handle layer - layer we can do. 
            {
                // If the node is going away, so are all the other layers on top of it. 
                if (theGraph[*i].displayInfo->layer==theFilter.getLayer())
                {
                    remove_vertex(*i, theGraph);     // GTL - what happens to the edges here? 
                    continue;                        // What if both vertices are removed? 
                }

                // remove the any vertex labels that have the same layer
                WatcherGraphNode::LabelList::iterator b=theGraph[*i].labels.begin();
                WatcherGraphNode::LabelList::iterator e=theGraph[*i].labels.end();
                WatcherGraphNode::LabelList::iterator newEnd=
                    remove_if(b, e, GraphFunctors::MatchLabelLayer(theFilter.getLayer()));
                theGraph[*i].labels.erase(newEnd, e);
            }
        }
    }
    // handle edges
    {
        // handle region
        {
            // if the vertices of the edge are removed above as 
            // part of the vertices region removal, do the edges still exist? 
        }

        // layers
        graph_traits<Graph>::edge_iterator i, end, next;
        tie(i, end) = edges(theGraph);
        for(next=i; i!=end; i=next) 
        {
            next++;
            if (theGraph[*i].displayInfo->layer==theFilter.getLayer())
            {
                remove_edge(*i, theGraph);    // GTL will this cause the dtor to be called on all labels attached to this edge?
                continue;
            }
        }
    }

    
    TRACE_EXIT_RET(retVal);
    return retVal;
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
        case LABEL_MESSAGE_TYPE:
            retVal=addRemoveAttachedLabel(dynamic_pointer_cast<LabelMessage>(message));
            break;
        case COLOR_MESSAGE_TYPE:
            retVal=updateNodeColor(dynamic_pointer_cast<ColorMessage>(message));
            break;
        default:
            retVal=false;
            break;
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
    retIter = find_if(beg, end, GraphFunctors::MatchNodeId(theGraph, id)); 

    bool retVal=retIter != end;
    LOG_DEBUG( (retVal?"Found":"Did not find") << " node " << id << " in the current graph"); 
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::findOrCreateNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter, const GUILayer &layer)
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
        theGraph[*retIter].displayInfo->loadConfiguration(layer, id); 
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::createNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter)
{
    TRACE_ENTER();
    graph_traits<Graph>::vertex_descriptor v = add_vertex(theGraph);
    theGraph[v].nodeId=id;
    retIter=v;
    TRACE_EXIT_RET(true);
    return true;
}

bool WatcherGraph::saveConfig() const
{
    TRACE_ENTER();
    graph_traits<Graph>::edge_iterator ei, eEnd;
    for(tie(ei, eEnd)=edges(theGraph); ei!=eEnd; ++ei)
    {
        theGraph[*ei].displayInfo->saveConfiguration(); 

        BOOST_FOREACH(const LabelDisplayInfoPtr &ldip, theGraph[*ei].labels)
            ldip->saveConfiguration();
    }
        
    graph_traits<Graph>::vertex_iterator vi, vEnd;
    for(tie(vi, vEnd)=vertices(theGraph); vi!=vEnd; ++vi)
    {
        theGraph[*vi].displayInfo->saveConfiguration();

        BOOST_FOREACH(const LabelDisplayInfoPtr &ldip, theGraph[*vi].labels)
            ldip->saveConfiguration();
    }

    TRACE_EXIT_RET_BOOL(true); 
    return true;
}
std::ostream &watcher::operator<<(std::ostream &out, const watcher::WatcherGraph &watcherGraph)
{
    TRACE_ENTER();
    watcherGraph.operator<<(out);
    TRACE_EXIT();
    return out;
}

// bool WatcherGraph::pack(ostream &os)
// {
//     TRACE_ENTER();
//     boost::archive::text_oarchive oa(os);
//     oa << (*this);
//     TRACE_EXIT();
//     return true; 
// }
// 
// WatcherGraphPtr WatcherGraph::unpack(std::istream& is)
// {
//     boost::archive::text_iarchive ia(is);
//     WatcherGraph* ret = 0;
//     try
//     {
//         ia >> ret;
//     }
//     catch (boost::archive::archive_exception& e)
//     {
//         LOG_WARN("Exception thrown while serializing the graph: " << e.what());
//         return WatcherGraphPtr();
//     }
//     return WatcherGraphPtr(ret); 
// }
// 
// template<typename Archive>
// void WatcherGraph::serialize(Archive &ar, const unsigned int /* file_version */)
// {
//     TRACE_ENTER();
//     ar & theGraph;
//     TRACE_EXIT();
// }
//
//
//BOOST_CLASS_EXPORT(watcher::WatcherGraph);


