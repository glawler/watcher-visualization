#ifndef LAYER_DISPLAY_INFO_H
#define LAYER_DISPLAY_INFO_H

#include "libwatcher/messageTypesAndVersions.h" // for GUILayer
#include "displayInfo.h"
#include "nodeDisplayInfo.h"
#include "edgeDisplayInfo.h"
#include "labelDisplayInfo.h"

namespace watcher
{
    /** 
     * @class LayerDisplayInfo
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     *
     * Keep track of display information for a Layer.
     *
     */
    class LayerDisplayInfo : 
        public DisplayInfo
    {
        public:
            LayerDisplayInfo(); 
            virtual ~LayerDisplayInfo(); 
       
            /**
             * loadLayer(const string &layerName)
             * Load the layer corresponding to the name, into this class instance. 
             * The configuration information comes from the cfg file, or if it doesn't 
             * exist, then defaults are loaded (and written back to the singletonConfig). 
             *
             * @prereq singletonConfig has been initialized with a cfg file. 
             *
             * @param theLayer - the layer information to load/create.
             */
            bool loadLayer(const watcher::event::GUILayer &theLayer); 

            /**
             * saveLayer()
             * This saves the current state of the display information to the 
             * singleton config instance. If you want the changes saved for this layer  
             * when the cfg file is written out, make sure to call this prior to doing so. 
             */
            void saveLayer(); 

            /**
             * saveLayerAs()
             * Same as saveLayer, but you specify which layer you'd like this saved as.
             * @param layer - the layer to save as. 
             */
            void saveLayerAs(const watcher::event::GUILayer &layer);

            /** The label information for this layer. It is the same for all labels on this layer */
            LabelDisplayInfo labelDisplayInfo; 

            /** The edge information for this layer. It is the same for all edges on this layer */
            EdgeDisplayInfo edgeDisplayInfo; 

            // typedef std::map<NodeIdentifier, NodeDisplayInfo> NodeDisplayInfoMap;
            /** The node information for this layer. May be different for each node. */
            // NodeDisplayInfoMap nodeDisplayInfo; 
            NodeDisplayInfo defaultNodeDisplayInfo;

        protected:

        private:
            DECLARE_LOGGER();

            /** libconfig base path to all layers */
            const std::string basePath; 

            /** load the default data */
            bool loadDefault(); 

            /** the name of this layer */
            watcher::event::GUILayer theLayer; 

    };
}

#endif // LAYER_DISPLAY_INFO_H

