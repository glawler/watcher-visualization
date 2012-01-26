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
#define BOOST_TEST_MODULE watcher::message test
#include <boost/test/unit_test.hpp>
#include <sstream>

#include "../messageFactory.h"
#include "../messageTypesAndVersions.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( multi_message_stream )
{
	// Do not put multiple messages in a single stream. 
	// (Library limitation.)
	MessagePtr ms[] = { 
		createMessage(MESSAGE_STATUS_TYPE),
		createMessage(CONNECTIVITY_MESSAGE_TYPE)
	}; 

	stringstream ss;
	ms[0]->pack(ss); 
	ms[1]->pack(ss); 

	// When the underlying YAML library fixes issue #148, 
	// this test will fail and I will modify libwatcher 
	// appropriately. 
	MessagePtr to;
	BOOST_REQUIRE_NO_THROW(to=Message::unpack(ss)); 
	BOOST_REQUIRE(to.get() != 0); 
	BOOST_REQUIRE_NO_THROW(to=Message::unpack(ss)); 
	BOOST_REQUIRE(to.get() == 0); 
}

BOOST_AUTO_TEST_CASE( multi_parse_test )
{
	MessagePtr from[] = { 
		createMessage(MESSAGE_STATUS_TYPE), 
 		createMessage(GPS_MESSAGE_TYPE), 
		createMessage(LABEL_MESSAGE_TYPE), 
		createMessage(EDGE_MESSAGE_TYPE), 
		createMessage(COLOR_MESSAGE_TYPE), 
		createMessage(CONNECTIVITY_MESSAGE_TYPE), 
		createMessage(NODE_STATUS_MESSAGE_TYPE), 
		createMessage(DATA_POINT_MESSAGE_TYPE), 
		createMessage(NODE_PROPERTIES_MESSAGE_TYPE), 
		createMessage(SEEK_MESSAGE_TYPE), 
  		createMessage(START_MESSAGE_TYPE), 
		createMessage(STOP_MESSAGE_TYPE), 
		createMessage(SPEED_MESSAGE_TYPE), 
		createMessage(PLAYBACK_TIME_RANGE_MESSAGE_TYPE), 
		createMessage(MESSAGE_STREAM_FILTER_MESSAGE_TYPE), 
		createMessage(SUBSCRIBE_STREAM_MESSAGE_TYPE), 
		createMessage(STREAM_DESCRIPTION_MESSAGE_TYPE), 
		createMessage(LIST_STREAMS_MESSAGE_TYPE), 
	};
	int array_size=sizeof(from)/sizeof(from[0]);

	for (int i=0; i<array_size; i++) {
		MessagePtr to; 
		stringstream ss; 
		BOOST_REQUIRE_NO_THROW(from[i]->pack(ss)); 
		BOOST_TEST_MESSAGE("serialized messge:"); 
		BOOST_TEST_MESSAGE("------------------------------");
		BOOST_TEST_MESSAGE(ss.str()); 
		BOOST_TEST_MESSAGE("------------------------------");
		BOOST_REQUIRE_NO_THROW(to=Message::unpack(ss)); 
		BOOST_REQUIRE(to.get() != 0); 
		BOOST_TEST_MESSAGE("Unpacked message:" << *to); 
		BOOST_TEST_MESSAGE("------------------------------");
		BOOST_TEST_MESSAGE(*to); 
		BOOST_TEST_MESSAGE("------------------------------");
	}

	// unpack one at a time from multi packed stream
	{
		stringstream ss; 
		for (int i=0; i<array_size; i++) 
			from[i]->pack(ss); 

		YAML::Parser parser(ss); 
		YAML::Node node; 
		for (int i=0; i<array_size; i++) {
			parser.GetNextDocument(node); 
			MessagePtr to;
			BOOST_REQUIRE_NO_THROW(to=Message::unpack(node)); 
			BOOST_REQUIRE(to.get()!=0); 
			BOOST_TEST_MESSAGE("Unpacked message:" << *to); 
			BOOST_TEST_MESSAGE("------------------------------");
			BOOST_TEST_MESSAGE(*to); 
			BOOST_TEST_MESSAGE("------------------------------");
		}
	}
}



