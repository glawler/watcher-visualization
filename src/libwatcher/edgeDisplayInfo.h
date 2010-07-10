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
 * @file edgeDisplayInfo.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef EDGE_DISPLAY_INFO_H
#define EDGE_DISPLAY_INFO_H

#include "watcherTypes.h"   // for Timestamp
#include "messageTypesAndVersions.h" // for GUILayer. 
#include "watcherColors.h"
#include "displayInfo.h"

namespace watcher
{
    /** 
     * Keep track of display information for an edge. 
     * Created from an edge related message.
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     */
    class EdgeDisplayInfo : 
        public DisplayInfo
    {
        public:
            EdgeDisplayInfo();
            virtual ~EdgeDisplayInfo(); 

            watcher::Color color;        ///< Color of the edge line
            float width;                        ///< width of the edge line
           
            bool flash;                         ///< To flash or not to flash, that is the question
            Timestamp flashInterval;            ///< flash every flashRate milliseconds
            Timestamp nextFlashUpdate;          ///< Next time to invert the colors. 
            bool isFlashed;                     ///< true if color is currently inverted.

            std::string label;                  ///< written next to the edge, not used often
            std::string labelFont; 
            double labelPointSize; 
            watcher::Color labelColor;

            bool loadConfiguration(const watcher::event::GUILayer &layer); 

            void saveConfiguration() const;

        protected:

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<EdgeDisplayInfo> EdgeDisplayInfoPtr; 
}

#endif // EDGE_DISPLAY_INFO_H


