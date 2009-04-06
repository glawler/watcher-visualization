/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "stopWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::event::StopMessage, "StopMessage");

namespace watcher {
    namespace event {
        INIT_LOGGER(StopMessage, "Message.StopMessage");

        StopMessage::StopMessage() : Message(STOP_MESSAGE_TYPE, STOP_MESSAGE_VERSION)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
    }
}
