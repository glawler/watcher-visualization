/**
 * @file displayInfo.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef DISPLAY_INFO_H
#define DISPLAY_INFO_H

#include <boost/utility.hpp>                // for noncopyable
#include "messageTypesAndVersions.h"        // for GUILayer
#include "logger.h"

namespace watcher
{
    /** 
     * @class DisplayInfo
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     *
     * Base class for keeping track of display information. Derived classes
     * will be read in their own display configuration and use it to keep 
     * track of the state of the display of the object. 
     *
     */
    class DisplayInfo : 
        public boost::noncopyable   // for now, may change later
    {
        public:

            /** 
             * DisplayInfo()
             *
             * @param category - the category or name of the resource in this layer. 
             * 
             * Creates default settings in the constructor. 
             */
            DisplayInfo(const std::string &category); 
            virtual ~DisplayInfo(); 

            /** 
             * Make sure the lower level of the cfg structure exists 
             * and pass back a path to it. Uses layerName and current 
             * value in categoryName to build the path. 
             */
            const std::string getBasePath(const watcher::event::GUILayer &layerName); 

            /**
             * Given a layer, load the configuration found there. Search is by 
             * member variable categoryName, so that must be set. 
             *
             * @param layer - layer data of the categoryName to load. 
             * @return bool - true if config loadded, false otherwise. 
             */
            virtual bool loadConfiguration(const watcher::event::GUILayer &layer) = 0; 

            /**
             * saveConfiguration()
             * This saves the current state of the display information to the 
             * singleton config instance. If you want the changes saved for this layer  
             * when the cfg file is written out, make sure to call this prior to doing so. 
             * The cfg info is saved to the layer specified in the loadConfiguration() call 
             * or the default layer, if loadConfiguration is never called. 
             */
            virtual void saveConfiguration() = 0; 

            /** The layer that this info is displayed on. */
            watcher::event::GUILayer layer; 

        protected:

            /** The 2nd level of the cfg path, usually "node" or "edge" */
            std::string categoryName;

            /** The "lowest" level in the cfg file */
            const std::string baseName; 

            /** The string that separates levels in teh cfg path */
            const std::string separator; 

        private:

            DECLARE_LOGGER();

            /** The "layer" level string */
            const std::string layerCfgId;
    };
}

#endif // DISPLAY_INFO_H
