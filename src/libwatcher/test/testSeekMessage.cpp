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
 * @file testSeekMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE testSeekMessage
#include <boost/test/unit_test.hpp>
#include <sstream>

#include <../seekWatcherMessage.h>

using namespace std;
//using namespace watcher;
using namespace watcher::event;
using namespace boost;
//using namespace boost::archive;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    SeekMessage m1;

    SeekMessage m2(3);

    SeekMessage m3(8, SeekMessage::cur);

    SeekMessage m4(-42, SeekMessage::end);
}

BOOST_AUTO_TEST_CASE( pack_test )
{
    BOOST_TEST_MESSAGE("serializing shared pointer to base class");
    SeekMessagePtr msg( new SeekMessage );
    ostringstream os;
    msg.get()->pack(os);

    BOOST_TEST_MESSAGE("serialized: " << os.str());

    BOOST_TEST_MESSAGE("de-serializing shared pointer to base class");
    istringstream is(os.str());
    MessagePtr newmsg = Message::unpack(is);
    BOOST_REQUIRE(newmsg.get() != 0);

    SeekMessagePtr pnewmsg = dynamic_pointer_cast<SeekMessage>(newmsg);
    BOOST_REQUIRE(pnewmsg.get() != 0);

    BOOST_CHECK_EQUAL(*msg, *pnewmsg);
}

#if 0
BOOST_AUTO_TEST_CASE( serialize_test )
{
    SeekMessage msg(-99, SeekMessage::end);
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    BOOST_TEST_MESSAGE("serialized: " << ofs.str());
    SeekMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_CHECK_EQUAL( msg, newmsg );
}

BOOST_AUTO_TEST_CASE ( shared_archive_test )
{
    SeekMessagePtr msg(new SeekMessage(-42, SeekMessage::end));
    ostringstream ofs;
    {
        text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    BOOST_TEST_MESSAGE("serialized: " << ofs.str());

    SeekMessagePtr newmsg;
    {
        text_iarchive ia(ifs);
        ia >> newmsg;
    }
    BOOST_TEST_MESSAGE("newmsg: " << *newmsg);

    BOOST_CHECK_EQUAL( *msg, *newmsg );
}

BOOST_AUTO_TEST_CASE ( base_shared_archive_test )
{
    MessagePtr msg(new SeekMessage(-42, SeekMessage::end));
    BOOST_TEST_MESSAGE("msg: " << *msg); //SLICE
    ostringstream ofs;
    {
        text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    BOOST_TEST_MESSAGE("serialized: " << ofs.str());
    MessagePtr newmsg;
    {
        text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_TEST_MESSAGE("newmsg: " << *newmsg);//SLICE

    SeekMessagePtr pnewmsg = dynamic_pointer_cast<SeekMessage>(newmsg);
    BOOST_CHECK_NE( pnewmsg.get(), static_cast<SeekMessage*>(0) );

    SeekMessagePtr pmsg = dynamic_pointer_cast<SeekMessage>(msg);
    BOOST_CHECK_NE( pmsg.get(), static_cast<SeekMessage*>(0) );

    BOOST_TEST_MESSAGE("pmsg: " << *pmsg);
    BOOST_TEST_MESSAGE("pnewmsg: " << *pnewmsg);

    BOOST_CHECK_EQUAL( *pmsg, *pnewmsg );
}

BOOST_AUTO_TEST_CASE ( base_shared_archive_test_2 )
{
    BOOST_TEST_MESSAGE("serialize via derived, de-serialize via base");

    SeekMessagePtr msg(new SeekMessage(-42, SeekMessage::end));
    BOOST_TEST_MESSAGE("msg: " << *msg); //SLICE
    ostringstream ofs;
    {
        text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    BOOST_TEST_MESSAGE("serialized: " << ofs.str());
    MessagePtr newmsg;
    {
        text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_TEST_MESSAGE("newmsg: " << *newmsg);//SLICE

    SeekMessagePtr pnewmsg = dynamic_pointer_cast<SeekMessage>(newmsg);
    BOOST_REQUIRE( pnewmsg.get() != 0 );

    BOOST_TEST_MESSAGE("pnewmsg: " << *pnewmsg);

    BOOST_CHECK_EQUAL( *msg, *pnewmsg );
}
#endif
