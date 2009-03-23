#include <boost/serialization/array.hpp>        // address.v4 bytes is an array of char
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>

#include "labelMessage.h"
#include "messageTypesAndVersions.h"
#include "watcherGlobalFunctions.h"     // for address serialization

using namespace std;

namespace watcher {
    namespace event {
        INIT_LOGGER(LabelMessage, "Message.LabelMessage");
        BOOST_CLASS_EXPORT_GUID(LabelMessage, "LabelMessage");

        LabelMessage::LabelMessage(const string &label_, int fontSize_)   :
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(Color::black),
            background(Color::white),
            expiration(0),
            addLabel(true),
            address(),
            lat(0),
            lng(0),
            alt(0)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        LabelMessage::LabelMessage(const string &label_, const boost::asio::ip::address &address_, int fontSize_)   :
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(Color::black),
            background(Color::white),
            expiration(0),
            addLabel(true),
            address(address_),
            lat(0),
            lng(0),
            alt(0)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        LabelMessage::LabelMessage(const std::string &label_, const float &lat_, const float &lng_, const float &alt_, const int fontSize_) : 
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(Color::black),
            background(Color::white),
            expiration(0),
            addLabel(true),
            address(),
            lat(lat_),
            lng(lng_),
            alt(alt_)
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
                addLabel==other.addLabel;

            // Compare either address or space coords but not both. Address takes precidence.
            if (address.to_v4().to_ulong()==0)
                retVal = 
                    retVal && 
                    lat==other.lat && 
                    lng==other.lng &&
                    alt==other.alt;
            else
                retVal = retVal && address==other.address;

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
            foreground=other.foreground;
            background=other.background;
            expiration=other.expiration;
            addLabel=other.addLabel;
            address=other.address;
            lat=other.lat;
            lng=other.lng;
            alt=other.alt;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &LabelMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " label: " << label;
            if (address.to_v4().to_ulong()==0)
                out << " (floating) ";
            else
                out << " (attached) "; 
            out << " font size: " << fontSize; 
            out << " address: " << address << (address.is_v4() ? " (v4)" : " (v6)"); 
            out << " fg: (" << foreground << ")"; 
            out << " bg: (" << background << ")"; 
            out << " exp: " << expiration;
            out << " add: " << (addLabel ? "true" : "false"); 
            out << " lat: " << lat; 
            out << " lng: " << lng; 
            out << " alt: " << alt; 

            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const LabelMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }
    }
}
