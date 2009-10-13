/** 
 * @file testConnectivityMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::Message.ConnectivityMessage test

#include <boost/test/unit_test.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#include <libwatcher/connectivityMessage.h>
#include <libwatcher/watcherGlobalFunctions.h>
#include <logger.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

DECLARE_GLOBAL_LOGGER("testConnectivityMessage"); 

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties");    // must be in first test case

    /* Ensure the class can be instantiated */
    BOOST_REQUIRE_NO_THROW( watcher::event::ConnectivityMessage() );
    watcher::event::ConnectivityMessage m;
    BOOST_TEST_MESSAGE("m=" << m);
}

BOOST_AUTO_TEST_CASE( pack_test )
{
    ConnectivityMessagePtr cm( new ConnectivityMessage );
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.101"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.102"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.103"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.104"));
    cm->neighbors.push_back(NodeIdentifier::from_string("192.168.1.105"));

    ostringstream os;
    cm->pack(os);

    LOG_DEBUG("the message: " << *cm); 

    BOOST_TEST_MESSAGE("serialized: " << os.str());
    LOG_DEBUG("serialized: " << os.str());
    istringstream is(os.str());
    MessagePtr newmsg = Message::unpack(is);
    BOOST_REQUIRE(newmsg.get() != 0);

    ConnectivityMessagePtr pnewmsg = dynamic_pointer_cast<ConnectivityMessage>(newmsg);
    BOOST_REQUIRE(pnewmsg.get() != 0);

    BOOST_CHECK_EQUAL(*cm, *pnewmsg);

    LOG_DEBUG("cm:" << *cm);
    LOG_DEBUG("pnewmsg:" << *pnewmsg);

    // ConnectivityMessagePtr cmp(new ConnectivityMessage);
    // istringstream is2(os.str());
    // cmp->unpack(is2);
    // BOOST_CHECK_EQUAL(*cm, *cmp);
}

