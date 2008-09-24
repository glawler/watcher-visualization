#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/array.hpp>        // address.v4 bytes is an array of char
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>

#include "labelMessage.h"
#include "messageTypesAndVersions.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(LabelMessage, "Message.LabelMessage");
BOOST_CLASS_EXPORT_GUID(LabelMessage, "LabelMessage");

LabelMessage::LabelMessage(const string &label_, int fontSize_)   :
    Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
    label(label_),
    fontSize(fontSize_),
    address(),
    foreground(Color::black),
    background(Color::white),
    expiration(0)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

LabelMessage::LabelMessage(const string &label_, const boost::asio::ip::address &address_, int fontSize_)   :
    Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
    label(label_),
    fontSize(fontSize_),
    address(address_),
    foreground(Color::black),
    background(Color::white),
    expiration(0)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

LabelMessage::LabelMessage(const LabelMessage &other)
{
    TRACE_ENTER();
    this->operator=(other); 
    TRACE_EXIT();
}

bool LabelMessage::operator==(const LabelMessage &other) const
{
    TRACE_ENTER();
    
    bool retVal = 
        Message::operator==(other) && 
        label==other.label && 
        address==other.address;

    // These are not distinguishing features
    //  foreground==other.foreground,
    //  background==other.background,
    //  expiration==other.expiration,
    //  fontSize==other.FontSize;

    TRACE_EXIT_RET(retVal);
    return retVal;
}

LabelMessage &LabelMessage::operator=(const LabelMessage &other)
{
    TRACE_ENTER();

    Message::operator=(other);
    label=other.label; 
    fontSize=other.fontSize;
    address=other.address;
    foreground=other.foreground;
    background=other.background;
    expiration=other.expiration;
    fontSize=other.fontSize;

    TRACE_EXIT();
    return *this;
}

// virtual 
std::ostream &LabelMessage::toStream(std::ostream &out) const
{
    TRACE_ENTER();

    Message::toStream(out);
    out << " label: " << label;
    out << " font size: " << fontSize; 
    out << " address: " << address << (address.is_v4() ? " (v4)" : " (v6)"); 
    out << " fg: (" << foreground << ")"; 
    out << " bg: (" << background << ")"; 
    out << " exp: " << expiration;

    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const LabelMessage &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}

void LabelMessage::serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version) 
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & label;
    ar & foreground;
    ar & background;
    ar & expiration;
    ar & fontSize;

    string tmp;
    ar & tmp;
    address=boost::asio::ip::address::from_string(tmp); 

    TRACE_EXIT();
}

void LabelMessage::serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & label;
    ar & foreground;
    ar & background;
    ar & expiration;
    ar & fontSize;

    string tmp=address.to_string();
    ar & tmp;

    TRACE_EXIT();
}

