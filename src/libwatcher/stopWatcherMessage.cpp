/** @file stopWatcherMessage.cpp
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#include "watcherSerialize.h"
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
