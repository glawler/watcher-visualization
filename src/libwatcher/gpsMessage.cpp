#include "watcherSerialize.h"
#include "gpsMessage.h"

using namespace std;


namespace watcher {
    namespace event {
        INIT_LOGGER(GPSMessage, "Message.GPSMessage");

        GPSMessage::GPSMessage(const double &x_, const double &y_, const double &z_) : 
            Message(GPS_MESSAGE_TYPE, GPS_MESSAGE_VERSION),
            x(x_),
            y(y_),
            z(z_),
            dataFormat(LAT_LONG_ALT_WGS84),
            layer(PHYSICAL_LAYER)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        GPSMessage::GPSMessage(const GPSMessage &other) :
            Message(other), 
            x(other.x),
            y(other.y),
            z(other.z),
            dataFormat(other.dataFormat),
            layer(other.layer)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool GPSMessage::operator==(const GPSMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                x == other.x &&
                y == other.y &&
                z == other.z && 
                dataFormat == other.dataFormat &&
                layer == other.layer;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        GPSMessage &GPSMessage::operator=(const GPSMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            x = other.x;
            y = other.y;
            z = other.z;
            dataFormat = other.dataFormat;
            layer = other.layer; 

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &GPSMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();
            
            out << std::setprecision(std::numeric_limits<double>::digits10 + 2); 
            Message::toStream(out);
            out << " x: " << x;
            out << " y: " << y;
            out << " z: " << z;
            out << " format: " << dataFormat;
            out << " layer: " << layer; 

            TRACE_EXIT();
            return out;
        }

        ostream &operator<<(ostream &out, const GPSMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

        template <typename Archive> void GPSMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & x;
            ar & y;
            ar & z;
            ar & dataFormat;
            ar & layer; 
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::GPSMessage);
