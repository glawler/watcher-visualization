/** 
 * @file testLabelMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include <libwatcher/labelMessage.h>
#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher::event;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

    LabelMessage lm1;
    lm1.label="Hello world";
    lm1.fontSize=60;
    lm1.fromNodeID=asio::ip::address_v4((0x7f000001));   // 127.0.0.1
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
    LOG_INFO("------------------------------------------Testing LabelMessage archiving..."); 

    LabelMessage lmOut;;

    lmOut.label="Hello world";
    lmOut.fontSize=60;
    lmOut.fromNodeID=asio::ip::address::from_string("127.0.0.1"); 
    lmOut.foreground=Color::black;
    lmOut.background=Color::white;
    lmOut.expiration=10;
    lmOut.addLabel=false;       // Normally you would not mix address, and (lat,lng, alt), but 
    lmOut.lat=1.23456789;       // for testing it's ok. 
    lmOut.lng=1.23456789;
    lmOut.alt=1.23456789;

    LOG_INFO("Serializing: " << lmOut); 
    ostringstream os1;
    lmOut.pack(os1); 

    LOG_INFO("Serialized lmOut: " << os1.str()); 

    LabelMessage lmIn;
    istringstream is1(os1.str());
    lmIn=*(boost::dynamic_pointer_cast<LabelMessage>(Message::unpack(is1))); 

    LOG_INFO( "Checking text archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    LOG_DEBUG("lmOut :" << lmOut); 
    LOG_DEBUG("lmIn :" << lmIn); 

    BOOST_CHECK_EQUAL( lmOut, lmIn );
}

BOOST_AUTO_TEST_CASE( shared_ptr_archive_test )
{
    LOG_INFO("----------------Testing LabelMessagePtr archiving..."); 
    {
        LabelMessagePtr archivedDataPtr(new LabelMessage("THis is a shared_ptr labelMessage"));
        ostringstream os; 
        archivedDataPtr->pack(os); 

        LabelMessagePtr unArchivedDataPtr;
        istringstream is(os.str());
        unArchivedDataPtr=boost::dynamic_pointer_cast<LabelMessage>(Message::unpack(is)); 

        LOG_INFO("Checking equality of archived data via shared_ptrs..."); 
        LOG_DEBUG("*archivedDataPtr: " << *archivedDataPtr); 
        LOG_DEBUG("*unArchivedDataPtr: " << *unArchivedDataPtr); 
        BOOST_CHECK_EQUAL( *archivedDataPtr, *unArchivedDataPtr );
    }

    LOG_INFO("------------Testing unarchiving via a base class shared_ptr"); 
    {
        LabelMessagePtr archivedDataPtr(new LabelMessage("THis is a shared_ptr labelMessage"));
        ostringstream os; 
        archivedDataPtr->pack(os); 

        MessagePtr basePtrToUnarchived;
        istringstream is(os.str());
        basePtrToUnarchived=Message::unpack(is); 

        LOG_DEBUG("*basePtrToUnarchived after unarchiving: " << *basePtrToUnarchived); 

        LabelMessagePtr derivedPtrToUnarchived = boost::dynamic_pointer_cast<LabelMessage>(basePtrToUnarchived);
        BOOST_REQUIRE( derivedPtrToUnarchived.get() != 0 );  // <-------- GTL FAIL!

        LOG_INFO("Checking equality of archived data via shared_ptrs..."); 
        LOG_DEBUG("*archivedDataPtr: " << *archivedDataPtr); 
        LOG_DEBUG("*basePtrToUnarchived: " << *basePtrToUnarchived); 
        LOG_DEBUG("*derivedPtrToUnarchived: " << *derivedPtrToUnarchived); 
        
        BOOST_CHECK_EQUAL( *archivedDataPtr, *derivedPtrToUnarchived );     // GTL FAIL!
    }
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("-----------------------------------Testing LabelMessage output"); 

    LabelMessagePtr lmp1 = LabelMessagePtr(new LabelMessage);

    lmp1->label="Hello world";
    lmp1->fontSize=60;
    lmp1->fromNodeID=boost::asio::ip::address::from_string("127.0.0.1");  
    lmp1->foreground=Color::black;
    lmp1->background=Color::white;
    lmp1->expiration=10;

    LOG_DEBUG("*lmp1 :" << *lmp1); 

    LabelMessage lm1 = *lmp1;

    LOG_FATAL("To do: finish writing output unit test"); 
    // Althought testing against the timestamp string will be tricky.
}

//
// This is really more a test of message::unpack(). Can it handle mulitple messages in the same stream?
//
BOOST_AUTO_TEST_CASE( multi_pack_test )
{
    unsigned int loopNum=5;
    ostringstream os;
    LabelMessagePtr m;
    for(unsigned int i=0; i<loopNum; i++)
    {
        m=LabelMessagePtr(new LabelMessage("Expiration will vary, look there")); 
        m->expiration=i;
        m->pack(os);
    }

    LOG_DEBUG("Multiple messages in a stream: " << os.str()); 

    istringstream is(os.str()); 
    for(unsigned int i=0; i<loopNum; i++)
    {
        m=boost::dynamic_pointer_cast<LabelMessage>(Message::unpack(is)); 
        BOOST_REQUIRE(m.get()!=0); 
        BOOST_CHECK_EQUAL(m->expiration, i);
        LOG_DEBUG("Unpacked: " << *m); 
    }
}

