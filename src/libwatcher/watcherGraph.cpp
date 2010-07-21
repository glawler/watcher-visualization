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

#include <boost/pointer_cast.hpp>       // for dynamic_pointer_cast<>
#include <boost/foreach.hpp>

#include "watcherGraph.h"
#include "watcherTypes.h"

#include "messageStreamFilter.h"
#include "logger.h"

#include "connectivityMessage.h"
#include "gpsMessage.h"
#include "nodeStatusMessage.h"
#include "edgeMessage.h"
#include "colorMessage.h"
#include "singletonConfig.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;

INIT_LOGGER(WatcherGraph, "WatcherGraph");

WatcherGraph::WatcherGraph(const size_t &maxNodes, const size_t &maxLayers) : 
    maxNumNodes(maxNodes), maxNumLayers(maxLayers), timeForward(true), numValidNodes(0), numValidLayers(0)
{
    layers=new WatcherLayerData[maxLayers];
    if (!layers) { 
        LOG_FATAL("Unable to allocate " << (sizeof(WatcherLayerData)*maxNumNodes) << " bytes to store layer data: " << strerror(errno)); 
        exit(EXIT_FAILURE);
    }

    nodes=new NodeDisplayInfo[maxNumNodes]; 
    if (!nodes) { 
        LOG_FATAL("Unable to allocate " << (sizeof(NodeDisplayInfo)*maxNumNodes) << " bytes for node data info: " << strerror(errno)); 
        exit(EXIT_FAILURE);
    }

    index2nidMap=new unsigned int[maxNumNodes];
    if (!index2nidMap) { 
        LOG_FATAL("Unable to allocate " << (sizeof(size_t)*maxNumNodes) << " bytes to store index to nid map data: " << strerror(errno)); 
        exit(EXIT_FAILURE);
    }
    memset(index2nidMap, 0, sizeof(index2nidMap[0])*maxNumNodes); 

    layers[0].initialize(PHYSICAL_LAYER, maxNumNodes); 
    numValidLayers++; 
}

// virtual 
WatcherGraph::~WatcherGraph() 
{ 
    if (nodes) 
        delete [] nodes;
    nodes=NULL;
    numValidNodes=0;

    if (layers)
        delete [] layers;
    layers=NULL;
    numValidLayers=0;

    if (index2nidMap) 
        delete [] index2nidMap;
    index2nidMap=NULL;

}

bool WatcherGraph::layerExists(const std::string &name) const 
{
    return layerIndexMap.find(name)!=layerIndexMap.end();
}
size_t WatcherGraph::name2LayerIndex(const std::string &name) 
{
    Name2LayerIndexMap::const_iterator layer=layerIndexMap.find(name);
    if (layer==layerIndexMap.end()) { 
        if (numValidLayers==maxNumLayers) { // adding this layer would go past static layers array.
            LOG_FATAL("Requested access to layer " << name << " which doesn't exist. Creating the layer would cause us to go past the hard limit of "
                      << maxNumLayers << " specified in the configuration file. Please increase this number and re-run."); 
            LOG_FATAL("Current layers:"); 
            for (size_t l=0; l<numValidLayers; l++) 
                LOG_FATAL("\t" << layers[l].layerName); 
            exit(EXIT_FAILURE);
        }
        layers[numValidLayers].initialize(name, maxNumNodes);  // will exit() on error
        layerIndexMap[name]=numValidLayers; 
        numValidLayers++;
        return numValidLayers-1;
    }
    return layer->second;
}

void WatcherGraph::clear() 
{
    // set nodes as inactive.
    // for (size_t i=0; i<numValidNodes; i++) 
    //     nodes[i].isActive=false;        // may want to actually clear the data inside nodes?

    // clear layers.
    for (size_t i=0; i<numValidLayers; i++) 
        layers[i].clear();
}

void WatcherGraph::setTimeDirectionForward(bool forward)
{
    //
    // If we're changing direction - clear everything but the nodes on the assumption
    // that it doesn't exist yet. This may be a false assumption for things with 
    // infinite duration - or it may not, depending. To be on the safe side (or the 
    // easier side in any case), just clear them. (Note nodes always exist.) 
    //
    if (timeForward!=forward) 
        clear();
    timeForward=forward;
}

