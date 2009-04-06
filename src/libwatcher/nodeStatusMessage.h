/** @file
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-23
 */
#ifndef NODE_CONNECTION_MESSAGE_H
#define NODE_CONNECTION_MESSAGE_H

#include "watcherMessage.h"
#include "watcherTypes.h"

namespace watcher 
{
    namespace event 
    {
        /**
         * Notify GUI of node connection event.
         * @author Geoff Lawler <geoff.lawler@sparta.com>
         * @date 2009-03-23
         */
        class NodeStatusMessage : public Message 
        {
            public:
                enum statusEvent 
                {
                    connect,
                    disconnnect
                };

                NodeStatusMessage(const statusEvent &event); 

                template <typename Archive> void serialize(Archive & ar, const unsigned int version);

            private:
                DECLARE_LOGGER();

                statusEvent event;    // What happened
                NodeIdentifier nodeId;
        };

        template <typename Archive> void NodeStatusMessage::serialize(Archive & ar, const unsigned int version)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & event;
            ar & nodeId;
            TRACE_EXIT();
        }
    }
}
#endif


