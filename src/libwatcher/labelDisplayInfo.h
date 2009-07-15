/**
 * @file labelDisplayInfo.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef LABEL_DISPLAY_INFO_H
#define LABEL_DISPLAY_INFO_H

#include <string>
#include "watcherTypes.h"
#include "watcherColors.h"
#include "labelMessage.h"
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

            watcher::event::Color backgroundColor;
            watcher::event::Color foregroundColor;

            std::string fontName; // not really supported
            float pointSize;

            /** The text in the label */
            std::string labelText; 

            /** This doesn't really belong here, but I've nowhere else to put it. */
            Timestamp expiration; 

            /** Can be loaded from a LabelMessage */
            bool loadConfiguration(const watcher::event::LabelMessagePtr &labelMessage); 

            /** Can be loaded from cfg file ... */
            bool loadConfiguration(const watcher::event::GUILayer &layer); 

            /** Can be saved to cfg file ... */
            void saveConfiguration(); 

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<LabelDisplayInfo> LabelDisplayInfoPtr; 

    std::ostream &operator<<(std::ostream &out, const watcher::LabelDisplayInfo &obj);
}


#endif // LABEL_DISPLAY_INFO_H