size_t WatcherGraph::nid2Index(const NodeIdentifier &nid)
{
    // This needs to be as fast as possible as we do this (maybe) multiple times per message.
    // And with large testbeds, this can be 1000s of times a second.

    // Check for address type taken out for performance
    // if (!nid.is_v4()) {
    //     LOG_FATAL("Only ipv4 addresses supported");
    //     exit(EXIT_FAILURE);
    // }

    unsigned int addr=nid.to_v4().to_ulong();
    NID2IndexMap::iterator i=nid2IndexMap.find(addr);  // O(1) when using std::unordered_map, O(log(n)) with std::map
    if (i==nid2IndexMap.end()) {
        // We can take this check out if we don't trust our users. But this code block should only happen
        // once per node, so we are less concerned about performance here. 
        if (numValidNodes==maxNumNodes) {
            LOG_FATAL("Watcher has seen more nodes than it was told it would see, " << numValidNodes << " unable to continue.");
            LOG_FATAL("Known nodes:");
            BOOST_FOREACH(NID2IndexMap::value_type &a, nid2IndexMap) { 
                struct in_addr addr = { a.first }; 
                LOG_FATAL("\t" << inet_ntoa(addr));
            }
            exit(EXIT_FAILURE);  // this could be handled more gracefully.
        }
        nid2IndexMap[addr]=numValidNodes;       // creates entry for addr
        index2nidMap[numValidNodes]=addr;       // writes into exising new'd memory   
        nodes[numValidNodes].loadConfiguration(PHYSICAL_LAYER, nid); // nodes are always on the physical layer. (for now). 
        numValidNodes++;
        LOG_INFO("Loaded configuration for node " << nid << ". This is node number " << numValidNodes-1); 
        return numValidNodes-1;
    }
    return i->second;
}

unsigned int WatcherGraph::index2Nid(const size_t index) const
{
    // we assume the node has been seen and is valid. 
    // this may be a bad assumption. We can check the size of the nid2IndexMap to 
    // see if it is out of range, but what then? throw an exception? exit()? 
    return index2nidMap[index];
}
    
bool WatcherGraph::addNodeNeighbors(const ConnectivityMessagePtr &message)
{
    size_t l=name2LayerIndex(message->layer); 
    size_t a=nid2Index(message->fromNodeID); 

    LOG_DEBUG("Clearing neighbors for node " << message->fromNodeID << " (" << a << ") on layer " << layers[l].layerName << " (" << l << ")");

    // clear existing neighbors
    std::fill_n(layers[l].edges[a], maxNumNodes, 0); 

    // all new edges don't expire as the messages format does not support it. Expired edges use edgeMessge.
    std::fill_n(layers[l].edgeExpirations[a], maxNumNodes, watcher::Infinity); 

    // toggle new neighbors
    BOOST_FOREACH(const ConnectivityMessage::NeighborList::value_type &nid, message->neighbors) 
        layers[l].edges[a][nid2Index(nid)]=1;

    return true;
}

bool WatcherGraph::addEdge(const EdgeMessagePtr &message) 
{
    // 
    // do we want a callback here to update edge locations
    // info as well? We'd need to add storage for endpoints
    // or other trig data to edgeDisplayInfo (which *will* speed 
    // drawing the edges so maybe thats a good idea).
    //
    size_t a=nid2Index(message->node1);
    size_t b=nid2Index(message->node2);
    size_t l=name2LayerIndex(message->layer);

    layers[l].edges[a][b]=message->addEdge;

    if (!message->addEdge) {
        layers[l].edgeExpirations[a][b]=watcher::Infinity;
        if (message->bidirectional) { 
            layers[l].edges[b][a]=false;
            layers[l].edgeExpirations[b][a]=watcher::Infinity;
        }
    }

    if (message->expiration!=Infinity) {
        // Timestamp oldExp=layers[l].edgeExpirations[a][b];
        if (timeForward) 
            layers[l].edgeExpirations[a][b]=message->timestamp+message->expiration;  
        else 
            layers[l].edgeExpirations[a][b]=message->timestamp-message->expiration;  
        // LOG_DEBUG("Set edge expiration. Was: " << oldExp << " now: " << theGraph[ei.first].expiration);
    }

    if (layers[l].edges[a][b] && message->middleLabel && !message->middleLabel->label.empty()) 
        layers[l].addRemoveEdgeLabel(message->middleLabel, timeForward, a, b); 

    if (nodes[a].isActive && message->node1Label && !message->node1Label->label.empty()) 
        layers[l].addRemoveLabel(message->node1Label, timeForward, a); 

    if (nodes[b].isActive && message->node2Label && !message->node2Label->label.empty()) 
        layers[l].addRemoveLabel(message->node2Label, timeForward, b); 

    // if you want dynamic colors and widths (i.e. controlled by the test nodes at run time), 
    // uncomment the following. Is it worth doing this copy for every edge message we get
    // when the vast majority of the time the edges do not change attributes 
    // dynamically?
    // layers[l].edgeDisplayInfo.color=message->color;
    // layers[l].edgeDisplayInfo.width=message->width;

    if (message->bidirectional) { 
        layers[l].edges[b][a]=true;
        // Timestamp oldExp=layers[l].edgeExpirations[b][a];
        if (timeForward) 
            layers[l].edgeExpirations[b][a]=message->timestamp+message->expiration;  
        else 
            layers[l].edgeExpirations[b][a]=message->timestamp-message->expiration;  

        if (layers[l].edges[b][a] && message->middleLabel && !message->middleLabel->label.empty()) 
            layers[l].addRemoveEdgeLabel(message->middleLabel, timeForward, b, a); 
    }

    return true;
}

