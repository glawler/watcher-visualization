#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include "gpsMessage.h"

using namespace std;
using namespace watcher;

INIT_LOGGER(GPSMessage, "Message.GPSMessage");
BOOST_CLASS_EXPORT_GUID(GPSMessage, "GPSMessage");

GPSMessage::GPSMessage(const float &lat_, const float &lng_, const float &alt_) : 
    Message(GPS_MESSAGE_TYPE, GPS_MESSAGE_VERSION),
    lat(lat_),
    lng(lng_),
    alt(alt_)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

GPSMessage::GPSMessage(const GPSMessage &other)
{
    TRACE_ENTER();
    (*this)=other;
    TRACE_EXIT();
}

bool GPSMessage::operator==(const GPSMessage &other) const
{
    TRACE_ENTER();
    
    bool retVal = 
        Message::operator==(other) && 
        lat == other.lat &&
        lng == other.lng &&
        alt == other.alt;

    TRACE_EXIT_RET(retVal);
    return retVal;
}

GPSMessage &GPSMessage::operator=(const GPSMessage &other)
{
    TRACE_ENTER();

    Message::operator=(other);
    lat = other.lat;
    lng = other.lng;
    alt = other.alt;

    TRACE_EXIT();
    return *this;
}

// virtual 
std::ostream &GPSMessage::toStream(std::ostream &out) const
{
    TRACE_ENTER();

    Message::toStream(out);
    out << " lat: " << lat;
    out << " lng: " << lng;
    out << " alt: " << alt;

    TRACE_EXIT();
    return out;
}

ostream &watcher::operator<<(ostream &out, const GPSMessage &mess)
{
    TRACE_ENTER();
    mess.operator<<(out);
    TRACE_EXIT();
    return out;
}


void GPSMessage::serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & lat;
    ar & lng;
    ar & alt;
    TRACE_EXIT();
}
void GPSMessage::serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version)
{
    TRACE_ENTER();
    ar & boost::serialization::base_object<Message>(*this);
    ar & lat;
    ar & lng;
    ar & alt;
    TRACE_EXIT();
}

