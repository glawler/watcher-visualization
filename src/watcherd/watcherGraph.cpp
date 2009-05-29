#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include <boost/bind.hpp>               // for boost::bind()
#include <boost/tuple/tuple.hpp>        // for tie()
#include <boost/foreach.hpp>
#include <boost/graph/graphviz.hpp>

#include <algorithm>

#include "libwatcher/connectivityMessage.h"
#include "libwatcher/gpsMessage.h"
#include "libwatcher/nodeStatusMessage.h"
#include "libwatcher/edgeMessage.h"
#include "libwatcher/colorMessage.h"

#include "watcherGraph.h"

#include <boost/graph/adj_list_serialize.hpp>
#include "libwatcher/watcherSerialize.h"

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
            bool operator()(const LabelMessagePtr &lhs)
            {
                return *l == *lhs;
            }
        };
        struct MatchLabelLayer
        {
            MatchLabelLayer(const GUILayer &l_) : l(l_) {} 
            const GUILayer &l;
            bool operator()(const LabelMessagePtr &lhs)
            {
                return l == lhs->layer;
            }
        };
        struct LabelExpired
        {
            LabelExpired(const Timestamp &t) : now(t) {}
            const Timestamp now;
            bool operator()(const LabelMessagePtr lhs)
            {
                return (lhs->expiration > 0) ? (now > lhs->expiration) : false; 
            }
        };
        struct EdgeExpired
        {
            EdgeExpired(const WatcherGraph::Graph &g_, const Timestamp &t_) : g(g_), t(t_) {}
            const WatcherGraph::Graph &g;
            const Timestamp &t;
            bool operator()(boost::graph_traits<WatcherGraph::Graph>::edge_descriptor const &e)
            {
                return (g[e].expiration > 0) ? (t > g[e].expiration) : false; 
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
                if (g[v].attachedLabels.size())
                    label << "\\n numlabels: " << g[v].attachedLabels.size(); 
                if (g[v].gpsData) 
                    label << "\\ngps: " << g[v].gpsData->x << "," << g[v].gpsData->y << "," <<  g[v].gpsData->z;
                label << "\""; 

                out << label.str(); 
                if (g[v].color) 
                    out << " color=" << g[v].color->color.toString();
                else
                    out << " color=black"; 
                
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
                if (!g[e].label.empty())
                    label << "\\nlabel:" <<  g[e].label;
                if (g[e].attachedLabels.size())
                    label << "\\nnumlabels: " << g[e].attachedLabels.size(); 
                label << "\""; 

                out << label.str(); 
                out << " color=" << g[e].color;
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
        std::pair<graph_traits<Graph>::edge_descriptor, bool> ei=add_edge(*src, *dest, theGraph);
        theGraph[ei.first].layer=message->layer; 
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
    findOrCreateNode(message->node1, src);
    findOrCreateNode(message->node2, dest);

    std::pair<graph_traits<Graph>::edge_descriptor, bool> ei=add_edge(*src, *dest, theGraph);

    if(!ei.second) 
        LOG_ERROR("Unable to add edge to graph between " << message->node1 << " and " << message->node2);

    Timestamp now=Timestamp(time(NULL))*1000; 

    theGraph[ei.first].color=message->edgeColor; 
    if (message->expiration>0) 
        theGraph[ei.first].expiration=now+message->expiration;  
    theGraph[ei.first].width=message->width;
    theGraph[ei.first].layer=message->layer;

    if (message->node1Label && !message->node1Label->label.empty())
    {
        LabelMessagePtr lmp(new LabelMessage(*message->node1Label));
        if (lmp->expiration>0)
            lmp->expiration+=now; 
        theGraph[*src].attachedLabels.push_back(lmp);
    }
    if (message->middleLabel && !message->middleLabel->label.empty())
    {
        LabelMessagePtr lmp(new LabelMessage(*message->middleLabel));
        if (lmp->expiration>0)
            lmp->expiration+=now; 
        theGraph[ei.first].attachedLabels.push_back(lmp);
    }
    if (message->node2Label && !message->node2Label->label.empty())
    {
        LabelMessagePtr lmp(new LabelMessage(*message->node2Label));
        if (lmp->expiration>0)
            lmp->expiration+=now; 
        theGraph[*dest].attachedLabels.push_back(lmp);
    }

    if(message->bidirectional)
    {
        ei=add_edge(*dest, *src, theGraph);
        if(!ei.second) 
            LOG_ERROR("Unable to add edge to graph between " << message->node2 << " and " << message->node1);

        theGraph[ei.first].color=message->edgeColor; 
        theGraph[ei.first].expiration=message->expiration>0 ? Timestamp(message->expiration+(time(NULL)*1000)) : -1;
        theGraph[ei.first].width=message->width;

        if (message->middleLabel && !message->middleLabel->label.empty())
        {
            LabelMessagePtr lmp(new LabelMessage(*message->middleLabel));
            if (lmp->expiration>0)
                lmp->expiration+=now; 
            theGraph[ei.first].attachedLabels.push_back(lmp);
        }
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
        WatcherGraphNode::LabelMessageList::iterator b=theGraph[*ei].attachedLabels.begin();
        WatcherGraphNode::LabelMessageList::iterator e=theGraph[*ei].attachedLabels.end();
        WatcherGraphNode::LabelMessageList::iterator newEnd=remove_if(b, e, GraphFunctors::LabelExpired(now));
        theGraph[*ei].attachedLabels.erase(newEnd, e);
    }

    // remove expired node labels
    graph_traits<Graph>::vertex_iterator vi, vEnd;
    for(tie(vi, vEnd)=vertices(theGraph); vi!=vEnd; ++vi)
    {
        WatcherGraphNode::LabelMessageList::iterator b=theGraph[*vi].attachedLabels.begin();
        WatcherGraphNode::LabelMessageList::iterator e=theGraph[*vi].attachedLabels.end();
        WatcherGraphNode::LabelMessageList::iterator newEnd=remove_if(b, e, GraphFunctors::LabelExpired(now));
        theGraph[*vi].attachedLabels.erase(newEnd, e);
    }

    TRACE_EXIT();
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
        LOG_DEBUG("Updating connection status for node " << theGraph[*nodeIter].nodeId);
        theGraph[*nodeIter].connected=message->event==NodeStatusMessage::connect ? true : false;
        retVal=true;
    }
    
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::updateNodeColor(const ColorMessagePtr &message)
{
    TRACE_ENTER();

    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findOrCreateNode(message->fromNodeID, nodeIter))
    {
        LOG_DEBUG("Updating color information for node " << theGraph[*nodeIter].nodeId);
        theGraph[*nodeIter].color.reset(new ColorMessage(*message)); 
        retVal=true;
    }
    
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool WatcherGraph::addRemoveAttachedLabel(const LabelMessagePtr &message)
{
    TRACE_ENTER();

    bool retVal;
    boost::graph_traits<Graph>::vertex_iterator nodeIter;
    if(findOrCreateNode(message->fromNodeID, nodeIter))
    {
        LOG_DEBUG("Updating attached label information for node " << theGraph[*nodeIter].nodeId);
        if (message->addLabel)
        {
            LabelMessagePtr lmp(new LabelMessage(*message));
            lmp->expiration=(message->expiration<0) ? -1 : message->expiration+(Timestamp(time(NULL))*1000); 
            theGraph[*nodeIter].attachedLabels.push_back(lmp);
        }
        else
        {
            // GTL - this is some ugly ass code. Compact, but very very ugly. 
            // See http://www.sgi.com/tech/stl/remove_if.html for details
            WatcherGraphNode::LabelMessageList::iterator b=theGraph[*nodeIter].attachedLabels.begin();
            WatcherGraphNode::LabelMessageList::iterator e=theGraph[*nodeIter].attachedLabels.end();
            WatcherGraphNode::LabelMessageList::iterator newEnd=remove_if(b, e, GraphFunctors::MatchMessageLabelPtr(message));
            theGraph[*nodeIter].attachedLabels.erase(newEnd, e);
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
                if (theGraph[*i].layer==theFilter.getLayer())
                {
                    remove_vertex(*i, theGraph);     // GTL - what happens to the edges here? 
                    continue;                        // What if both vertices are removed? 
                }

                // remove the any vertex labels that have the same layer
                WatcherGraphNode::LabelMessageList::iterator b=theGraph[*i].attachedLabels.begin();
                WatcherGraphNode::LabelMessageList::iterator e=theGraph[*i].attachedLabels.end();
                WatcherGraphNode::LabelMessageList::iterator newEnd=
                    remove_if(b, e, GraphFunctors::MatchLabelLayer(theFilter.getLayer()));
                theGraph[*i].attachedLabels.erase(newEnd, e);
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
            if (theGraph[*i].layer==theFilter.getLayer())
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

bool WatcherGraph::pack(ostream &os)
{
    TRACE_ENTER();
    boost::archive::text_oarchive oa(os);
    oa << (*this);
    TRACE_EXIT();
    return true; 
}

WatcherGraphPtr WatcherGraph::unpack(std::istream& is)
{
    boost::archive::text_iarchive ia(is);
    WatcherGraph* ret = 0;
    try
    {
        ia >> ret;
    }
    catch (boost::archive::archive_exception& e)
    {
        LOG_WARN("Exception thrown while serializing the graph: " << e.what());
        return WatcherGraphPtr();
    }
    return WatcherGraphPtr(ret); 
}

template<typename Archive>
void WatcherGraph::serialize(Archive &ar, const unsigned int /* file_version */)
{
    TRACE_ENTER();
    ar & theGraph;
    TRACE_EXIT();
}


BOOST_CLASS_EXPORT(watcher::WatcherGraph);


