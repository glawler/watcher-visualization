#ifndef WGNODE_HASANYBODY_SEEN_MY_GAL
#define WGNODE_HASANYBODY_SEEN_MY_GAL

/**
 * @file
 * @author Geoff.Lawler@cobham.com
 * @date 2009-05-19
 */

#include <string>
#include <list>
#include <boost/serialization/access.hpp>

#include "libwatcher/watcherTypes.h"    // for Timestamp
#include "libwatcher/labelMessage.h" 

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

            /**
             * The edge's label. This is different that a test node attached label.
             * It is the label of the edge itself. 
             */
            std::string label;

            Color color;
            Timestamp expiration;
            float width;

            /** An edge can have mulitple lables attached to it */
            typedef std::list<LabelMessagePtr> LabelMessageList;
            LabelMessageList attachedLabels; 

            GUILayer layer;  // Needed so we can remove by layer if needed. 

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
