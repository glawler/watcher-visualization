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
     * Keep track of display information for a Label.
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     */
    class LabelDisplayInfo : 
        public DisplayInfo
    {
        public:
            LabelDisplayInfo();
            virtual ~LabelDisplayInfo(); 

            watcher::event::Color backgroundColor;
            watcher::event::Color foregroundColor;

            std::string fontName; ///< not really supported
            float pointSize;

            /** The text in the label */
            std::string labelText; 

            /** This doesn't really belong here, but I've nowhere else to put it. */
            Timestamp expiration; 

            /** load configuration from a LabelMessage.
             * @retval true configuration succeeded
             * @retval false failed
             */
            bool loadConfiguration(const watcher::event::LabelMessagePtr &labelMessage); 

            bool loadConfiguration(const watcher::event::GUILayer &layer); 

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

