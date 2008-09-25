#ifndef WATCHER_EDGE_MESSAGE_DATA_H
#define WATCHER_EDGE_MESSAGE_DATA_H

#include <string>
#include <boost/asio.hpp>

#include "logger.h"
#include "message.h"
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
            std::string label;
            int fontSize;
            boost::asio::ip::address node1;
            boost::asio::ip::address node2;
            watcher::Color edgeColor;
            watcher::Color labelColorForeground;
            watcher::Color labelColorBackground;
            unsigned int expiration;
            unsigned int width;
            GUILayer layer;

            EdgeMessage();
            EdgeMessage(
                    const boost::asio::ip::address &node1_, // address of node1
                    const boost::asio::ip::address &node2_, // address of node2
                    const GUILayer &layer_,                 // Which GUI layer the edge is on
                    const Color &c_=Color::blue,            // color of edge
                    const unsigned int &width=15,           // width of the edge 
                    unsigned int expiration_=0,             // expiration time in milliseconds, 0=never expire
                    const std::string &label_="",           // edge's label
                    const Color &labelfg_=Color::black,     // color of label's foreground
                    const Color &labelbg_=Color::white,     // color of label's foreground
                    const unsigned int fontSize_=10);       // edge's label's font size


            EdgeMessage(const EdgeMessage &other);

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
