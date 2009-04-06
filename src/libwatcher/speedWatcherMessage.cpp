/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "speedWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::event::SpeedMessage, "SpeedMessage");

namespace watcher {
    namespace event {
        /**
         * Set the playback speed of the event stream.
         * A negative value indicates reverse direction.
         */
        SpeedMessage::SpeedMessage(float speed_)
            : Message(SPEED_MESSAGE_TYPE, SPEED_MESSAGE_VERSION), speed(speed_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
       
        std::ostream& operator<< (std::ostream& o, const SpeedMessage& rhs)
        {
            return o << "SpeedMessage(speed=" << rhs.speed << ')';
        }

        INIT_LOGGER(SpeedMessage, "Message.SpeedMessage");
    }
}
