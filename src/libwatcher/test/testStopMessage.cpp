#define BOOST_TEST_MODULE testStopMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <libwatcher/stopWatcherMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    StopMessage m;
}

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
