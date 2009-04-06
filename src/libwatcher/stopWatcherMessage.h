/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef STOP_WATCHER_MESSAGE_H
#define STOP_WATCHER_MESSAGE_H

#include "watcherMessage.h"

namespace watcher {
    namespace event {

        /**
         * Stop playback of event stream.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class StopMessage : public Message {
            public:
            StopMessage(); 

            private:
            friend class boost::serialization::access;
            template <typename Archive> void serialize(Archive& ar, const unsigned int version);
            DECLARE_LOGGER();
        };

        template <typename Archive>
            void StopMessage::serialize(Archive& ar, const unsigned int version)
            {
                TRACE_ENTER();
                ar & boost::serialization::base_object<Message>(*this);
                TRACE_EXIT();
            }

        typedef boost::shared_ptr<StopMessage> StopMessagePtr;
    }

}
#endif
