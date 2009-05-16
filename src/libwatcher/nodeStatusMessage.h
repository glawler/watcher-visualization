/** 
 * @file
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-23
 */
#ifndef NODE_CONNECTION_MESSAGE_H
#define NODE_CONNECTION_MESSAGE_H

#include <string>

#include "message.h"
#include "watcherTypes.h"

namespace watcher 
{
    namespace event 
    {
        /**
         * Notify GUI of node status  events.
         * @author Geoff Lawler <geoff.lawler@sparta.com>
         * @date 2009-03-23
         */
        class NodeStatusMessage : public Message 
        {
            public:
                enum statusEvent 
                {
                    connect,
                    disconnect
                };

                NodeStatusMessage(const statusEvent &event=connect); 

                NodeStatusMessage(const NodeStatusMessage &copy); 

                virtual ~NodeStatusMessage();

                bool operator==(const NodeStatusMessage &other) const;
                NodeStatusMessage &operator=(const NodeStatusMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const; 

                static std::string statusEventToString(const statusEvent &e); 

                statusEvent event;      // What happened

            private:

                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int version);

                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<NodeStatusMessage> NodeStatusMessagePtr; 
        std::ostream &operator<<(std::ostream &out, const NodeStatusMessage &mess);
    }
}
#endif


