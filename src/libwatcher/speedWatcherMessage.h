/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef SPEED_WATCHER_MESSAGE_H
#define SPEED_WATCHER_MESSAGE_H

#include "watcherMessage.h"

namespace watcher {
    namespace event {

        /**
         * Set playback speed of event stream.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class SpeedMessage : public Message {
            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int version);
                DECLARE_LOGGER();

            public:
                float speed;    //< playback speed.  negative value indicates reverse direction
                SpeedMessage(float speed = 1.0);
                bool operator== (const SpeedMessage& rhs) const { return speed == rhs.speed; }
                friend std::ostream& operator<< (std::ostream& o, const SpeedMessage& rhs);
        };

        template <typename Archive>
        void SpeedMessage::serialize(Archive & ar, const unsigned int version)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & speed;
            TRACE_EXIT();
        }

        typedef boost::shared_ptr<SpeedMessage> SpeedMessagePtr;
    }
}
#endif
