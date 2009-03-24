#ifndef WATCHER_EDGE_MESSAGE_DATA_H
#define WATCHER_EDGE_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "message.h"
#include "labelMessage.h"
#include "watcherColors.h"

namespace watcher {
    namespace event {
        class EdgeMessage : public Message {
            public:
                // The data
                boost::asio::ip::address node1;
                boost::asio::ip::address node2;
                Color edgeColor;
                unsigned int expiration;
                unsigned int width;
                GUILayer layer;
                bool addEdge;   // if true, add an edge, else remove existing edge;
                LabelMessagePtr middleLabel;
                LabelMessagePtr node1Label;
                LabelMessagePtr node2Label;
                bool bidirectional;

                EdgeMessage();
                EdgeMessage(
                            const boost::asio::ip::address &node1_, // address of node1
                            const boost::asio::ip::address &node2_, // address of node2
                            const GUILayer &layer_,                 // Which GUI layer the edge is on
                            const Color &c_=Color::blue,            // color of edge
                            const unsigned int &width=15,           // width of the edge 
                            const bool bidirectional_=false,        // Is this edge bidirectional?
                            unsigned int expiration_=0,             // expiration time in milliseconds, 0=never expire
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
                template <typename Archive>
                void serialize(Archive& ar, const unsigned int file_version) 
                {
                    TRACE_ENTER();

                    ar & boost::serialization::base_object<Message>(*this);
                    ar & node1;
                    ar & node2;
                    ar & edgeColor;
                    ar & expiration;
                    ar & width;
                    ar & layer;
                    ar & addEdge;
                    ar & node1Label;
                    ar & middleLabel;
                    ar & node2Label;
                    ar & bidirectional;

                    TRACE_EXIT();
                }

                DECLARE_LOGGER();
        };

        typedef boost::shared_ptr<EdgeMessage> EdgeMessagePtr; 

        std::ostream &operator<<(std::ostream &out, const EdgeMessage &mess);
    }
}
#endif // WATCHER_EDGE_MESSAGE_DATA_H