void WatcherGraph::doMaintanence(const watcher::Timestamp &ts)
{
    Timestamp now=ts==0?watcher::getCurrentTime():ts;
    for (size_t l=0; l!=numValidLayers; l++)  {
        if (!layers[l].isActive) 
            continue;
        {
            WatcherLayerData::UpgradeLock lock(layers[l].floatingLabelsMutex); 
            WatcherLayerData::WriteLock writeLock(lock); 
            for (WatcherLayerData::FloatingLabels::iterator label=layers[l].floatingLabels.begin(); label!=layers[l].floatingLabels.end(); ) {
                if (label->expiration!=Infinity && (timeForward ? (now > label->expiration) : (now < label->expiration))) 
                    layers[l].floatingLabels.erase(label++); 
                else
                    ++label;
            }
        }
        for (size_t n=0; n<numValidNodes; n++)  {
            if (!nodes[n].isActive) 
                continue;
            {
                for (size_t n2=0; n2<numValidNodes; n2++)  {
                    if (nodes[n2].isActive) {
                        if (layers[l].edges[n][n2] && layers[l].edgeExpirations[n][n2]!=Infinity) {
                            if ((timeForward ? (now > layers[l].edgeExpirations[n][n2]) : (now < layers[l].edgeExpirations[n][n2]))) { 
                                layers[l].edges[n][n2]=0; 
                                layers[l].edgeExpirations[n][n2]=Infinity; 
                            }
                        }
                    }
                    WatcherLayerData::UpgradeLock lock(layers[l].edgeLabelsMutexes[n][n2]);
                    WatcherLayerData::WriteLock writeLock(lock);
                    for (WatcherLayerData::EdgeLabels::iterator label=layers[l].edgeLabels[n][n2].begin(); label!=layers[l].edgeLabels[n][n2].end(); ) {
                        if (label->expiration!=Infinity && (timeForward ? (now > label->expiration) : (now < label->expiration))) 
                            layers[l].edgeLabels[n][n2].erase(label++); 
                        else
                            ++label;
                    }
                }
                WatcherLayerData::UpgradeLock lock(layers[l].nodeLabelsMutexes[n]); 
                WatcherLayerData::WriteLock writeLock(lock); 
                for (WatcherLayerData::NodeLabels::iterator label=layers[l].nodeLabels[n].begin(); label!=layers[l].nodeLabels[n].end(); ) {
                    if (label->expiration!=Infinity && (timeForward ? (now > label->expiration) : (now < label->expiration))) 
                        layers[l].nodeLabels[n].erase(label++); 
                    else
                        ++label;
                }
            }
        }
    }
    // May add these back as toggable functionality for use in smaller test bed scenarios
    // removed: support for flashing
    // removed: support for spinning
    // removed: support for labels on edges
}

bool WatcherGraph::updateNodeLocation(const GPSMessagePtr &message)
{
    LOG_DEBUG("Updating GPS information for node " << message->fromNodeID); 
    size_t index=nid2Index(message->fromNodeID); 
    nodes[index].x=message->x;
    nodes[index].y=message->y;
    nodes[index].z=message->z;
    if (locationTranslationFunction) 
        locationTranslationFunction(nodes[index].x, nodes[index].y, nodes[index].z, message->dataFormat); 
    return true;
}

