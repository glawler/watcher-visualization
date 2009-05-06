/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/export.hpp>
#include "stopWatcherMessage.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(StopMessage, "Message.StopMessage");

        StopMessage::StopMessage() : Message(STOP_MESSAGE_TYPE, STOP_MESSAGE_VERSION)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        template <typename Archive> void StopMessage::serialize(Archive& ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::StopMessage)
