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

#include "message.h"
#include "marshalYAML.h"
#include "logger.h"
#include "messageFactory.h"

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

		MessagePtr Message::unpack(istream &in) { 
			YAML::Parser parser(in); 
			YAML::Node node; 
			parser.GetNextDocument(node); 
			MessagePtr retVal=Message::unpack(node);  // do not create unnamed smart_ptrs. 
			return retVal; 
		}

		MessagePtr Message::unpack(YAML::Node &node) { 
			// This is a little awkward: read the base message, create a derived message, 
			// then copy the base data into the derived data. 
			MessagePtr header(new Message); 
			try { 
				header->serialize(node); 
			}
			catch (YAML::ParserException &e) {
				return MessagePtr();  // equiv to NULL
			}
			catch (std::runtime_error &e) {
				LOG_WARN("Unable to parse incoming message, may have put more than one message in a stream."); 
				return MessagePtr(); 
			}

			// create derived message instance based on type. 
			MessagePtr m(createMessage(header->type)); 

			// We use the data already read from the stream to 
			// populate the base class data in the derived class 
			// instance. This is not really a good thing, but due to
			// limitions of the yaml-cpp library (see issue #148) and
			// my lack of cleverness, it'll have to do "for now". 
			// NOTE: typecast derived to base for oper=().
			(static_cast<MessagePtr>(m))->operator=(*header);  

			// Now read the rest of the YAML doc that contains the 
			// data for the derived Message instance. 
			try { 
				m->serialize(node); 
			}
			catch (YAML::ParserException &e) {
				return MessagePtr();  // equiv to NULL
			}
			return m; 
		}

		void Message::pack(std::ostream& os) const
		{
			YAML::Emitter emitter; 
			bool err=false; 
			try {
				this->serialize(emitter); 
			}
			catch (YAML::EmitterException) {
				err=true;
			}
			if (!err)
				os.write(emitter.c_str(), emitter.size()); 
			
			LOG_DEBUG("serialized message: " << emitter.c_str()); 
		}

		YAML::Emitter &Message::serialize(YAML::Emitter &e) const {
			// e << YAML::Comment("Message"); 
			// e << YAML::BeginDoc; 
			// e << YAML::Flow << YAML::BeginMap;
			e << YAML::Key << "version" << YAML::Value << version;
			e << YAML::Key << "type" << YAML::Value << (const unsigned int&)type;
			e << YAML::Key << "timestamp" << YAML::Value << timestamp;
			e << YAML::Key << "fromNodeID" << YAML::Value << fromNodeID.to_string(); 
			// e << YAML::EndMap; 
			// e << YAML::EndDoc; 
			return e; 
		}
		YAML::Node &Message::serialize(YAML::Node &node) {
			node["version"] >> version;
			node["type"] >> (unsigned int&)type;
			node["timestamp"] >> timestamp;
			string str;
			node["fromNodeID"] >> str;
			fromNodeID=NodeIdentifier::from_string(str); 
			return node;
		}
	}
}

