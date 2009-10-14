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
#include "watcherMarshal.h"

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

        // manipulator for reading a MessageType
        Marshal::Input& operator>> (Marshal::Input& in, MessageType& t)
        {
            int msgtype;
            in >> msgtype;

            MessageType tmp = UNKNOWN_MESSAGE_TYPE;

#define CASE(X) case X: tmp = X; break;
            // TODO add the other message types - melkins
            switch (msgtype) {
                CASE( TEST_MESSAGE_TYPE );
                CASE( MESSAGE_STATUS_TYPE );
                CASE( GPS_MESSAGE_TYPE );
                CASE( LABEL_MESSAGE_TYPE );
                CASE( EDGE_MESSAGE_TYPE );
                CASE( COLOR_MESSAGE_TYPE );
                CASE( CONNECTIVITY_MESSAGE_TYPE );
                CASE( SEEK_MESSAGE_TYPE );
                CASE( START_MESSAGE_TYPE );
                CASE( STOP_MESSAGE_TYPE );
                CASE( SPEED_MESSAGE_TYPE );
                CASE( NODE_STATUS_MESSAGE_TYPE );
                // no default case so the compiler emits warnings for unhandled types
            }
#undef CASE

            if (tmp == UNKNOWN_MESSAGE_TYPE)
                throw std::runtime_error("invalid message type in message");

            t = tmp;

            return in;
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

            // complete reading the header
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
