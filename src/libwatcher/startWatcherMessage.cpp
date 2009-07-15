/** @file startWatcherMessage.cpp
 * @author Michael Elkins <michael.elkins@cobham.com>
 * @date 2009-03-20
 */
#include "watcherSerialize.h"
#include "startWatcherMessage.h"

namespace watcher {
    namespace event {
        INIT_LOGGER(StartMessage, "Message.StartMessage");

        StartMessage::StartMessage(): Message(START_MESSAGE_TYPE, START_MESSAGE_VERSION)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        std::ostream& operator<< (std::ostream& os, const StartMessage& /* m */)
        {
            return os << "StartMessage()";
        }

        bool operator== (const StartMessage& /* lhs */, const StartMessage& /* rhs */) { return true; };

        template <typename Archive> void StartMessage::serialize(Archive & ar, const unsigned int /* version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            TRACE_EXIT();
        }

    }
}

BOOST_CLASS_EXPORT(watcher::event::StartMessage);
