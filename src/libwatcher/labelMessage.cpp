#include "watcherSerialize.h"
#include "labelMessage.h"
#include "messageTypesAndVersions.h"

using namespace std;

namespace watcher {
    namespace event {
        INIT_LOGGER(LabelMessage, "Message.LabelMessage");

        LabelMessage::LabelMessage(const string &label_, int fontSize_)   :
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(label_),
            fontSize(fontSize_),
            foreground(Color::black),
            background(Color::white),
            expiration(0),
            addLabel(true),
            layer(),
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
            layer(),
            lat(0),
            lng(0),
            alt(0)
        {
            TRACE_ENTER();
            fromNodeID=address_; 
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
            layer(),
            lat(lat_),
            lng(lng_),
            alt(alt_)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        LabelMessage::LabelMessage(const LabelMessage &other) : 
            Message(LABEL_MESSAGE_TYPE, LABEL_MESSAGE_VERSION),
            label(other.label),
            fontSize(other.fontSize),
            foreground(other.foreground),
            background(other.background),
            expiration(other.expiration),
            addLabel(other.addLabel),
            layer(other.layer),
            lat(other.lat),
            lng(other.lng),
            alt(other.alt)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool LabelMessage::operator==(const LabelMessage &other) const
        {
            TRACE_ENTER();

            bool retVal;

            // Compare either address or space coords but not both. Address takes precidence.
            if (fromNodeID==NodeIdentifier())
                retVal = 
                    retVal && 
                    lat==other.lat && 
                    lng==other.lng &&
                    alt==other.alt;
            else
                retVal = fromNodeID==other.fromNodeID && label==other.label;

            // These are not distinguishing features
            //  foreground==other.foreground,
            //  background==other.background,
            //  expiration==other.expiration,
            //  fontSize==other.FontSize;

            retVal = retVal && Message::operator==(other); 

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
            layer=other.layer;
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
            if (fromNodeID.to_v4().to_ulong()==0)
                out << " (floating) ";
            else
                out << " (attached) "; 
            out << " font size: " << fontSize; 
            out << " layer: " << layer; 
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

        template <typename Archive> void LabelMessage::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & boost::serialization::base_object<Message>(*this);
            ar & label;
            ar & foreground;
            ar & background;
            ar & expiration;
            ar & fontSize;
            ar & addLabel;
            ar & layer; 
            ar & lat;
            ar & lng;
            ar & alt;
            TRACE_EXIT();
        }

    }
}

BOOST_CLASS_EXPORT(watcher::event::LabelMessage);


