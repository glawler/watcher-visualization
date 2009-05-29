#ifndef EDGE_DISPLAY_INFO_H
#define EDGE_DISPLAY_INFO_H

#include "libwatcher/watcherColors.h"
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

            bool loadLayer(const std::string &basePath); 
            void saveConfiguration(const std::string &basePath);

            watcher::event::Color color;
            int width; 
           
            bool flash;                         // To flash or not to flash, that is the question
            long long int flashInterval;        // flash every flashRate milliseconds
            long long int nextFlashUpdate;      // Next time to invert the colors. 
            bool isFlashed;                     // true if color is currently inverted.

            std::string label;                  // written next to the edge, not used often


        protected:

        private:

            DECLARE_LOGGER();
    };
}

#endif // EDGE_DISPLAY_INFO_H

