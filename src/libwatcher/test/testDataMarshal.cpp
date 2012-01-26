/* Copyright 2012 SPARTA, Inc., dba Cobham Analytic Solutions
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
#define BOOST_TEST_MODULE watcher::DataMarshaller test
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <sstream>

#include "../messageFactory.h"
#include "../messageTypesAndVersions.h"
#include "../labelMessage.h"
#include "../dataMarshaller.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( single_message_buffer ) {
	LabelMessagePtr m_message(LabelMessagePtr(
				new LabelMessage("Hello World", 1.23, 2.34, 3.45))); 
	BOOST_TEST_MESSAGE("To Marshal:" << *m_message); 
    DataMarshaller::NetworkMarshalBuffers data;
	bool retVal=DataMarshaller::marshalPayload(m_message, data); 
	BOOST_REQUIRE(retVal==true); 

	// first buffer is header, others are messages. 
	const char *buffer=(const char*)
				boost::asio::buffer_cast<const unsigned char*>(data[0]); 
	size_t buffer_size=boost::asio::buffer_size(data[0]); 
	size_t payload_size; 
	unsigned short message_num; 
	retVal=DataMarshaller::unmarshalHeader(buffer, buffer_size, payload_size, message_num); 
	BOOST_REQUIRE(retVal==true); 
	BOOST_REQUIRE(payload_size!=0); 
	BOOST_REQUIRE(message_num==1); 

	BOOST_TEST_MESSAGE("payload size: " << payload_size << ", # messages: " << message_num); 

	buffer=(const char*)boost::asio::buffer_cast<const unsigned char*>(data[1]); 
	buffer_size=boost::asio::buffer_size(data[1]); 
	MessagePtr um_message; 
	retVal=DataMarshaller::unmarshalPayload(um_message, buffer, buffer_size); 
	BOOST_REQUIRE(retVal==true); 
	BOOST_REQUIRE(um_message.get()!=0); 

	LabelMessagePtr label_message(boost::dynamic_pointer_cast<LabelMessage>(um_message)); 
	BOOST_REQUIRE(*m_message == *label_message); 
	BOOST_TEST_MESSAGE("Unmarshalled:" << *label_message); 
}

// BOOST_AUTO_TEST_CASE( multi_message ) {
// 
// 	std::vector<MessagePtr> messages = {
// 		createMessage(GPS_MESSAGE_TYPE), 
// 		createMessage(CONNECTIVITY_MESSAGE_TYPE), 
// 		createMessage(LABEL_MESSAGE_TYPE), 
// 		createMessage(NODE_PROPERTIES_MESSAGE_TYPE), 
// 	}; 
// 
// 	// Marshal all messages at once. 
//     DataMarshaller::NetworkMarshalBuffers data;
// 	bool retVal=DataMarshaller::marshalPayload(messages, data); 
// 	BOOST_CHECK_EQUAL(retVal, true); 
// 
// 	// first buffer is header, others are messages. 
// 	const char *buffer=(const char*)
// 				boost::asio::buffer_cast<const unsigned char*>(data[0]); 
// 	size_t buffer_size=boost::asio::buffer_size(data[0]); 
// 	size_t payload_size; 
// 	unsigned short message_num; 
// 	retVal=DataMarshaller::unmarshalHeader(buffer, buffer_size, payload_size, message_num); 
// 	BOOST_REQUIRE(retVal==true); 
// 	BOOST_REQUIRE(payload_size!=0); 
// 	BOOST_REQUIRE(message_num==messages.size()); 
// 	BOOST_TEST_MESSAGE("payload size: " << payload_size << ", # messages: " << message_num); 
// 
// 
// 	buffer=(const char*)boost::asio::buffer_cast<const unsigned char*>(data[1]); 
// 	buffer_size=boost::asio::buffer_size(data[1]); 
// 	vector<MessagePtr> um_messages; 
// 	retVal=DataMarshaller::unmarshalPayload(um_messages, message_num, buffer, buffer_size); 
// 	BOOST_REQUIRE(retVal==true); 
// 	BOOST_REQUIRE(um_messages.size()==messages.size()); 
// 
// 	for (int i=0; i<um_messages.size(); i++) { 
// 		BOOST_REQUIRE(um_messages[i].get()!=0); 
// 		BOOST_REQUIRE(um_messages[i]->operator==(*messages[i]) == true); 
// 		BOOST_TEST_MESSAGE("Unmarshalled:" << *um_messages[i]); 
// 	}
// 
// }
