/** @file stopWatcherMessage.h
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#ifndef STOP_WATCHER_MESSAGE_H
#define STOP_WATCHER_MESSAGE_H

#include "message.h"

namespace watcher {
    namespace event {

        /**
         * Stop playback of event stream.
         * @author Michael Elkins <michael.elkins@cobham.com>
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

        typedef boost::shared_ptr<StopMessage> StopMessagePtr;
    }
}
#endif
