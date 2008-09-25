#include <boost/asio.hpp>

#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>       // for serializing addresses.
#include <boost/serialization/export.hpp>

#include "edgeMessage.h"
#include "messageTypesAndVersions.h"
#include "watcherGlobalFunctions.h"         // for address serialize(). 

using namespace std;
using namespace watcher;
using namespace boost;

INIT_LOGGER(EdgeMessage, "Message.EdgeMessage");
BOOST_CLASS_EXPORT_GUID(EdgeMessage, "EdgeMessage");

EdgeMessage::EdgeMessage(
        const asio::ip::address &node1_,        
        const asio::ip::address &node2_,        
        const GUILayer &layer_,                 
        const Color &c_,
        const unsigned int &width_,
        unsigned int expiration_, 
        const string &label_, 
        const Color &labelfg_, 
        const Color &labelbg_, 
        const unsigned int fontSize_) :
    Message(EDGE_MESSAGE_TYPE, EDGE_MESSAGE_VERSION),
    label(label_), 
    fontSize(fontSize_),
    node1(node1_),
    node2(node2_),
    edgeColor(c_),
    labelColorForeground(labelfg_),
    labelColorBackground(labelbg_),
    expiration(expiration_),
    width(width_),
    layer(layer_)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

EdgeMessage::EdgeMessage() : 
    Message(EDGE_MESSAGE_TYPE, EDGE_MESSAGE_VERSION),
    label(), 
    fontSize(10),
    node1(),
    node2(),
    edgeColor(),
    labelColorForeground(),
    labelColorBackground(),
    expiration(0), 
    width(15),
    layer()
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

bool EdgeMessage::operator==(const EdgeMessage &other) const
{
    TRACE_ENTER();
    
    bool retVal = 
        Message::operator==(other) && 
        label==other.label && 
        node1==other.node1 && 
        node2==other.node2 && 
        layer==other.layer;

    // Other member variables are not distinguishing features

    TRACE_EXIT_RET(retVal);
    return retVal;
}

EdgeMessage &EdgeMessage::operator=(const EdgeMessage &other)
{
    TRACE_ENTER();

    Message::operator=(other);
    label=other.label;
    fontSize=other.fontSize;
    node1=other.node1;
    node2=other.node2;
    edgeColor=other.edgeColor;
    labelColorForeground=other.labelColorForeground;
    labelColorBackground=other.labelColorBackground;
    expiration=other.expiration;
    width=other.width;
    layer=other.layer;

    TRACE_EXIT();
    return *this;
}

// virtual 
std::ostream &EdgeMessage::toStream(std::ostream &out) const
{
    TRACE_ENTER();

    Message::toStream(out);
    out << " label: " << label;
    out << " fontSize: " << fontSize;
    out << " node1: " << node1;
    out << " node2: " << node2;
    out << " edgeColor: " << edgeColor;
    out << " labelfg: " << labelColorForeground; 
    out << " labelbg: " << labelColorBackground;
    out << " expiration: " << expiration;
    out << " width: " << width;
    out << " layer: " << layer;

    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const EdgeMessage &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}

void EdgeMessage::serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version) 
{
    TRACE_ENTER();

    ar & boost::serialization::base_object<Message>(*this);
    ar & label;
    ar & fontSize;
    ar & node1;
    ar & node2;
    ar & edgeColor;
    ar & labelColorForeground;
    ar & labelColorBackground;
    ar & expiration;
    ar & width;
    ar & layer;

    TRACE_EXIT();
}

void EdgeMessage::serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();

    ar & boost::serialization::base_object<Message>(*this);
    ar & label;
    ar & fontSize;
    ar & node1;
    ar & node2;
    ar & edgeColor;
    ar & labelColorForeground;
    ar & labelColorBackground;
    ar & expiration;
    ar & width;
    ar & layer;

    TRACE_EXIT();
}

