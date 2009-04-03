/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef SPEED_WATCHER_MESSAGE_H
#define SPEED_WATCHER_MESSAGE_H

#include "watcherMessage.h"

namespace watcher {
    namespace watchapi {

        /**
         * Set playback speed of event stream.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class SpeedMessage : public WatcherMessage {
            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive & ar, const unsigned int version);
                DECLARE_LOGGER();
                float speed;    //< playback speed.  negative value indicates reverse direction

            public:
                SpeedMessage(float speed);
        };

        template <typename Archive>
        void SpeedMessage::serialize(Archive & ar, const unsigned int version)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<WatcherMessage>(*this);
            ar & speed;
            TRACE_EXIT();
        }
    }
}
#endif
