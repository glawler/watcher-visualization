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
#define BOOST_TEST_MODULE watcher::Message.Message test

#include <boost/test/unit_test.hpp>
#include <libwatcher/message.h>
#include <logger.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

DECLARE_GLOBAL_LOGGER("MessageTest"); 

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties");    // must be in first test case

    /* Ensure the class can be instantiated */
    BOOST_REQUIRE_NO_THROW(Message());
    Message m;
    BOOST_TEST_MESSAGE("m=" << m);
}

BOOST_AUTO_TEST_CASE( serialize_test )
{
    Message mOut(CONNECTIVITY_MESSAGE_TYPE, CONNECTIVITY_MESSAGE_VERSION);
    mOut.fromNodeID=NodeIdentifier::from_string("192.168.1.100"); 
    LOG_DEBUG("mOut=" << mOut);

    ostringstream o;
    mOut.serialize(o);

    Message mIn;
    LOG_DEBUG("mIn (before) =" << mIn);
    istringstream i(o.str()); 
    mIn.unserialize(i); 
    LOG_DEBUG("mIn (after) =" << mIn);

    BOOST_CHECK_EQUAL(mOut, mIn); 
}
