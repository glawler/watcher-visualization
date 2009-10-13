/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/time.h>

#include "watcherSerialize.h"
#include "message.h"
#include "logger.h"

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
            mess.operator<<(out);
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

#if 0
            ia.register_type<StartMessage>();
            ia.register_type<SeekMessage>();
            ia.register_type<StopMessage>();
            ia.register_type<TestMessage>();
#endif

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
