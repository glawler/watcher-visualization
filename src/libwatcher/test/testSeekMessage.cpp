#define BOOST_TEST_MODULE testSeekMessage
#include <boost/test/unit_test.hpp>
#if 0
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#endif
#include <sstream>

#include <libwatcher/seekWatcherMessage.h>

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
    msg->pack(os);

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
