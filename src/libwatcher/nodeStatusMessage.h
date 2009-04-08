/** @file
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-23
 */
#ifndef NODE_CONNECTION_MESSAGE_H
#define NODE_CONNECTION_MESSAGE_H

#include "message.h"
#include "watcherTypes.h"
#include "watcherGlobalFunctions.h"

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

                NodeStatusMessage(const statusEvent &event = connect); 

                template <typename Archive> void serialize(Archive & ar, const unsigned int version);

            private:
                DECLARE_LOGGER();

                statusEvent event;    // What happened
                NodeIdentifier nodeId;
                friend class boost::serialization::access;
                friend std::ostream& operator<<(std::ostream&, const NodeStatusMessage&);
        };

    }
}
#endif
