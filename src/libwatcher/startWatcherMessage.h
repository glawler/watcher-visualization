/** @file
 * @author Michael Elkins <michael.elkins@sparta.com>
 * @date 2009-03-20
 */
#ifndef START_WATCHER_MESSAGE_H
#define START_WATCHER_MESSAGE_H

#include "message.h"

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

        std::ostream& operator<< (std::ostream&, const StartMessage&);

        typedef boost::shared_ptr<StartMessage> StartMessagePtr;

        bool operator== (const StartMessage& lhs, const StartMessage& rhs);
    }
}
#endif
