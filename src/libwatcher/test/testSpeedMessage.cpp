#define BOOST_TEST_MODULE testSpeedMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <sstream>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>

#include <libwatcher/speedWatcherMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    SpeedMessage m1;

    SpeedMessage m2(42);

    SpeedMessage m3(-98);
}

BOOST_AUTO_TEST_CASE( serialize_test )
{
    SpeedMessage msg(37);
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    SpeedMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_CHECK_EQUAL( msg, newmsg );
}

BOOST_AUTO_TEST_CASE ( shared_archive_test )
{
    SpeedMessagePtr msg(new SpeedMessage(37));
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    SpeedMessagePtr newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_CHECK_EQUAL( *msg, *newmsg );
}

BOOST_AUTO_TEST_CASE ( shared_archive_base_test )
{
    MessagePtr msg(new SpeedMessage(37));
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    MessagePtr newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    SpeedMessagePtr dmsg = boost::dynamic_pointer_cast<SpeedMessage>(msg);
    BOOST_CHECK_NE( *dmsg, 0 );
    SpeedMessagePtr dnewmsg = boost::dynamic_pointer_cast<SpeedMessage>(newmsg);
    BOOST_CHECK_NE( *dnewmsg, 0 );

    BOOST_CHECK_EQUAL( *dmsg, *dnewmsg );
}

BOOST_AUTO_TEST_CASE ( shared_poly_archive_base_test )
{
    MessagePtr msg(new SpeedMessage(37));
    ostringstream ofs;
    {
        boost::archive::polymorphic_text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    MessagePtr newmsg;
    {
        boost::archive::polymorphic_text_iarchive ia(ifs);
        ia >> newmsg;
    }

    SpeedMessagePtr dmsg = boost::dynamic_pointer_cast<SpeedMessage>(msg);
    BOOST_CHECK_NE( *dmsg, 0 );
    SpeedMessagePtr dnewmsg = boost::dynamic_pointer_cast<SpeedMessage>(newmsg);
    BOOST_CHECK_NE( *dnewmsg, 0 );

    BOOST_CHECK_EQUAL( *dmsg, *dnewmsg );
}
