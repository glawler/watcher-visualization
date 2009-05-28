#ifndef NODE_DISPLAY_INFO_H
#define NODE_DISPLAY_INFO_H

#include <string>

#include "libwatcher/watcherTypes.h"  // for NodeIdentifier
#include "libwatcher/watcherColors.h"
#include "displayInfo.h"

namespace watcher
{
    /** 
     * @class NodeDisplayInfo
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     *
     * Keep track of display information for a specific or default node.
     */
    class NodeDisplayInfo : 
        public DisplayInfo
    {
        public:
            /**
             * 
             */
            NodeDisplayInfo();

            virtual ~NodeDisplayInfo(); 

            bool loadLayer(const std::string &basePath);

            /**
             * saveLayer()
             * This saves the current state of the display information to the 
             * singleton config instance. If you want the changes saved for this layer  
             * when the cfg file is written out, make sure to call this prior to doing so. 
             */
            void saveConfiguration(const std::string &basePath); 

            /** The node's ID. */
            NodeIdentifier nodeId;
            
            /** Item's shape */
            enum NodeShape { CIRCLE=0, SQUARE, TRIANGLE, TORUS, TEAPOT };
            static std::string nodeShapeToString(const NodeDisplayInfo::NodeShape &shape);
            static NodeShape stringToNodeShape(const std::string &shape);
            NodeShape shape;

            /** display effects for a node - may not all be supported in GUI */
            bool sparkle;
            bool spin;
            bool flash;

            /** how big is the node from "normal" */
            float size;

            /**
             * Spin data. Used when spin is enabled (spin==true)
             */
            int spinTimeout;                // update rotatation every spinTimeout milliseconds
            float spinIncrement;            // Amount of spin per timeout period
            long long int nextSpinUpdate;   // epoch milliseconds, same as destime.
            float spinRotation_x;           // cur rotation for x plane
            float spinRotation_y;           // cur rotation for y plane
            float spinRotation_z;           // cur rotation for d plane, no, wait z plane.

            /**
             * Flash data. Flash is done by inverting the color of the thing every
             * x milliseconds.
             */
            long long int flashInterval;        // flash every flashRate milliseconds
            long long int nextFlashUpdate;      // Next time to invert the colors. 
            bool isFlashed;                     // true if color is currently inverted.

            /** 
             * Allow for different types of default labels.  
             * If not FREE_FORM, what you think you'll get is probably 
             * correct. If FREE_FORM, it'll just use the value in 'label'.
             */
            enum LabelDefault { FOUR_OCTETS, THREE_OCTETS, TWO_OCTETS, LAST_OCTET, HOSTNAME, FREE_FORM }; 
            static std::string labelDefault2String(const NodeDisplayInfo::LabelDefault &labDef);
            std::string label;

            watcher::event::Color color; 

        protected:

        private:

            DECLARE_LOGGER();
    };
}

#endif // NODE_DISPLAY_INFO_H

