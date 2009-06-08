#include <sys/time.h>

#include "watcherSerialize.h"
#include "message.h"

using namespace std;

namespace watcher {
    namespace event {
        INIT_LOGGER(Message, "Message");


        Message::Message() : version(0), type(UNKNOWN_MESSAGE_TYPE), timestamp(0)
        {
            TRACE_ENTER();
            struct timeval tp;
            gettimeofday(&tp, NULL);
            timestamp = (long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;
            TRACE_EXIT(); 
        }

        Message::Message(const MessageType &t, const unsigned int v) : 
            version(v), type(t), timestamp(0)
        {
            TRACE_ENTER();
            struct timeval tp;
            gettimeofday(&tp, NULL);
            timestamp = (long long int)tp.tv_sec * 1000 + (long long int)tp.tv_usec/1000;
            TRACE_EXIT();
        }

        Message::Message(const Message &other) :
            version(other.version), 
            type(other.type), 
            timestamp(other.timestamp), 
            fromNodeID(other.fromNodeID)
        {
            TRACE_ENTER();
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
            fromNodeID=other.fromNodeID;
            TRACE_EXIT();
            return *this;
        }

        // virtual 
        std::ostream &Message::toStream(std::ostream &out) const
        {
            TRACE_ENTER();
            out << "from: " << fromNodeID << " version: " << version << " type: " << type << " time: " << timestamp << " "; 
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

        template <typename Archive> void Message::serialize(Archive & ar, const unsigned int /* file_version */)
        {
            TRACE_ENTER();
            ar & version;
            ar & type;
            ar & timestamp;
            ar & fromNodeID;
            TRACE_EXIT();
        }

        MessagePtr Message::unpack(std::istream& is)
        {
            TRACE_ENTER();
            is >> std::setprecision(std::numeric_limits<double>::digits10 + 2); 
            boost::archive::text_iarchive ia(is);
            Message* ret = 0;
            try
            {
                ia >> ret;
            }
            catch (boost::archive::archive_exception& e)
            {
                LOG_WARN("Exception thrown while deserializing the message: " << e.what());
                TRACE_EXIT();
                return MessagePtr();
            }
            TRACE_EXIT();
            return MessagePtr(ret); 
        }

        void Message::pack(std::ostream& os) const
        {
            TRACE_ENTER();
            os << std::setprecision(std::numeric_limits<double>::digits10 + 2); 
            boost::archive::text_oarchive oa(os);
            const Message* base = this;
            oa << base;
            TRACE_EXIT();
        }
    }
}

BOOST_CLASS_EXPORT(watcher::event::Message);
