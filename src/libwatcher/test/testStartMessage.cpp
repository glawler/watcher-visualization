#define BOOST_TEST_MODULE testStartMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <libwatcher/startWatcherMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::watchapi;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    StartMessage m;
}

BOOST_AUTO_TEST_CASE( serialize_test )
{
    StartMessage msg;
        ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    StartMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> msg;
    }
}
