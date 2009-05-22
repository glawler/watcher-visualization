#ifndef THIS_IS_NOT_THE_DISCRETE_MASS_OF_ONE_KIND_OF_TISSUE_ENCLOSED_IN_TISSUE_OF_A_DIFFERENT_KIND_OF_NODE_ITS_THE_OTHER_ONE
#define THIS_IS_NOT_THE_DISCRETE_MASS_OF_ONE_KIND_OF_TISSUE_ENCLOSED_IN_TISSUE_OF_A_DIFFERENT_KIND_OF_NODE_ITS_THE_OTHER_ONE

/**
 * @file
 * @author Geoff.Lawler@cobham.com
 * @date 2009-05-19
 */
#include <list>
#include <string>

#include "logger.h"
#include "libwatcher/gpsMessage.h"
#include "libwatcher/labelMessage.h"
#include "libwatcher/messageTypesAndVersions.h"     // for GUILayer

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

            NodeIdentifier nodeId;
            GPSMessagePtr gpsData;
            std::string label;
            bool connected;

            GUILayer layer;  // Needed so we can remove by layer if needed. 

            typedef std::list<LabelMessagePtr> LabelMessageList;
            LabelMessageList attachedLabels; 

            /** output operator **/
            virtual std::ostream &toStream(std::ostream &out) const;

            /** output operator **/
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        protected:
        private:

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
