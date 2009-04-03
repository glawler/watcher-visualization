#define BOOST_TEST_MODULE testSeekMessage
#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <libwatcher/seekWatcherMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::watchapi;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    /* Ensure the class can be instantiated */
    SeekMessage m1;

    SeekMessage m2(3);

    SeekMessage m3(8, SeekMessage::cur);

    SeekMessage m4(-42, SeekMessage::end);
}

BOOST_AUTO_TEST_CASE( serialize_test )
{
    SeekMessage msg(-99, SeekMessage::end);
    ostringstream ofs;
    {
        boost::archive::text_oarchive oa(ofs);
        oa << msg;
    }

    istringstream ifs(ofs.str());
    SeekMessage newmsg;
    {
        boost::archive::text_iarchive ia(ifs);
        ia >> newmsg;
    }

    BOOST_CHECK_EQUAL( msg, newmsg );
}
