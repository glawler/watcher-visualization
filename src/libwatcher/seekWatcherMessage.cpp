/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include "seekWatcherMessage.h"

namespace watcher {
    namespace watchapi {
        INIT_LOGGER(SeekMessage, "WatcherMessage.SeekMessage");

        /**
         * Seek to a particular point in time in the event stream.
         * @param offset_ time at which to seek to
         * @param w how to interpret the time offset
         */
        SeekMessage::SeekMessage(float offset_, whence w)
            : offset(offset_), rel(w)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }
    }
}
