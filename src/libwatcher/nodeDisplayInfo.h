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
 * @file nodeDisplayInfo.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef NODE_DISPLAY_INFO_H
#define NODE_DISPLAY_INFO_H

#include <string>

#include "watcherTypes.h"               // for NodeIdentifier
#include "messageTypesAndVersions.h"    // for GUILayer
#include "watcherColors.h"
#include "colors.h"
#include "displayInfo.h"

namespace watcher
{
    /** 
     * Keep track of display information for a specific or default node.
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     */
    class NodeDisplayInfo : 
        public DisplayInfo
    {
        public:
            NodeDisplayInfo();

            virtual ~NodeDisplayInfo(); 

            /** The node's ID. */
            NodeIdentifier nodeId;
            
            enum NodeShape { CIRCLE=0, SQUARE, TRIANGLE, TORUS, TEAPOT };
            static std::string nodeShapeToString(const NodeDisplayInfo::NodeShape &shape);
            static NodeShape stringToNodeShape(const std::string &shape);
            /** Item's shape */
            NodeShape shape;

            bool sparkle; ///< display effects for a node - may not all be supported in GUI
            bool spin; ///< display effects for a node - may not all be supported in GUI
            bool flash; ///< display effects for a node - may not all be supported in GUI

            /** how big is the node from "normal" */
            float size;

            /* Spin data. Used when spin is enabled (spin==true) */
            int spinTimeout;                ///< update rotatation every spinTimeout milliseconds
            float spinIncrement;            ///< Amount of spin per timeout period
            long long int nextSpinUpdate;   ///< epoch milliseconds, same as destime.
            float spinRotation_x;           ///< cur rotation for x plane
            float spinRotation_y;           ///< cur rotation for y plane
            float spinRotation_z;           ///< cur rotation for d plane, no, wait z plane.

            /*
             * Flash data. Flash is done by inverting the color of the thing every
             * x milliseconds.
             */
            long long int flashInterval;        ///< flash every flashRate milliseconds
            long long int nextFlashUpdate;      ///< Next time to invert the colors. 
            bool isFlashed;                     ///< true if color is currently inverted.

            /** 
             * Allow for different types of default labels.  
             * If not FREE_FORM, what you think you'll get is probably 
             * correct. If FREE_FORM, it'll just use the value in 'label'.
             */
            enum LabelDefault { FOUR_OCTETS, THREE_OCTETS, TWO_OCTETS, LAST_OCTET, HOSTNAME, FREE_FORM }; 
            static std::string labelDefault2String(const NodeDisplayInfo::LabelDefault &labDef);
            std::string label;
            std::string labelFont;
            double labelPointSize;
            watcher::Color labelColor;

            watcher::Color color; 

            /* The configuration interface. */

            /**
             * Given a Config path, load the Node found there. 
             *
             * @param layer target Watcher layer
             *
             * @param nid if given the configuration for that specific node, if found, 
             * will be loaded. If not found, the default node config for this layer will be loaded. 
             * (Note - this will overwrite any existing configuration this instance may have, including
             * resetting the nodeId). If this parameter is not specfied, the default configuration will 
             * be loaded. 
             *
             * @retval bool true if config loadded
             * @retval false otherwise. 
             */
            bool loadConfiguration(const watcher::event::GUILayer &layer, const NodeIdentifier &nid); 

            bool loadConfiguration(const watcher::event::GUILayer &layer); 

            void saveConfiguration(); 

        protected:

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<NodeDisplayInfo> NodeDisplayInfoPtr; 
}

#endif // NODE_DISPLAY_INFO_H

