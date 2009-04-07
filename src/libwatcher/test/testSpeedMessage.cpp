#define BOOST_TEST_MODULE testSpeedMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>

#include <libwatcher/speedWatcherMessage.h>

#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

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

#if 0
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
    BOOST_CHECK_NE( dmsg.get(), static_cast<SpeedMessage*>(0) );
    BOOST_TEST_MESSAGE( "dmsg->speed=" << dmsg->speed );
    SpeedMessagePtr dnewmsg = boost::dynamic_pointer_cast<SpeedMessage>(newmsg);
    BOOST_CHECK_NE( dnewmsg.get(), static_cast<SpeedMessage*>(0) );
    BOOST_TEST_MESSAGE( "dnewmsg->speed=" << dnewmsg->speed );

    BOOST_CHECK_EQUAL( *dmsg, *dnewmsg );

    // GTL - check that the pointers are pointing to correct vtables
    LOG_DEBUG("     *msg: " << *msg); 
    LOG_DEBUG("  *newmsg: " << *newmsg); 
    LOG_DEBUG("    *dmsg: " << *dmsg); 
    LOG_DEBUG(" *dnewmsg: " << *dnewmsg); 

    string dStr("SpeedMessage(speed=37)");  // This is the output of operator<< on derived speedTest
    ostringstream boutput, doutput;
    boutput << *newmsg;
    doutput << *dnewmsg; 

    BOOST_CHECK_NE( dStr, boutput.str() );
    BOOST_CHECK_EQUAL( dStr, doutput.str() );

    LOG_DEBUG("   base: " << boutput.str()); 
    LOG_DEBUG("derived: " << doutput.str()); 
}
#endif

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
    BOOST_TEST_MESSAGE( "dmsg->speed=" << dmsg->speed );
    SpeedMessagePtr dnewmsg = boost::dynamic_pointer_cast<SpeedMessage>(newmsg);
    BOOST_CHECK_NE( *dnewmsg, 0 );
    BOOST_TEST_MESSAGE( "dnewmsg->speed=" << dnewmsg->speed );

    BOOST_CHECK_EQUAL( *dmsg, *dnewmsg );
}
