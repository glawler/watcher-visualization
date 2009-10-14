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
#include <iomanip>

#include "message.h"
#include "logger.h"
#include "marshal.hpp"

#include "libwatcher/messageStatus.h"

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

        //default empty implementation, should be overriden by subclasses
        std::ostream& Message::packPayload (std::ostream& os) const
        {
            TRACE_ENTER();
            TRACE_EXIT();
            return os;
        }

        Marshal::Input& operator>> (Marshal::Input& in, MessageType& t)
        {
            int msgtype;
            in >> msgtype;

            // TODO add the other message types - melkins
            switch (msgtype) {
                case UNKNOWN_MESSAGE_TYPE:
                    t = UNKNOWN_MESSAGE_TYPE;
                default:
                    throw std::runtime_error("invalid message type in message");
            }
            return in;
        }

        Marshal::Input& operator>> (Marshal::Input& in, NodeIdentifier& id)
        {
            std::string s;
            in >> s;
            id = boost::asio::ip::address::from_string(s);
            return in;
        }

        MessagePtr Message::create(MessageType t)
        {
            MessagePtr ret;
            // TODO maybe this should be a map instead of a swich statement? - melkins
            switch (t) {
                case MESSAGE_STATUS_TYPE:
                    ret.reset( new MessageStatus() );
                default:
                    throw std::runtime_error("invalid message type in message");
            }
            return ret;
        }

        void Message::readPayload(Marshal::Input& is)
        {
            // empty default implementation
            TRACE_ENTER();
            TRACE_EXIT();
        }

        MessagePtr Message::unpack(std::istream& is)
        {
            TRACE_ENTER();
            is >> std::setprecision(std::numeric_limits<double>::digits10 + 2); 

            unsigned int version;
            MessageType type;

            Marshal::Input marshal(is);
            marshal >> version;
            marshal >> type;

            // construct a new message by type
            MessagePtr ret = Message::create(type);

            // fill in values already read
            ret->version = version;
            ret->type = type;

            // complete reading header
            marshal >> ret->timestamp;
            marshal >> ret->fromNodeID;

            // read message-specific message payload via virtual method
            ret->readPayload(marshal);

            TRACE_EXIT();
            return ret; 
        }

        std::ostream& Message::pack(std::ostream& os) const
        {
            TRACE_ENTER();
            os << std::setprecision(std::numeric_limits<double>::digits10 + 2); 

            Marshal::Output out(os);
            out << version;
            out << type;
            out << timestamp;
            out << fromNodeID;

            // pack message-specific payload after header
            packPayload(os);

            TRACE_EXIT();

            return os;
        }
    }
}
