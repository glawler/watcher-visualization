/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WGNODE_HASANYBODY_SEEN_MY_GAL
#define WGNODE_HASANYBODY_SEEN_MY_GAL

/**
 * @file watcherGraphEdge.h
 * @author Geoff.Lawler@cobham.com
 * @date 2009-05-19
 */

#include <string>
#include <list>
#include <boost/serialization/access.hpp>

#include "watcherTypes.h"    // for Timestamp
#include "edgeDisplayInfo.h"
#include "labelDisplayInfo.h"

namespace watcher
{
    using namespace event; 

    /**
     * @class WatcherGraphEdge
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-05-15
     *
     * A class that holds the data in the edges of a WatcherGraph
     */
    class WatcherGraphEdge
    {
        public:
            /**
             * Create a graph edge
             */
            WatcherGraphEdge();

            /**
             * And this too shall pass
             */
            virtual ~WatcherGraphEdge();

            /** Display information about this node. */
            EdgeDisplayInfoPtr displayInfo; 

            /** An edge can have mulitple lables attached to it */
            typedef std::list<LabelDisplayInfoPtr> LabelList;
            LabelList labels;

            /** When this edge expires: is in absolute epoch milliseconds. */
            Timestamp expiration; 

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
    typedef boost::shared_ptr<WatcherGraphEdge> WatcherGraphEdgePtr;

    /** write a human readable version of the WatcherGraphEdge class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const WatcherGraphEdge &theEdge);
}

#endif // WGNODE_HASANYBODY_SEEN_MY_GAL
