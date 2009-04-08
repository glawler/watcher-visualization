#define BOOST_TEST_MODULE testStartMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <libwatcher/startWatcherMessage.h>

using namespace std;
using namespace watcher::event;
using namespace boost;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    BOOST_REQUIRE_NO_THROW( watcher::event::StartMessage() );
    watcher::event::StartMessage m;
    BOOST_TEST_MESSAGE("m=" << m);
}

BOOST_AUTO_TEST_CASE( pack_test )
{
    StartMessagePtr msg( new StartMessage );
    ostringstream os;
    msg->pack(os);

    BOOST_TEST_MESSAGE("serialized: " << os.str());
    istringstream is(os.str());
    MessagePtr newmsg = Message::unpack(is);
    BOOST_REQUIRE(newmsg.get() != 0);

    StartMessagePtr pnewmsg = dynamic_pointer_cast<StartMessage>(newmsg);
    BOOST_REQUIRE(pnewmsg.get() != 0);

    BOOST_CHECK_EQUAL(*msg, *pnewmsg);
}

#if 0
BOOST_AUTO_TEST_CASE( serialize_test )
{
    watcher::event::StartMessage msg;
    std::ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    std::istringstream ifs(ofs.str());
    watcher::event::StartMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> msg;
    }

    BOOST_CHECK_EQUAL( msg, newmsg );
}

BOOST_AUTO_TEST_CASE ( base_shared_archive_test )
{
    // OUTPUT
    watcher::event::MessagePtr msg(new watcher::event::StartMessage());
    BOOST_REQUIRE(msg.get() != 0);
    watcher::event::StartMessagePtr pmsg = boost::dynamic_pointer_cast<watcher::event::StartMessage>(msg);
    BOOST_REQUIRE( pmsg.get() != 0 );
    BOOST_TEST_MESSAGE("pmsg: " << *pmsg);

    //serialize
    std::ostringstream ofs;
    boost::archive::text_oarchive oa(ofs);
    oa << msg;

    // INPUT
    BOOST_TEST_MESSAGE("Testing input of archive via shared pointer to base class");
    std::istringstream ifs(ofs.str());
    watcher::event::MessagePtr newmsg;
    boost::archive::text_iarchive ia(ifs);
    ia >> newmsg;
    BOOST_REQUIRE(newmsg.get() != 0);
    watcher::event::StartMessagePtr pnewmsg = boost::dynamic_pointer_cast<watcher::event::StartMessage>(newmsg);
    BOOST_REQUIRE( pnewmsg.get() != 0 );
    BOOST_TEST_MESSAGE("pnewmsg: " << *pnewmsg);
    BOOST_CHECK_EQUAL( *pmsg, *pnewmsg );
}
#endif
