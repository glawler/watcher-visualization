/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file watcherGraph.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
#define WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS

#include <boost/function.hpp>

#include <map>
// #include <unordered_map>        // TR1 requires extra compile flag to get functionality

#include "watcherLayerData.h"
#include "nodeDisplayInfo.h"

#include "connectivityMessage.h"
#include "gpsMessage.h"
#include "nodeStatusMessage.h"
#include "edgeMessage.h"
#include "nodePropertiesMessage.h"
#include "colorMessage.h"


namespace watcher
{
    using namespace event;      // for libwatcher and messages. 
    class MessageStreamFilter;  // 

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
             * WatcherGraph. Must specify the maximum number of nodes in the test.
             */
            WatcherGraph(const size_t &maxNodes, const size_t &maxLayers);

            /**
             * On a large enough time line, the survival rate for everyone drops to zero.
             */
            virtual ~WatcherGraph();

            /**
             * Fixed-size array of nodeDisplayInfos. 
             *
             * To get the index into the array for a particular node, 
             * call nid2Index(node's address). To iterate over all known nodes,
             * interate from 0..maxNodesIndex. The array is filled in as new nodes are
             * heard from and maxNodesIndex is set to the last "valid" nodeDisplayInfo
             * index.
             *
             * ex: 
             * for (size_t i=0; i!=graph.numValidNodes; i++) 
             *      doSomethingWithNode(nodes[i]); 
             *
             * or to access a node directly if you know the ip address:
             * node[nid2Index(address)]
             *
             */
            NodeDisplayInfo *nodes;

            /** numner of valid nodes in nodes */
            size_t numValidNodes;

            /**
             * Convert a watcher nodeId into an integer that cna be used to index
             * into the various arrays of nodes, edges, and labels. This function 
             * takes O(1) time to do the mapping.
             *
             * ex: 
             * cout << nodes[nid2Index(message->fromNodeId)] << endl;
             *
             * ex:
             * int nid=nid2Index(message->fromNodeId);
             * nodes[nid].label="wello horld"; 
             * nodes[nid].color=watcher::colors::darkorange;
             *
             * size_t nid1=nid2Index(message->node1);
             * size_t nid2=nid2Index(message->node2);
             * layers[message->layer].edges[nid1][nid2]=true;
             *
             */
            size_t nid2Index(const NodeIdentifier &nid);

            /**
             * @param the index to be mapped back to a ipv4 address
             * @return the ipv4 address as size_t in network byte order. 
             *
             * ex:
             * struct in_addr addr;
             * char buf[24];
             * addr.s_addr=graph.index2Nid(i);
             * printf("addr: %s\n", inet_ntop(AF_INET, addr, buf, sizeof(buf)));
             *
             */
            unsigned int index2Nid(const size_t index) const; 

            /**
             * all layer data, including edges and labels on a per layer instance. 
             *
             * Inside each layer is an array of lables on nodes, floating labels, 
             * and edges for that layer. Edge existence between two nodes is stored
             * in a boolean matrix. See WatcherLayerData.h for more info.
             */
            WatcherLayerData *layers;

            /** largest index into layers array that is currently valid */
            size_t numValidLayers;

            /** 
             * Get the layer named name. This initializes the layer if it does not 
             * exist! Returns an index into layers where layer "name" exists. Exits if
             * creating the layer would go past numLayers so be careful. 
             */
            size_t name2LayerIndex(const std::string &name); 

            /**
             * @return true if layer exists
             */
            bool layerExists(const std::string &name) const; 

            /**
             * updateGraph takes a message, and if it the message applicable to the state of the 
             * watcherGraph, will update the internal graph state and execute the graphUpdated
             * callback (if it exists).
             * 
             * @param message  the newly arrived data
             * @return if the graph was updated then true, else false.
             */
            bool updateGraph(const MessagePtr &message);

            /**
             * If you want your GPS coords in watcherGraph to be in a different 
             * unit (say OpenGL screen coords), set this function pointer and 
             * watcherGraph will use it to translate all incoming GPS data into
             * your unit.
             */
            typedef boost::function<bool (double &x, double &y, double &z, const GPSMessage::DataFormat &f)> LocationTranslateFunction;
            LocationTranslateFunction locationTranslationFunction;

            /**
             * Update Graph internals (component experations, etc). 
             * Should be called periodically. If a timestamp is given,
             * use that for current time.
             */
            void doMaintanence(const watcher::Timestamp &ts=0);

            /**
             * If you're using playback time for doMaintanence(), then you must 
             * notify watcherGraph of the direction of time so it can compute
             * expirations correctly. If the direction changes, then there are 
             * no current events as nothing has happened yet, so the entire 
             * graph is cleared. Note: clients who use threads to read/update the
             * graph must insure that there is only one thread accessing the 
             * the labels on any layer when it clears itself. 
             */
            void setTimeDirectionForward(bool forward); 

            /**
             * clear all data.
             */
            void clear();

            /**
             * Save current configuration of all labels, nodes, and edges to the SingletonCconfig 
             * instance. Call this before saving system configuration to a cfg file. 
             * @retval true
             */
            bool saveConfiguration(void); 

            /**
             * Write an instance of this class as a human readable stream to the otream given
             * @param out the output stream
             * @return reference to output stream passed in
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * @param out the output stream
             * @return reference to output stream passed in
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            /** 
             * Serialize the Graph to a stream
             */
             // bool pack(std::ostream &os);

             /**
              * Unserialize a Graph from a stream
              */
             // WatcherGraphPtr unpack(std::istream& is);

        protected:

        private:

            // friend class boost::serialization::access;
            // template <typename Archive> void serialize(Archive & ar, const unsigned int /* file_version */);

            DECLARE_LOGGER();

            /** max number of supported nodes. */
            size_t maxNumNodes; 

            /** max number of supported layers. */
            size_t maxNumLayers; 

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
             * Update a node's color.
             */
            bool updateNodeColor(const ColorMessagePtr &message);

            /**
             * Update an attached label - either add or remove it.
             */
            bool addRemoveLabel(const LabelMessagePtr &message);

            /**
             * Update, create, or remove a node's properties.
             */
            bool updateNodeProperties(const NodePropertiesMessagePtr &message);

            /** Keep track of which direction we're going in time. */
            bool timeForward;

            /** Build a map of ipv4 addresses to indexes. These indexes are used to index into 
             * the node array and the edges arrays. 
             */
            // GTL - move to unordered_map to get constant time lookups. 
            // typedef std::unordered_map<unsigned int, size_t> NID2IndexMap;
            typedef std::map<unsigned int, size_t> NID2IndexMap;
            NID2IndexMap nid2IndexMap; 

            /** in case anyone needs to map an index back to an address, we keep the
             * addresses in a big array indexed by the index. :)
             */
            unsigned int *index2nidMap;

            typedef std::map<std::string, size_t> Name2LayerIndexMap; 
            Name2LayerIndexMap layerIndexMap; 

    }; // like a fired school teacher.

    /** 
     * typedef a shared pointer to this class
     */
    typedef boost::shared_ptr<WatcherGraph> WatcherGraphPtr;

    /** 
     * write a human readable version of the WatcherGraph class to the ostream given
     */
    std::ostream &operator<<(std::ostream &out, const WatcherGraph &watcherGraph);

}

#endif // WATCHER_GRAPH_H_WHAT_DO_VEGAN_ZOMBIES_EAT_____GRAINS__GRAINS
