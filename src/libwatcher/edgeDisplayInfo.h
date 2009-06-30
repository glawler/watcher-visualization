#ifndef EDGE_DISPLAY_INFO_H
#define EDGE_DISPLAY_INFO_H

#include "watcherTypes.h"   // for Timestamp
#include "messageTypesAndVersions.h" // for GUILayer. 
#include "watcherColors.h"
#include "displayInfo.h"

namespace watcher
{
    /** 
     * @class EdgeDisplayInfo
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     *
     * Keep track of display information for an edge. 
     *
     */
    class EdgeDisplayInfo : 
        public DisplayInfo
    {
        public:
            EdgeDisplayInfo();
            virtual ~EdgeDisplayInfo(); 

            /** created from an edge related message */

            watcher::event::Color color;
            float width; 
           
            bool flash;                         // To flash or not to flash, that is the question
            Timestamp flashInterval;            // flash every flashRate milliseconds
            Timestamp nextFlashUpdate;          // Next time to invert the colors. 
            bool isFlashed;                     // true if color is currently inverted.

            std::string label;                  // written next to the edge, not used often
            std::string labelFont; 
            double labelPointSize; 
            watcher::event::Color labelColor;

            /** Can be loaded from cfg file ... */
            bool loadConfiguration(const watcher::event::GUILayer &layer); 

            /** Can be saved to cfg file ... */
            void saveConfiguration(); 

        protected:

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<EdgeDisplayInfo> EdgeDisplayInfoPtr; 
}

#endif // EDGE_DISPLAY_INFO_H

