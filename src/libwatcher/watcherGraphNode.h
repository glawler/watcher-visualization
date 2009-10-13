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
 * @file watcherGraphNode.h
 * @author Geoff Lawler <geoff.lawler@cobham.com> 
 * @date 2009-07-15
 */
#ifndef THIS_IS_NOT_THE_DISCRETE_MASS_OF_ONE_KIND_OF_TISSUE_ENCLOSED_IN_TISSUE_OF_A_DIFFERENT_KIND_OF_NODE_ITS_THE_OTHER_ONE
#define THIS_IS_NOT_THE_DISCRETE_MASS_OF_ONE_KIND_OF_TISSUE_ENCLOSED_IN_TISSUE_OF_A_DIFFERENT_KIND_OF_NODE_ITS_THE_OTHER_ONE

#include <boost/serialization/access.hpp>
#include <list>
#include <string>

#include "declareLogger.h"
#include "gpsMessage.h"
#include "labelMessage.h"
#include "colorMessage.h"
#include "messageTypesAndVersions.h"     // for GUILayer

#include "nodeDisplayInfo.h"
#include "labelDisplayInfo.h"

namespace watcher
{
    using namespace event;

    /**
     * @class WatcherGraphNode
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * A class that holds the data at the vertexes of a WatcherGraph
     */
    class WatcherGraphNode 
    {
        // GTL TO DO: put in oper=() and copy ctor(). 
        public:
            WatcherGraphNode();
            virtual ~WatcherGraphNode();

            /** configurable Display information about this node */
            NodeDisplayInfoPtr displayInfo; 

            /** Unique node id */
            NodeIdentifier nodeId;

            /** Curerent GPS coordinates for this node. */
            GPSMessagePtr gpsData;
            bool connected;

            /** An edge can have mulitple lables attached to it */
            typedef std::list<LabelDisplayInfoPtr> LabelList;
            LabelList labels;

            /** output operator **/
            virtual std::ostream &toStream(std::ostream &out) const;

            /** output operator **/
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:
        private:
            friend class boost::serialization::access;
            template <typename Archive> void serialize(Archive & ar, const unsigned int file_version);

            DECLARE_LOGGER(); 
    };

    /** typedef a shared pointer to this class
    */
    typedef boost::shared_ptr<WatcherGraphNode> WatcherGraphNodePtr;

    /** write a human readable version of the WatcherGraphNode class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const WatcherGraphNode &theNode);
}

#endif 
