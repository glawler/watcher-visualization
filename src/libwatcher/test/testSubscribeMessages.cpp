/* Copyright 2012 SPARTA, Inc.
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
#define BOOST_TEST_MODULE subscribe_messages_test test

#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include "../subscribeStreamMessage.h"
#include "../listStreamsMessage.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    BOOST_REQUIRE_NO_THROW( watcher::event::SubscribeStreamMessage() );
    watcher::event::SubscribeStreamMessage ssm;
    BOOST_TEST_MESSAGE("ssm=" << ssm);

    BOOST_REQUIRE_NO_THROW( watcher::event::ListStreamsMessage() );
    watcher::event::ListStreamsMessage lsm;
    BOOST_TEST_MESSAGE("lsm=" << lsm);
}

BOOST_AUTO_TEST_CASE( pack_test_subscribe ) {
	SubscribeStreamMessagePtr messages[] = { 
		SubscribeStreamMessagePtr(new SubscribeStreamMessage),
		SubscribeStreamMessagePtr(new SubscribeStreamMessage(123)),
		SubscribeStreamMessagePtr(new SubscribeStreamMessage(INT_MAX)),
	};
	for (int i=0; i<sizeof(messages)/sizeof(messages[0]); i++) {
		ostringstream os;
		messages[i]->pack(os);
		BOOST_TEST_MESSAGE("flattened message: " << os.str()); 
		istringstream is(os.str());
		MessagePtr newmsg = Message::unpack(is);
		BOOST_REQUIRE(newmsg.get() != 0);

		SubscribeStreamMessagePtr pnewmsg = dynamic_pointer_cast<SubscribeStreamMessage>(newmsg);
		BOOST_REQUIRE(pnewmsg.get() != 0);
		BOOST_CHECK_EQUAL(*messages[i], *pnewmsg);
		BOOST_TEST_MESSAGE("message:" << *messages[i]);
		BOOST_TEST_MESSAGE("new message:" << *pnewmsg);
	}
}

BOOST_AUTO_TEST_CASE( pack_test_list ) {
	ListStreamsMessagePtr messages[] = { 
		ListStreamsMessagePtr(new ListStreamsMessage),		// empty
		ListStreamsMessagePtr(new ListStreamsMessage),		// will be not-empty
	};
	std::vector<EventStreamInfoPtr> evstreams = { 
		EventStreamInfoPtr(new EventStreamInfo(1, "one")), 
		EventStreamInfoPtr(new EventStreamInfo(2, "two")), 
		EventStreamInfoPtr(new EventStreamInfo(3, "three")), 
		EventStreamInfoPtr(new EventStreamInfo(4, "four")), 
		EventStreamInfoPtr(new EventStreamInfo(666, "beast")), 
	}; 
	messages[1]->evstreams=evstreams; 

	for (int i=0; i<sizeof(messages)/sizeof(messages[0]); i++) {
		ostringstream os;
		messages[i]->pack(os);
		BOOST_TEST_MESSAGE("flattened message: " << os.str()); 
		istringstream is(os.str());
		MessagePtr newmsg = Message::unpack(is);
		BOOST_REQUIRE(newmsg.get() != 0);

		ListStreamsMessagePtr pnewmsg = dynamic_pointer_cast<ListStreamsMessage>(newmsg);
		BOOST_REQUIRE(pnewmsg.get() != 0);
		BOOST_CHECK_EQUAL(*messages[i], *pnewmsg);
		BOOST_TEST_MESSAGE("message:" << *messages[i]);
		BOOST_TEST_MESSAGE("new message:" << *pnewmsg);
	}
}
