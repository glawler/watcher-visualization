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
#include "messageStatus.h"
#include "logger.h"

using namespace std;

namespace watcher {
    namespace event {

        INIT_LOGGER(MessageStatus, "Message.Status");

        MessageStatus::MessageStatus(const Status stat) : 
            Message(MESSAGE_STATUS_TYPE, MESSAGE_STATUS_VERSION),
            status(stat)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        MessageStatus::MessageStatus(const MessageStatus &other) :
            Message(other.type, other.version),
            status(other.status)
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        MessageStatus::~MessageStatus()  
        {
            TRACE_ENTER();
            TRACE_EXIT();
        }

        bool MessageStatus::operator==(const MessageStatus &other) const
        {
            TRACE_ENTER();

            bool retVal = 
                Message::operator==(other) && 
                status==other.status;

            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        MessageStatus &MessageStatus::operator=(const MessageStatus &other)
        {
            TRACE_ENTER();
            Message::operator=(other);
            status=other.status;
            TRACE_EXIT();
            return *this;
        }

        // static
        const char *MessageStatus::statusToString(const Status &status)
        {
            TRACE_ENTER();
            const char *retVal="";
            switch(status)
            {
                case status_ok: retVal="ok"; break;
                case status_error: retVal="error"; break;
                case status_ack: retVal="ack"; break;
                case status_nack: retVal="nack"; break;
                case status_disconnected: retVal="disconnected"; break;
            }
            TRACE_EXIT_RET(retVal);
            return retVal;
        }

        // virtual
        std::ostream &MessageStatus::toStream(std::ostream &out) const
        {
            TRACE_ENTER();
            Message::toStream(out); 
            out << " status : " << statusToString(status);
            TRACE_EXIT();
            return out;
        }

        ostream& operator<<(ostream &out, const MessageStatus &mess)
        {
            mess.operator<<(out);
            return out;
        }

		YAML::Emitter &MessageStatus::serialize(YAML::Emitter &e) const {
			// e << YAML::Comment("MessageStatus"); 
			e << YAML::Flow;
			e << YAML::BeginMap;
			Message::serialize(e); 
			e << YAML::Key << "status" << YAML::Value << (unsigned short&)status; 
			e << YAML::EndMap; 
			return e; 
		}
		YAML::Node &MessageStatus::serialize(YAML::Node &node) {
			// Do not serialize base data GTL - Message::serialize(node); 
			unsigned short tmp;
			node["status"] >> tmp;
			status=(Status)tmp; 
			return node;
		}
    }
}

