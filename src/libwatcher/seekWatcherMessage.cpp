/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#include <boost/serialization/export.hpp>
#include "seekWatcherMessage.h"

BOOST_CLASS_EXPORT_GUID(watcher::watchapi::SeekMessage, "SeekMessage");

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

        std::ostream& operator<< (std::ostream& o, const SeekMessage& m)
        {
            return o << "SeekMessage(offset=" << m.offset << " , rel=" << m.rel << ')';
        }
    }
}
