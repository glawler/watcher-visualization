/** @file
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-23
 */
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include "nodeStatusMessage.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(NodeStatusMessage, "Message.NodeStatusMessage");

        NodeStatusMessage::NodeStatusMessage(const statusEvent &event_) : event(event_)
        {
             TRACE_ENTER();
             TRACE_EXIT(); 
        }

        std::ostream& operator<< (std::ostream& os, const NodeStatusMessage& msg)
        {
             TRACE_ENTER();
             os << "NodeStatusMessage";
             TRACE_EXIT(); 
             return os;
        }

        template <typename Archive> void NodeStatusMessage::serialize(Archive & ar, const unsigned int version)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & event;
            ar & nodeId;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::NodeStatusMessage);
