#ifndef WATCHER_EDGE_MESSAGE_DATA_H
#define WATCHER_EDGE_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "logger.h"
#include "message.h"
#include "labelMessage.h"
#include "watcherColors.h"

// Forward decls;
namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
    namespace asio
    {
        namespace ip
        {
            class address;
        }
    }
}

namespace watcher 
{
    class EdgeMessage : public Message
    {
        public:
            // The data
            boost::asio::ip::address node1;
            boost::asio::ip::address node2;
            watcher::Color edgeColor;
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

            virtual void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            virtual void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        private:
            DECLARE_LOGGER();
    };

    typedef boost::shared_ptr<EdgeMessage> EdgeMessagePtr; 

    std::ostream &operator<<(std::ostream &out, const EdgeMessage &mess);
}

#endif // WATCHER_EDGE_MESSAGE_DATA_H
