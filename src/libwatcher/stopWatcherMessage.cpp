/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "stopWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::watchapi::StopMessage, "StopMessage");

namespace watcher {
    namespace watchapi {
        INIT_LOGGER(StopMessage, "WatcherMessage.StopMessage");
    }
}
