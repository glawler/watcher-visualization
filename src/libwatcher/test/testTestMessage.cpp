#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::Message.TestMessage test
#include <boost/test/unit_test.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>   // Need this to serialize shared_ptrs. 
#include <boost/serialization/vector.hpp>        // Need this to serialize std::vectors. 

#include "libwatcher/testMessage.h"
#include "logger.h"

using namespace std;
using namespace boost;
using namespace boost::archive;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

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
        polymorphic_binary_oarchive oa(os);
        oa << tmOut; 

        TestMessage tmIn;
        istringstream is(os.str());
        polymorphic_binary_iarchive ia(is);
        ia >> tmIn;

        LOG_DEBUG("tmOut: " << tmOut); 
        LOG_DEBUG(" tmIn: " << tmIn); 
        BOOST_CHECK_EQUAL( tmOut, tmIn);
    }
    {
        TestMessagePtr tmpOut(new TestMessage(strVal, ints));
        ostringstream os; 
        text_oarchive oa(os);
        oa << tmpOut; 

        TestMessagePtr tmpIn;
        istringstream is(os.str());
        text_iarchive ia(is);
        ia >> tmpIn;

        LOG_DEBUG("*tmpOut: " << *tmpOut); 
        LOG_DEBUG(" *tmpIn: " << *tmpIn); 
        LOG_DEBUG("     os: " << os.str());
        BOOST_CHECK_EQUAL( *tmpOut, *tmpIn);

        MessagePtr mpIn;
        is.str(os.str());
        text_iarchive ia2(is);
        ia2 >> mpIn;

        LOG_DEBUG("*tmpOut: " << *tmpOut); 
        LOG_DEBUG("  *mpIn: " << * mpIn); 

        LOG_ERROR("GTL - this next line actually fails. It should work, but does not."); 
        TestMessagePtr dmpIn=boost::dynamic_pointer_cast<TestMessage>(mpIn); 

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

