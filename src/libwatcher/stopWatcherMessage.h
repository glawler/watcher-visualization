/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef STOP_WATCHER_MESSAGE_H
#define STOP_WATCHER_MESSAGE_H

#include "watcherMessage.h"

namespace watcher {
    namespace watchapi {

        /**
         * Stop playback of event stream.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class StopMessage : public WatcherMessage {
            public:
                template <typename Archive> void serialize(Archive& ar, const unsigned int version);
            private:
                DECLARE_LOGGER();
        };

        template <typename Archive>
        void StopMessage::serialize(Archive& ar, const unsigned int version)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
    }
}
#endif
