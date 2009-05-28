#ifndef LABEL_DISPLAY_INFO_H
#define LABEL_DISPLAY_INFO_H

#include <string>
#include "libwatcher/watcherColors.h"
#include "displayInfo.h"

namespace watcher
{
    /** 
     * @class LabelDisplayInfo
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     *
     * Keep track of display information for a Label.
     *
     */
    class LabelDisplayInfo : 
        public DisplayInfo
    {
        public:
            LabelDisplayInfo();
            virtual ~LabelDisplayInfo(); 

            bool loadLayer(const std::string &basePath);
            void saveConfiguration(const std::string &basePath); 

            watcher::event::Color backgroundColor;
            watcher::event::Color foregroundColor;

            std::string fontName; // not really supported
            float pointSize;

            std::string labelText; 

        protected:

        private:

            DECLARE_LOGGER();
    };
}

#endif // LABEL_DISPLAY_INFO_H

