#define BOOST_TEST_MODULE testSpeedMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

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
