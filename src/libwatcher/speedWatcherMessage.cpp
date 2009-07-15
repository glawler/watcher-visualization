/** @file speedWatcherMessage.cpp
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#include "watcherSerialize.h"
#include "speedWatcherMessage.h"

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

        template <typename Archive>
        void SpeedMessage::serialize(Archive & ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & speed;
            TRACE_EXIT();
        }

        INIT_LOGGER(SpeedMessage, "Message.SpeedMessage");
    }
}

BOOST_CLASS_EXPORT(watcher::event::SpeedMessage);
