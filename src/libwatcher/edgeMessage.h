#ifndef WATCHER_EDGE_MESSAGE_DATA_H
#define WATCHER_EDGE_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "labelMessage.h"
#include "watcherColors.h"
#include "watcherTypes.h" 

namespace watcher {
    namespace event {
        class EdgeMessage : public Message {
            public:
                // The data
                NodeIdentifier node1;
                NodeIdentifier node2;
                Color edgeColor;
                Timestamp expiration; 
                float width;      
                GUILayer layer;
                bool addEdge;   // if true, add an edge, else remove existing edge;
                LabelMessagePtr middleLabel;
                LabelMessagePtr node1Label;
                LabelMessagePtr node2Label;
                bool bidirectional;

                EdgeMessage();
                EdgeMessage(
                            const NodeIdentifier &node1_, // address of node1
                            const NodeIdentifier &node2_, // address of node2
                            const GUILayer &layer_=PHYSICAL_LAYER,  // Which GUI layer the edge is on
                            const Color &c_=Color::blue,            // color of edge
                            const unsigned int &width=15,           // width of the edge 
                            const bool bidirectional_=false,        // Is this edge bidirectional?
                            const Timestamp  expiration_=Infinity,  // expiration time in milliseconds, 0=never expire
                            const bool &addEdge=true);              // If true, add an edge, else remove exising edge with same props as this one.

                EdgeMessage(const EdgeMessage &other);

                void setMiddleLabel(const LabelMessagePtr &label); 
                void setNode1Label(const LabelMessagePtr &label); 
                void setNode2Label(const LabelMessagePtr &label); 

                bool operator==(const EdgeMessage &other) const;
                EdgeMessage &operator=(const EdgeMessage &other);

                virtual std::ostream &toStream(std::ostream &out) const;
                std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            private:
                friend class boost::serialization::access;
                template <typename Archive> void serialize(Archive& ar, const unsigned int file_version);
                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<EdgeMessage> EdgeMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const EdgeMessage &mess);
    }
}
#endif // WATCHER_EDGE_MESSAGE_DATA_H
