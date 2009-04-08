/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef WATCHER_MESSAGE_H
#define WATCHER_MESSAGE_H

#include "logger.h"
#include "message.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace watcher {
    /** Namespace containing messages for the watcherd api */
    namespace event {

        /**
         * Parent class of all messages in the Watcher<->GUI protocol.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class WatcherMessage {
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, unsigned int version);
            protected:
                //don't allow for direct contruction
                WatcherMessage() {};

            public:
                virtual ~WatcherMessage() {};
        };

        template <typename Archive> void WatcherMessage::serialize(Archive& ar, unsigned int version)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        typedef boost::shared_ptr<WatcherMessage> WatcherMessagePtr;
    }
}
#endif
