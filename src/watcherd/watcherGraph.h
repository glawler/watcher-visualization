#ifndef WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
#define WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "libwatcher/watcherTypes.h"            // for Timestamp
#include "libwatcher/connectivityMessage.h"
#include "libwatcher/gpsMessage.h"
#include "libwatcher/nodeStatusMessage.h"
#include "libwatcher/edgeMessage.h"

#include "watcherdAPIMessageHandler.h"
#include "messageStreamFilter.h"

namespace watcher
{
    using namespace event;  // for libwatcher and messages. 

    /**
     * @class WatcherGraphNode
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * A class that holds the data at the vertexes of a WatcherGraph
     */
    class WatcherGraphNode
    {
        public:
            WatcherGraphNode();
            ~WatcherGraphNode();

            NodeIdentifier nodeId;
            GPSMessagePtr gpsData;
            std::string label;
            bool connected;

        protected:
        private:
    };

    /**
     * @class WatcherGraphEdge
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * A class that holds the data in the edges of a WatcherGraph
     */
    class WatcherGraphEdge
    {
        public:
            WatcherGraphEdge();
            ~WatcherGraphEdge();

            Color color;
            float expiration;
            float width;
            bool bidirectional;

        protected:
        private:
    };

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
             * The graph is a bidirectional adjacency_list. 
             */
            typedef boost::adjacency_list<
                boost::vecS, 
                boost::vecS, 
                boost::directedS,
                WatcherGraphNode,
                WatcherGraphEdge> Graph;

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
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

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
             * The actual boost::graph.
             */
            Graph theGraph;

        private:

            DECLARE_LOGGER();

            /** 
             * private data 
             **/

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
