#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>  // Need this to serialize shared_ptrs. 

#include "../labelMessage.h"
#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

    LabelMessage lm1;
    lm1.label="Hello world";
    lm1.fontSize=60;
    lm1.address=asio::ip::address_v4((0x7f000001));   // 127.0.0.1
    lm1.foreground=Color::black;
    lm1.background=Color::white;
    lm1.expiration=10;
    lm1.addLabel=true;

    LOG_INFO("Testing operator=()..."); 
    LabelMessage lm2;
    lm2=lm1;
    LOG_DEBUG("lm1 :" << lm1); 
    LOG_DEBUG("lm2 :" << lm2); 
    BOOST_CHECK_EQUAL(lm1,lm2);

    LOG_INFO("Testing copy constructor..."); 
    LabelMessage lm3(lm1);
    LOG_DEBUG("lm1 :" << lm1); 
    LOG_DEBUG("lm3 :" << lm3); 
    BOOST_CHECK_EQUAL(lm1,lm3); 

    LabelMessage withGPS("This label is floating in space!", 0.123, 0.123, 0.123);
    LabelMessage copy(withGPS);
    LOG_DEBUG("gps :" << withGPS); 
    LOG_DEBUG("cpy :" << copy); 
    BOOST_CHECK_EQUAL(withGPS, copy); 
    
}

BOOST_AUTO_TEST_CASE( archive_test )
{
    LOG_INFO("Testing LabelMessage archiving..."); 

    LabelMessage lmOut;;

    lmOut.label="Hello world";
    lmOut.fontSize=60;
    lmOut.address=asio::ip::address::from_string("127.0.0.1"); 
    lmOut.foreground=Color::black;
    lmOut.background=Color::white;
    lmOut.expiration=10;
    lmOut.addLabel=false;       // Normally you would not mix address, and (lat,lng, alt), but 
    lmOut.lat=1.23456789;       // for testing it's ok. 
    lmOut.lng=1.23456789;
    lmOut.alt=1.23456789;

    LOG_INFO("Serializing: " << lmOut); 
    ostringstream os1;
    archive::polymorphic_text_oarchive oa1(os1);
    oa1 << lmOut;

    LOG_INFO("Serialized lmOut: " << os1.str()); 

    LabelMessage lmIn;
    istringstream is1(os1.str());
    archive::polymorphic_text_iarchive ia1(is1);
    ia1 >> lmIn;

    LOG_INFO( "Checking text archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    LOG_DEBUG("lmOut :" << lmOut); 
    LOG_DEBUG("lmIn :" << lmIn); 

    BOOST_CHECK_EQUAL( lmOut, lmIn );

    ostringstream os2;
    archive::polymorphic_binary_oarchive oa2(os2);
    oa2 << lmOut;

    istringstream is2(os2.str());
    archive::polymorphic_binary_iarchive ia2(is2);
    ia2 >> lmIn;

    LOG_INFO( "Checking binary archiving..." ); 
    BOOST_CHECK_EQUAL( lmOut, lmIn );
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("Testing LabelMessage archiving..."); 

    LabelMessagePtr lmp1 = LabelMessagePtr(new LabelMessage);

    lmp1->label="Hello world";
    lmp1->fontSize=60;
    lmp1->address=boost::asio::ip::address::from_string("127.0.0.1");  
    lmp1->foreground=Color::black;
    lmp1->background=Color::white;
    lmp1->expiration=10;

    LOG_DEBUG("*lmp1 :" << *lmp1); 

    LabelMessage lm1 = *lmp1;

    LOG_FATAL("To do: finish writing output unit test"); 
    // Althought testing against the timestamp string will be tricky.

}

