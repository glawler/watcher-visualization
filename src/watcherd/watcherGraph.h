#ifndef WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
#define WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "messageStreamFilter.h"
#include "watcherGraphNode.h"
#include "watcherGraphEdge.h"

// GTL - How do I hide the message types from clients of WatcherGraph. I tried ME's 'class impl'
// trick, but it did not work. It's very possible that I did not do it correctly. 
#include "libwatcher/connectivityMessage.h"
#include "libwatcher/gpsMessage.h"
#include "libwatcher/nodeStatusMessage.h"
#include "libwatcher/edgeMessage.h"

namespace watcher
{
    using namespace event;  // for libwatcher and messages. 

    /** 
     * @class WatcherGraph
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * WatcherGraph keeps track of the evolving topologies described by a series of watcher messages. 
     * i.e. you feed it watcher messages that have anything to do with edges and nodes, and it will
     * keep track of the state of the topologial graph represented by that stream of messages. i.e. you
     * throw a bunch of NodeStatus connect/disconnect, edgeMessage, and connectivityMessage messages
     * at it and it keeps track of eveything for you. It exports the current topology as a boost::graph.
     *
     * Currently this class just tracks nodes, locations, and connectivities. It may add labels, etc
     * if it turns out to be a good place to put that stuff.
     *
     */
    class WatcherGraph 
    {
        public:

            /** 
             * WatcherGraph
             */
            WatcherGraph();

            /**
             * On a large enough time line, the survival rate for everyone drops to zero.
             */
            virtual ~WatcherGraph();

            /**
             * The graph is a boost directed adjacency_list. This interface is public
             * so GUI developers can get direct access if needed. 
             */
            typedef boost::adjacency_list<
                boost::vecS, 
                boost::vecS, 
                boost::directedS,
                WatcherGraphNode,
                WatcherGraphEdge> Graph;
            /**
             * The actual boost::graph.
             */
            Graph theGraph;

            /**
             * updateGraph()
             * updateGraph takes a message, and if it the message applicable to the state of the 
             * watcherGraph, will update the internal graph state and execute the graphUpdated
             * callback (if it exists).
             * 
             * @param MessagePtr - the newly arrived data
             * @return bool - if the graph was updated then true, else false.
             */
            bool updateGraph(const MessagePtr &message);

            /**
             * updateGraph(filter)
             * Applies the past filter to the in-memory graph.
             * When a filter is applied to the MessageStream feeding the graph, 
             * it should also be applied to the graph instance, otherwise old data that does
             * not match the new stream may still be in the in-memory graph.
             */
             bool updateGraph(const MessageStreamFilter &filter); 

            /**
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

        private:

            DECLARE_LOGGER();

            /**
             * Update the graph with a list of neighbors addes or removed.
             */
            bool addNodeNeighbors(const ConnectivityMessagePtr &message);

            /**
             * Add or remove a single edge in the graph.
             */
            bool addEdge(const EdgeMessagePtr &message);

            /**
             * Update a node's location.
             */
            bool updateNodeLocation(const GPSMessagePtr &message);

            /**
             * Update a node's state (connected or disconnected).
             */
            bool updateNodeStatus(const NodeStatusMessagePtr &message);

            /**
             * Update an attached label - either add or remove it.
             */
            bool addRemoveAttachedLabel(const LabelMessagePtr &message);

            /** Find a node in the graph based on a NodeIdentifier 
             * @param[in] id - the id of the node you want to find. 
             * @param[out] retIter - an iterator that points to the found node.
             * @return bool - true if successful, false otherwise
             */
            bool findNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retVal);

            /** Find a node, if it doesn't exist, create it. Returns iterator to the node 
             * @param[in] id - the id of the node you want to find/create.
             * @param[out] retIter - an iterator that points to the found node.
             * @return bool - true if successful, false otherwise
             */
            bool findOrCreateNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter);

            /** Create a node and return an iterator to it. 
             * @param[in] id - the id of the node you want to create. 
             * @param[out] retIter - an iterator that points to the found node.
             * @return bool - true if successful, false otherwise
             */
            bool createNode(const NodeIdentifier &id, boost::graph_traits<Graph>::vertex_iterator &retIter);

    }; // like a fired school teacher.

    /** typedef a shared pointer to this class
    */
    typedef boost::shared_ptr<WatcherGraph> WatcherGraphPtr;

    /** write a human readable version of the WatcherGraph class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const WatcherGraph &watcherGraph);

}

#endif // WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
