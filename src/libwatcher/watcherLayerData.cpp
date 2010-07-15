#include <errno.h>
#include <algorithm>            // for fill_n

#include "labelMessage.h"
#include "watcherLayerData.h"

namespace watcher {
    using namespace std;

    INIT_LOGGER(WatcherLayerData, "WatcherLayerData"); 

    WatcherLayerData::WatcherLayerData() : numNodes(0), layerName(""), isActive(false), edges(NULL), edgeExpirations(NULL), nodeLabels(NULL), nodeLabelsMutexes(NULL), configured(false)
    {
    }
    WatcherLayerData::WatcherLayerData(const string &name, const size_t &nn) : 
        numNodes(nn), layerName(name), isActive(false), edges(NULL), edgeExpirations(NULL), nodeLabels(NULL), nodeLabelsMutexes(NULL), configured(false)
    {
        initialize(name, nn); 
    }
    // virtual 
    WatcherLayerData::~WatcherLayerData() 
    {
        deinitialize();
    }
    void WatcherLayerData::deinitialize() 
    {
        if (edges) {
            for (size_t i=0; i<numNodes; i++)  
                if (edges[i])  
                    delete [] edges[i];
            delete [] edges;
            edges=NULL;
        }
        if (edgeExpirations) {
            for (size_t i=0; i<numNodes; i++)  
                if (edgeExpirations[i])  
                    delete [] edgeExpirations[i];
            delete [] edgeExpirations;
            edgeExpirations=NULL;
        }
        if (nodeLabels) {
            delete [] nodeLabels;
            nodeLabels=NULL;
        }
        if (nodeLabelsMutexes) { 
            delete [] nodeLabelsMutexes;
            nodeLabelsMutexes=NULL;
        }

        numNodes=0;
        layerName="";
        isActive=false;
        configured=false;
    }
    void WatcherLayerData::initialize(const string &name, const size_t &nn)  
    {
        deinitialize();   // free memory if allocated.

        numNodes=nn;
        layerName=name;
        isActive=true;

        nodeLabels=new NodeLabels[numNodes];
        if (!nodeLabels) { 
            LOG_FATAL("Unable to allocate " << (sizeof(LabelDisplayInfo)*numNodes) << " bytes to store per node label display info data on layer "
                    << layerName << ": " << strerror(errno)); 
            exit(EXIT_FAILURE);
        }

        nodeLabelsMutexes=new WatcherLayerMutex[numNodes];
        if (!nodeLabelsMutexes) { 
            LOG_FATAL("Unable to allocate " << (sizeof(WatcherLayerMutex)*numNodes) << " bytes to store per node label mutexes on layer "
                    << layerName << ": " << strerror(errno)); 
            exit(EXIT_FAILURE);
        }
       
        edges=new EdgeType*[numNodes];
        if (!edges) { 
            LOG_FATAL("Unable to allocate " << (sizeof(EdgeType*)*numNodes) << " bytes for edge pointers: " << strerror(errno)); 
            exit(EXIT_FAILURE);
        }
        for (size_t i=0; i<numNodes; i++) { 
            edges[i]=new EdgeType[numNodes]; 
            if (!edges[i]) { 
                LOG_FATAL("Unable to allocate " << (sizeof(EdgeType)*numNodes) << " bytes for edge data: " << strerror(errno)); 
                exit(EXIT_FAILURE);
            }
            std::fill_n(edges[i], numNodes, 0); 
        }

        edgeExpirations=new EdgeExpirationsType*[numNodes]; 
        if (!edgeExpirations) { 
            LOG_FATAL("Unable to allocate " << (sizeof(EdgeExpirationsType*)*numNodes*numNodes) << " bytes to store per edge expiration pointers on layer "
                    << layerName << ": " << strerror(errno)); 
            exit(EXIT_FAILURE);
        }
        for (size_t i=0; i<numNodes; i++) { 
            edgeExpirations[i]=new EdgeExpirationsType[numNodes]; 
            if (!edgeExpirations[i]) { 
                LOG_FATAL("Unable to allocate " << (sizeof(EdgeExpirationsType)*numNodes) << " bytes for edge expiration data: " << strerror(errno)); 
                exit(EXIT_FAILURE);
            }
            std::fill_n(edgeExpirations[i], numNodes, watcher::Infinity); 
        }

        clear(); 

        if (!edgeDisplayInfo.loadConfiguration(layerName)) { 
            LOG_FATAL("Unable to load display information for edge layer " << name); 
            exit(EXIT_FAILURE); 
        }
        // The first thing we load from this layer is the edge. 
        // When we load something else from this layer, then 
        // the configuration will exist and this will return
        // true - so, only check this after the first thing
        // you load from a possibly new layer.
        configured=edgeDisplayInfo.configurationExisted();

        referenceLabelDisplayInfo.loadConfiguration(layerName); 
        referenceFloatingLabelDisplayInfo.loadConfiguration(layerName); 
    }
    void WatcherLayerData::clear() 
    {
        for (size_t i=0; i<numNodes; i++)  
            std::fill_n(edges[i], numNodes, 0); 
        for (size_t i=0; i<numNodes; i++)  
            std::fill_n(edgeExpirations[i], numNodes, watcher::Infinity); 

        {
            UpgradeLock lock(floatingLabelsMutex); 
            WriteLock writeLock(lock); 
            floatingLabels.clear(); 
        }

        for (size_t n=0; n<numNodes; n++) {
            UpgradeLock lock(nodeLabelsMutexes[n]); 
            WriteLock writeLock(lock); 
            nodeLabels[n].clear();
        }
    }
    bool WatcherLayerData::addRemoveFloatingLabel(const event::LabelMessagePtr &m, const bool &timeForward)
    {
        FloatingLabelDisplayInfo fldi(referenceFloatingLabelDisplayInfo);  // load default label info
        fldi.initialize(m);       // update with specific settings from this message (label text, etc)
        fldi.labelText=m->label;
        UpgradeLock lock(floatingLabelsMutex); 
        WriteLock writeLock(lock); 
        if (m->addLabel) { 
            LOG_DEBUG("Adding floating label: " << *m); 
            if (fldi.expiration!=Infinity) {
                if (timeForward) 
                    fldi.expiration=m->timestamp+m->expiration;
                else 
                    fldi.expiration=m->timestamp-m->expiration;
            }
            floatingLabels.insert(fldi); 
        }
        else 
            floatingLabels.erase(fldi); 
        
        return true;
    }

    bool WatcherLayerData::addRemoveLabel(const event::LabelMessagePtr &m, const bool &timeForward, const size_t &nodeNum)
    {
        LabelDisplayInfo ldi(referenceLabelDisplayInfo);  // load default label info
        ldi.initialize(m);       // update with specific settings from this message (label text, etc)
        UpgradeLock lock(nodeLabelsMutexes[nodeNum]); 
        WriteLock writeLock(lock); 
        if (m->addLabel) { 
            LOG_DEBUG("Adding label: " << *m); 
            if (ldi.expiration!=Infinity) {
                if (timeForward) 
                    ldi.expiration=m->timestamp+m->expiration;
                else 
                    ldi.expiration=m->timestamp-m->expiration;
            }
            nodeLabels[nodeNum].insert(ldi); 
        }
        else 
            nodeLabels[nodeNum].erase(ldi); 
        
        return true;
    }

    bool WatcherLayerData::saveConfiguration(void)
    {
        referenceLabelDisplayInfo.saveConfiguration(); 
        referenceFloatingLabelDisplayInfo.saveConfiguration(); 
        edgeDisplayInfo.saveConfiguration(); 
        return true;
    }
} // end of namespace watcher

