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
 * @file testStopMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE testStopMessage
#include <boost/test/unit_test.hpp>
#if 0
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#endif
#include <sstream>

#include <libwatcher/stopWatcherMessage.h>

using namespace std;
using namespace watcher::event;
using namespace boost;
//using namespace boost::archive;
//using namespace boost::serialization;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    BOOST_REQUIRE_NO_THROW(StopMessage());
}

BOOST_AUTO_TEST_CASE( pack_test )
{
    BOOST_TEST_MESSAGE("serializing shared pointer to base class");
    StopMessagePtr msg( new StopMessage );
    ostringstream os;
    msg->pack(os);

    BOOST_TEST_MESSAGE("serialized: " << os.str());

    BOOST_TEST_MESSAGE("de-serializing shared pointer to base class");
    istringstream is(os.str());
    MessagePtr newmsg = Message::unpack(is);
    BOOST_REQUIRE(newmsg.get() != 0);

    StopMessagePtr pnewmsg = dynamic_pointer_cast<StopMessage>(newmsg);
    BOOST_REQUIRE(pnewmsg.get() != 0);

    BOOST_CHECK_EQUAL(*msg, *pnewmsg);
}

#if 0
BOOST_AUTO_TEST_CASE( serialize_test )
{
    StopMessage msg;
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    StopMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> msg;
    }
}

BOOST_AUTO_TEST_CASE ( base_shared_archive_test )
{
    MessagePtr msg(new StopMessage());
    ostringstream ofs;
    {
        text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    MessagePtr newmsg;
    {
        text_iarchive ia(ifs);
        ia >> newmsg;
    }

    StopMessagePtr pnewmsg = dynamic_pointer_cast<StopMessage>(newmsg);
    BOOST_REQUIRE( pnewmsg.get() != 0);

    StopMessagePtr pmsg = dynamic_pointer_cast<StopMessage>(msg);
    BOOST_REQUIRE( pmsg.get() != 0);

    BOOST_TEST_MESSAGE("pmsg: " << *pmsg);
    BOOST_TEST_MESSAGE("pnewmsg: " << *pnewmsg);

    BOOST_CHECK_EQUAL( *pmsg, *pnewmsg );
}
#endif