bool WatcherGraph::updateNodeStatus(const NodeStatusMessagePtr &message)
{
    LOG_DEBUG("Updating connection status for node " << message->fromNodeID); 
    size_t index=nid2Index(message->fromNodeID); 
    nodes[index].isConnected=message->event==NodeStatusMessage::connect ? true : false;
    return true;
}

bool WatcherGraph::updateNodeProperties(const NodePropertiesMessagePtr &message)
{
    LOG_DEBUG("Updating properties for node " << message->fromNodeID); 
    
    // create the layer if needed. This is odd though as the layer is empty and only 
    // ever modifies node settings. But it still needs to exist, so a GUI can toggle it 
    // on and off, etc. 
    name2LayerIndex(message->layer); 
    
    size_t index=nid2Index(message->fromNodeID); 
    if (message->useColor)
        nodes[index].color=message->color; 
    if (message->useShape)
        nodes[index].shape=message->shape;
    if (message->size>=0.0)
        nodes[index].size=message->size;
    if (message->displayEffects.size()) {
        BOOST_FOREACH(NodePropertiesMessage::DisplayEffect &e, message->displayEffects)
            switch(e) {
                case NodePropertiesMessage::SPIN: nodes[index].spin=!nodes[index].spin; break;
                case NodePropertiesMessage::FLASH: nodes[index].flash=!nodes[index].flash; break;
                case NodePropertiesMessage::SPARKLE: nodes[index].sparkle=!nodes[index].sparkle; break;
            }
    }
    if (message->nodeProperties.size()) 
        nodes[index].nodeProperties=message->nodeProperties;

    return true;
}

bool WatcherGraph::updateNodeColor(const ColorMessagePtr &message)
{
    LOG_DEBUG("Updating color information for node " << message->fromNodeID); 
    size_t index=nid2Index(message->fromNodeID); 
    nodes[index].color=message->color; 
    if (message->flashPeriod) {
        nodes[index].flash=true; 
        nodes[index].flashInterval=message->flashPeriod; 
    }
    return true;
}

bool WatcherGraph::addRemoveLabel(const LabelMessagePtr &message)
{
    size_t l=name2LayerIndex(message->layer); 
    if (message->lat && message->lng) 
        return layers[l].addRemoveFloatingLabel(message, timeForward); 
    else
        return layers[l].addRemoveLabel(message, timeForward, nid2Index(message->fromNodeID)); 
}
// virtual 
std::ostream &WatcherGraph::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    out << "WatcherGraph::toStream() not implemented, sorry.";
    TRACE_EXIT();
    return out;
}

bool WatcherGraph::updateGraph(const MessagePtr &message)
{
    TRACE_ENTER();
    bool retVal=true;

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
            retVal=addRemoveLabel(dynamic_pointer_cast<LabelMessage>(message));
            break;
        case COLOR_MESSAGE_TYPE:
            retVal=updateNodeColor(dynamic_pointer_cast<ColorMessage>(message));
            break;
        case NODE_PROPERTIES_MESSAGE_TYPE:
            retVal=updateNodeProperties(dynamic_pointer_cast<NodePropertiesMessage>(message));
            break;
        default:
            retVal=false;
            break;
    }

    TRACE_EXIT_RET(retVal);
    return retVal;
}
bool WatcherGraph::saveConfiguration(void)
{
    for (size_t n=0; n!=numValidNodes; n++)  
        nodes[n].saveConfiguration(); 
    for (size_t l=0; l!=numValidLayers; l++)  
        layers[l].saveConfiguration(); 

    // Graph remembers which layers there are and which are active.
    if (numValidLayers) { 
        SingletonConfig::lock(); 
        libconfig::Config &cfg=SingletonConfig::instance();
        string prop("layers"); 
        if (!cfg.getRoot().exists(prop))
            cfg.getRoot().add(prop, libconfig::Setting::TypeGroup);
        libconfig::Setting &layerCfg=cfg.lookup(prop); 
        for (size_t l=0; l!=numValidLayers; l++)  {
            if (!layerCfg.exists(layers[l].layerName))
                layerCfg.add(layers[l].layerName, libconfig::Setting::TypeBoolean);
            layerCfg[layers[l].layerName]=layers[l].isActive;
        }
        SingletonConfig::unlock();
    }

    return true;
}
std::ostream &watcher::operator<<(std::ostream &out, const watcher::WatcherGraph &watcherGraph)
{
    watcherGraph.operator<<(out);
    return out;
}



