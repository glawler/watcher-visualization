#include <boost/asio.hpp>

#include <boost/serialization/string.hpp>       // for serializing addresses.
#include <boost/serialization/shared_ptr.hpp>   // for serializing LabelMessagePtrs
#include <boost/serialization/export.hpp>

#include <iosfwd>

#include "edgeMessage.h"
#include "messageTypesAndVersions.h"
#include "watcherGlobalFunctions.h"         // for address serialize(). 

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {

        INIT_LOGGER(EdgeMessage, "Message.EdgeMessage");
        BOOST_CLASS_EXPORT_GUID(EdgeMessage, "EdgeMessage");

        EdgeMessage::EdgeMessage(
                                 const asio::ip::address &node1_,        
                                 const asio::ip::address &node2_,        
                                 const GUILayer &layer_,                 
                                 const Color &c_,
                                 const unsigned int &width_,
                                 const bool bidirectional_,
                                 unsigned int expiration_, 
                                 const bool &addEdge_) : 
            Message(EDGE_MESSAGE_TYPE, EDGE_MESSAGE_VERSION),
            node1(node1_),
            node2(node2_),
            edgeColor(c_),
            expiration(expiration_),
            width(width_),
            layer(layer_),
            addEdge(addEdge_),
            middleLabel(),
            node1Label(),
            node2Label(),
            bidirectional(bidirectional_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        EdgeMessage::EdgeMessage() : 
            Message(EDGE_MESSAGE_TYPE, EDGE_MESSAGE_VERSION),
            node1(),
            node2(),
            edgeColor(),
            expiration(0), 
            width(15),
            layer(),
            addEdge(true),
            middleLabel(),
            node1Label(),
            node2Label(),
            bidirectional(false)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        EdgeMessage::EdgeMessage(const EdgeMessage &other)
        {
            TRACE_ENTER();
            this->operator=(other); 
            TRACE_EXIT();
        }

        void EdgeMessage::setMiddleLabel(const LabelMessagePtr &label)
        {
            TRACE_ENTER();
            middleLabel=label;
            TRACE_EXIT();
        }
        void EdgeMessage::setNode1Label(const LabelMessagePtr &label) 
        {
            TRACE_ENTER();
            node1Label=label;
            node1Label->address=node1;
            TRACE_EXIT();
        }
        void EdgeMessage::setNode2Label(const LabelMessagePtr &label)
        {
            TRACE_ENTER();
            node2Label=label;
            node2Label->address=node2;
            TRACE_EXIT();
        }

        bool EdgeMessage::operator==(const EdgeMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                node1==other.node1 && 
                node2==other.node2 && 
                bidirectional==other.bidirectional &&
                layer==other.layer;

            // Other member variables are not distinguishing features

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        EdgeMessage &EdgeMessage::operator=(const EdgeMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            node1=other.node1;
            node2=other.node2;
            edgeColor=other.edgeColor;
            expiration=other.expiration;
            width=other.width;
            layer=other.layer;
            addEdge=other.addEdge;
            middleLabel=other.middleLabel;
            node1Label=other.node1Label;
            node2Label=other.node2Label;
            bidirectional=other.bidirectional;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &EdgeMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " node1: " << node1;
            out << " node2: " << node2;
            out << " edgeColor: " << edgeColor;
            out << " expiration: " << expiration;
            out << " width: " << width;
            out << " dir: " << (bidirectional ? "bidirectional" : "unidirectional");
            out << " layer: " << layer;
            out << " add: " << (addEdge ? "true" : "false"); 

            // shared_ptr doesn't have ?: overloaded
            out << " node1Label: ";
            if (node1Label) out << *node1Label;
            else out << " NULL "; 

            out << " middleLabel: ";
            if (middleLabel) out << *middleLabel;
            else out << " NULL "; 

            out << " node2Label: ";
            if (node2Label) out << *node2Label;
            else out << " NULL "; 

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const EdgeMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }
    }
}
