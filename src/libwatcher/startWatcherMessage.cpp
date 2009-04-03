/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "startWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::watchapi::StartMessage, "StartMessage");

namespace watcher {
    namespace watchapi {
        INIT_LOGGER(StartMessage, "WatcherMessage.StartMessage");
    }
}
