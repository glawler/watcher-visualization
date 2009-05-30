#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::Message.TestMessage test
#include <boost/test/unit_test.hpp>

#include "libwatcher/testMessage.h"
#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("test.log.properties"); 

    string strVal = "who tests the testTestMessage?";
    vector<int> ints;
    for(int i = 1; i <= 10; i++)
        ints.push_back(i); 
    TestMessage tm(strVal, ints); 
}

BOOST_AUTO_TEST_CASE( archive_test )
{
    LOG_INFO("-----------------------Testing testMessage archive"); 

}

BOOST_AUTO_TEST_CASE( shared_ptr_archive_test )
{
    LOG_INFO("------------------------------------------Testing LabelMessagePtr archiving..."); 

    string strVal = "who tests the testTestMessage?";
    vector<int> ints;
    for(int i = 1; i <= 10; i++)
        ints.push_back(i); 

    {
        TestMessage tmOut(strVal, ints); 
        ostringstream os; 
        tmOut.pack(os);

        istringstream is(os.str());
        // a little awkward
        TestMessage tmIn(*(dynamic_pointer_cast<TestMessage>(Message::unpack(is))));

        LOG_DEBUG("tmOut: " << tmOut); 
        LOG_DEBUG(" tmIn: " << tmIn); 
        BOOST_CHECK_EQUAL( tmOut, tmIn);
    }

    {
        TestMessagePtr tmpOut(new TestMessage(strVal, ints));
        ostringstream os; 
        tmpOut->pack(os);

        string archived(os.str());
        LOG_DEBUG(" *tmpOut: " << *tmpOut); 
        LOG_DEBUG("archived: " << archived);

        istringstream is(archived);
        TestMessagePtr tmpIn(dynamic_pointer_cast<TestMessage>(Message::unpack(is)));
        BOOST_REQUIRE(tmpIn.get() != 0);

        LOG_DEBUG("  *tmpIn: " << *tmpIn); 

        BOOST_CHECK_EQUAL( *tmpOut, *tmpIn);

        istringstream is2(archived);
        MessagePtr mpIn(Message::unpack(is2));
        BOOST_REQUIRE( mpIn.get() != 0 );

        LOG_DEBUG("*tmpOut: " << *tmpOut); 
        LOG_DEBUG("  *mpIn: " << * mpIn); 

        LOG_ERROR("GTL - this next line actually fails. It should work, but does not."); 
        TestMessagePtr dmpIn(dynamic_pointer_cast<TestMessage>(mpIn)); 
        BOOST_REQUIRE( dmpIn.get() != 0 );

        LOG_DEBUG("*dmpIn: " << * dmpIn); 
        BOOST_CHECK_EQUAL( *tmpOut, * dmpIn);
    }

}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("-----------------------------------Testing TestMessage output"); 

    string strVal = "who tests the testTestMessage?";
    vector<int> ints;
    for(int i = 1; i <= 10; i++)
        ints.push_back(i); 

    TestMessage tm(strVal, ints); 

    LOG_INFO("Writing TestMessage to stdout"); 
    cout << tm; 
}

