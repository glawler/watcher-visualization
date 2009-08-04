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

            watcher::event::Color color;        ///< Color of the edge line
            float width;                        ///< width of the edge line
           
            bool flash;                         ///< To flash or not to flash, that is the question
            Timestamp flashInterval;            ///< flash every flashRate milliseconds
            Timestamp nextFlashUpdate;          ///< Next time to invert the colors. 
            bool isFlashed;                     ///< true if color is currently inverted.

            std::string label;                  ///< written next to the edge, not used often
            std::string labelFont; 
            double labelPointSize; 
            watcher::event::Color labelColor;

            bool loadConfiguration(const watcher::event::GUILayer &layer); 

            void saveConfiguration(); 

        protected:

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<EdgeDisplayInfo> EdgeDisplayInfoPtr; 
}

#endif // EDGE_DISPLAY_INFO_H

