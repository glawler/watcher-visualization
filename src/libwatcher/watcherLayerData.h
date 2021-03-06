#ifndef WATCHER_LAYER_DATA_H
#define WATCHER_LAYER_DATA_H

#include <string>
#include <set>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "logger.h"
#include "edgeDisplayInfo.h"
#include "labelDisplayInfo.h"
#include "floatingLabelDisplayInfo.h"

namespace watcher {
    /**
     * Holds a single layer's data. A layer is a set of edges (and how to display them) and a set of labels
     * attached to both nodes and edges (and how to display them). Each label has it's own display info, 
     * that is all labels can look different. There is only one display info for all edges on a layer though.
     *
     * This class limits the number of nodes to a finite amount.  It must know how many nodes there are at
     * instantiation time. This allows O(1) edge insert/remove/update which is needed with very large 
     * numbers of nodes. 
     * 
     */
    class WatcherLayerData { 
        public:

            WatcherLayerData(); 
            WatcherLayerData(const std::string &layerName, const size_t &nodeNum); 
            virtual ~WatcherLayerData();

            /** 
             * like a a constructor, malloc all memory, then initialize all values 
             * from ambient SingletonConfig instance. 
             */
            void initialize(const std::string &name, const size_t &nn);

            /**
             * edges is a 2d array of bools. If edges[a][b]!=0, there is an 
             * edge between nodes a and b. 
             */
            typedef unsigned char EdgeType;
            EdgeType **edges;

            /**
             * Keep track of expiration time per edge data. Indexes mirror edges 2d array.
             *
             * Note: do not use memset() on this array! Timestamp is a longlong!
             */
            typedef Timestamp EdgeExpirationsType;
            EdgeExpirationsType **edgeExpirations;

            /** 
             * How to display the edge. Only one display type per layer. 
             * On instanciation, the class will load the display info based on the
             * layer's name passed to the c'tor. 
             */
            EdgeDisplayInfo edgeDisplayInfo;

            /** Small typedef wrappers around or choice of mutex type. */
            typedef boost::shared_mutex WatcherLayerMutex;
            typedef boost::shared_lock<WatcherLayerMutex> ReadLock;
            typedef boost::upgrade_lock<WatcherLayerMutex> UpgradeLock;
            typedef boost::upgrade_to_unique_lock<WatcherLayerMutex> WriteLock;

            bool addRemoveFloatingLabel(const event::LabelMessagePtr &m, const bool &timeForward);
            bool addRemoveLabel(const event::LabelMessagePtr &m, const bool &timeForward, const size_t &nodeNum);
            bool addRemoveEdgeLabel(const event::LabelMessagePtr &m, const bool &timeForward, const size_t &n1, const size_t &n2);

            /** 
             * nodeLabels is a fixed size array (numNodes long) of label display infos, 
             * i.e. each node can have up to numNodeLabels labels attached to it.
             *
             * If nodeLabels[i] is not empty, there are one or more labels on the 
             * node a in this layer.
             *
             * The only valid operation on this data structure using this class is to 
             * iterate over the list, read-only. Labels are inserted and removed via
             * updateGraph() only. Before iterating over any label list, the user of this 
             * class must lock access, then unlock when finished. There are N label lists, 
             * one for each node, thus there are N mutexes, one for each list. 
             *
             * access as all node labels for the layer, "layer", as follows:
             *   [GTL - IOU one example]
             */
            typedef std::set<LabelDisplayInfo> NodeLabels;
            NodeLabels *nodeLabels; 
            WatcherLayerMutex *nodeLabelsMutexes;

            /** 
             * floatingLabels is a set of label display info pointers.
             *
             * Usage is simular to nodeLabels, but there is a single list of floating labels per layer, 
             * instead of one per node. 
             */
            typedef std::set<FloatingLabelDisplayInfo> FloatingLabels;
            FloatingLabels floatingLabels;
            WatcherLayerMutex floatingLabelsMutex;

            /**
             * labels attached to edges. If edgeLabels[a][b] is not empty, there are one or more lables
             * attached to the edge. 
             */
            typedef std::set<LabelDisplayInfo> EdgeLabels;
            EdgeLabels **edgeLabels;
            WatcherLayerMutex **edgeLabelsMutexes;

            /** 
             * clear all data from the layer. Do not mark as inActive or deallocate any memory.
             */
            void clear(); 

            /**
             * is this layer active?
             */
            bool isActive;

            bool saveConfiguration(void); 

            /** The name of this layer */
            std::string layerName;

            /*
             * Set to false if this layer did not exist in the current configuration, 
             * true otherwise. 
             */
            bool configured;
   
            /** 
             * This label is loaded from the config file, then used to 
             * initialize all subsequently created LabelDisplayInfos. 
             * This is so we only load the configuration once per layer
             * as it's an expensive operation. This reference can
             * be used for both floating and non-floating labels.
             */
            LabelDisplayInfo referenceLabelDisplayInfo; 
            FloatingLabelDisplayInfo referenceFloatingLabelDisplayInfo; 

        protected:
        private:

            DECLARE_LOGGER();
            
            /** max numner of nodes supported by this layer. */
            size_t numNodes; 

            /** free all memory, set all values to zero/empty */
            void deinitialize();

            /** Not implemenented. */
            WatcherLayerData(const WatcherLayerData &noCopiesThanks); 

            /** Not implemenented. */
            WatcherLayerData &operator=(const WatcherLayerData &noCopiesThanks); 

            boost::mutex initMutex;

    }; // end of class
}


#endif /* WATCHER_LAYER_DATA_H */
