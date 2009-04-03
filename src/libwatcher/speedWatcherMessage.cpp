/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "speedWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::watchapi::SpeedMessage, "SpeedMessage");

namespace watcher {
    namespace watchapi {
        /**
         * Set the playback speed of the event stream.
         * A negative value indicates reverse direction.
         */
        SpeedMessage::SpeedMessage(float speed_ = 1.0)
            : speed(speed_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
       
        INIT_LOGGER(SpeedMessage, "WatcherMessage.SpeedMessage");
    }
}
