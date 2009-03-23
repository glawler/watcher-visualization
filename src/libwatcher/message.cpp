#include <sys/time.h>
#include <libwatcher/message.h>

#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/export.hpp>

using namespace std;

namespace watcher {
    namespace event {
        INIT_LOGGER(Message, "Message");

        BOOST_CLASS_EXPORT_GUID(Message, "Message");

        Message::Message() : version(0), type(UNKNOWN_MESSAGE_TYPE)
        {
            TRACE_ENTER();
            struct timeval tp;
            gettimeofday(&tp, NULL);
            timestamp = (long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;
            TRACE_EXIT(); 
        }

        Message::Message(const MessageType &t, const unsigned int v) : 
            version(v), type(t)
        {
            TRACE_ENTER();
            struct timeval tp;
            gettimeofday(&tp, NULL);
            timestamp = (long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;
            TRACE_EXIT();
        }

        Message::Message(const Message &other)
        {
            TRACE_ENTER();
            (*this)=other;
            TRACE_EXIT();
        }

        Message::~Message()
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool Message::operator==(const Message &other) const
        {
            TRACE_ENTER();
            bool retVal = version==other.version && type==other.type;
            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        Message &Message::operator=(const Message &other)
        {
            TRACE_ENTER();
            version=other.version;
            type=other.type;
            timestamp=other.timestamp;
            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &Message::toStream(std::ostream &out) const
        {
            TRACE_ENTER();
            out << " version: " << version << " type: " << type << " time: " << timestamp << " "; 
            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const Message &mess)
        {
            TRACE_ENTER();
            mess.operator<<(out);
            TRACE_EXIT();
            return out;
        }
    }
}
