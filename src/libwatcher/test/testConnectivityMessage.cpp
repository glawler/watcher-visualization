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
/** 
 * @file testConnectivityMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::Message.ConnectivityMessage test

#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include "../connectivityMessage.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    BOOST_REQUIRE_NO_THROW( watcher::event::ConnectivityMessage() );
    watcher::event::ConnectivityMessage m;
    BOOST_TEST_MESSAGE("m=" << m);
}

BOOST_AUTO_TEST_CASE( pack_test )
{
    ConnectivityMessagePtr cm( new ConnectivityMessage );
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.101"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.102"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.103"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.104"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.105"));

    ostringstream os;
    cm->pack(os);

    BOOST_TEST_MESSAGE("the message: " << *cm); 

    BOOST_TEST_MESSAGE("serialized: " << os.str());
    BOOST_TEST_MESSAGE("serialized: " << os.str());
    istringstream is(os.str());
    MessagePtr newmsg = Message::unpack(is);
    BOOST_REQUIRE(newmsg.get() != 0);

    ConnectivityMessagePtr pnewmsg = dynamic_pointer_cast<ConnectivityMessage>(newmsg);
    BOOST_REQUIRE(pnewmsg.get() != 0);

    BOOST_CHECK_EQUAL(*cm, *pnewmsg);

    BOOST_TEST_MESSAGE("cm:" << *cm);
    BOOST_TEST_MESSAGE("pnewmsg:" << *pnewmsg);

    // ConnectivityMessagePtr cmp(new ConnectivityMessage);
    // istringstream is2(os.str());
    // cmp->unpack(is2);
    // BOOST_CHECK_EQUAL(*cm, *cmp);
}

