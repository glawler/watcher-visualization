/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef START_WATCHER_MESSAGE_H
#define START_WATCHER_MESSAGE_H

#include "watcherMessage.h"

namespace watcher {
    namespace event {

        /**
         * Start playback of event stream.
         * @author Michael Elkins <michael.elkins@sparta.com>
         * @date 2009-03-20
         */
        class StartMessage : public Message {
            public:
            StartMessage(); 
            private:
            template <typename Archive> void serialize(Archive & ar, const unsigned int version);
            friend class boost::serialization::access;
            DECLARE_LOGGER();
        };

        template <typename Archive> void StartMessage::serialize(Archive & ar, const unsigned int version)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            TRACE_EXIT();
        }

        typedef boost::shared_ptr<StartMessage> StartMessagePtr;
    }
}
#endif
