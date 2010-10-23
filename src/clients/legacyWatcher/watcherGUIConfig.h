/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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
 * @file watcherConfig.h
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2010-10-22
 */
#ifndef WATHCER_CONFIG_H
#define WATHCER_CONFIG_H

#include <string>
#include <libwatcher/watcherColors.h>
#include <libwatcher/watcherTypes.h>
#include <declareLogger.h>

namespace watcher
{
    /** 
     * This class is really just a big struct around configuration information 
     * for a watcher GUI. Everything in this class can be loaded and save to a file
     * via loadConfiguration() and saveConfiguration().
     */
    class WatcherGUIConfig 
    {
        public:

            WatcherGUIConfig();
            virtual ~WatcherGUIConfig();

            virtual bool loadConfiguration(); 
            virtual bool saveConfiguration(); 

        public: 
            /** 
             * The public data. Can be updated during runtime
             * and saved to a file via saveConfiguration()
             */
            bool monochromeMode;
            bool threeDView;
            bool backgroundImage; 
            bool showGroundGrid;

            bool showWallTimeinStatusString;
            bool showPlaybackTimeInStatusString;
            bool showPlaybackRangeString;
            bool showVerboseStatusString;
            bool showDebugInfo;
            bool showStreamDescription;

            bool autorewind;
            bool messageStreamFiltering;

            float rgbaBGColors[4];

            size_t maxNodes;
            size_t maxLayers;

            int statusFontPointSize; 
            std::string statusFontName;
            watcher::Color hierarchyRingColor;
            watcher::Timestamp playbackStartTime;

            float scaleText;
            float scaleLine;
            float gpsScale; 
            float layerPadding;
            float antennaRadius; 
            float ghostLayerTransparency;

            struct ManetAdj
            {
                float angleX;
                float angleY;
                float angleZ;
                float scaleX;
                float scaleY;
                float scaleZ;
                float shiftX;
                float shiftY;
                float shiftZ;
            }; 

            ManetAdj manetAdj; 
            ManetAdj manetAdjInit;

            std::string serverName;

            /**
             * This is the set of layers from the configuration
             * at the start or runtime. These are not saved during 
             * saveConfig as this class does not keep track of
             * when layers are added or toggled active.
             * bool==true when the layer should be active at start, 
             * false otherwise.
             */
            typedef std::pair<std::string, bool> ActiveLayer;
            typedef std::vector<ActiveLayer> InitialLayers;
            InitialLayers initialLayers;

        protected:

            DECLARE_LOGGER(); 
            
        private:

    };
}

#endif /* WATHCER_CONFIG_H */
