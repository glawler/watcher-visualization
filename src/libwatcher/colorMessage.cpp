#include <boost/serialization/export.hpp>

#include "watcherGlobalFunctions.h" // for address serialization
#include "colorMessage.h"

using namespace std;
using namespace boost;

namespace watcher {
    namespace event {
        INIT_LOGGER(ColorMessage, "Message.ColorMessage");
        BOOST_CLASS_EXPORT_GUID(ColorMessage, "ColorMessage");

        ColorMessage::ColorMessage() : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            color(Color::black),
            nodeAddr(asio::ip::address::from_string("127.0.0.1")),
            flashPeriod(0),
            expiration(0)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }


        ColorMessage::ColorMessage(
                                   const Color &c, 
                                   const boost::asio::ip::address &address,
                                   const uint32_t &e,
                                   const uint32_t &f) : 
            Message(COLOR_MESSAGE_TYPE, COLOR_MESSAGE_VERSION),
            color(c), 
            nodeAddr(address),
            flashPeriod(e),
            expiration(f)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        ColorMessage::ColorMessage(const ColorMessage &other)
        {
            TRACE_ENTER();
            (*this)=other;
            TRACE_EXIT();
        }

        bool ColorMessage::operator==(const ColorMessage &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                color == other.color;
            nodeAddr == other.nodeAddr;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        ColorMessage &ColorMessage::operator=(const ColorMessage &other)
        {
            TRACE_ENTER();

            Message::operator=(other);
            color = other.color;
            nodeAddr = other.nodeAddr;
            flashPeriod = other.flashPeriod;
            expiration = other.expiration;

            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &ColorMessage::toStream(std::ostream &out) const
        {
            TRACE_ENTER();

            Message::toStream(out);
            out << " color: " << color; 
            out << " node: " << nodeAddr;
            out << " flashPeriod: " << flashPeriod;
            out << " expiration: " << expiration;
            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const ColorMessage &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }

    }
}