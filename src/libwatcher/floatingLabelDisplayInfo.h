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
#ifndef FLOATING_LABEL_DISPLAY_INFO_H
#define FLOATING_LABEL_DISPLAY_INFO_H

#include "labelDisplayInfo.h"

namespace watcher
{
    /** 
     * Keep track of display information for a floating label (label+coordinates).
     * @author Geoff.Lawler <Geoff.Lawler@cobham.com> 
     */
    class FloatingLabelDisplayInfo : public LabelDisplayInfo
    {
        public:
            FloatingLabelDisplayInfo(); 
            FloatingLabelDisplayInfo(const FloatingLabelDisplayInfo &copy); 
            virtual ~FloatingLabelDisplayInfo(); 

            FloatingLabelDisplayInfo &operator=(const FloatingLabelDisplayInfo &lhs);

            void initialize(const watcher::event::LabelMessagePtr &m); 

            /** The coordinates of the floating label */
            double lat, lng, alt;

            virtual std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            void saveConfiguration() const; 

        protected:

        private:

            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<FloatingLabelDisplayInfo> FloatingLabelDisplayInfoPtr; 

    std::ostream &operator<<(std::ostream &out, const watcher::FloatingLabelDisplayInfo &obj);
}


#endif // FLOATING_LABEL_DISPLAY_INFO_H

