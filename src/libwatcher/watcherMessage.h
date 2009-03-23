/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef WATCHER_MESSAGE_H
#define WATCHER_MESSAGE_H

namespace watcher {
    /** Namespace containing messages for the watcherd api */
    namespace watchapi {

        /**
         * Parent class of all messages in the Watcher<->GUI protocol.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class WatcherMessage {
            protected:
                //don't allow for direct contruction
                WatcherMessage() {};

            public:
                virtual ~WatcherMessage() {};
        };
    }
}
#endif
