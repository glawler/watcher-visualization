/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "startWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::event::StartMessage, "StartMessage");

namespace watcher {
    namespace event {
        INIT_LOGGER(StartMessage, "Message.StartMessage");

        StartMessage::StartMessage(): Message(START_MESSAGE_TYPE, START_MESSAGE_VERSION)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
    }
}
