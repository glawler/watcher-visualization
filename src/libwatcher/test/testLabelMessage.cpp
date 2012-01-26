/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */
/** 
 * @file testLabelMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include "../labelMessage.h"
#include "../colors.h"

using namespace std;
using namespace boost;
using namespace watcher::event;
using namespace watcher::colors;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LabelMessage lm1;
    lm1.label="Hello world";
    lm1.fontSize=60;
    lm1.fromNodeID=asio::ip::address_v4((0x7f000001));   // 127.0.0.1
    lm1.foreground=black;
    lm1.background=white;
    lm1.expiration=10;
    lm1.addLabel=true;

    BOOST_TEST_MESSAGE("Testing operator=()..."); 
    LabelMessage lm2;
    lm2=lm1;
    BOOST_TEST_MESSAGE("lm1 :" << lm1); 
    BOOST_TEST_MESSAGE("lm2 :" << lm2); 
    BOOST_CHECK_EQUAL(lm1,lm2);

    BOOST_TEST_MESSAGE("Testing copy constructor..."); 
    LabelMessage lm3(lm1);
    BOOST_TEST_MESSAGE("lm1 :" << lm1); 
    BOOST_TEST_MESSAGE("lm3 :" << lm3); 
    BOOST_CHECK_EQUAL(lm1,lm3); 

    LabelMessage withGPS("This label is floating in space!", 0.123, 0.123, 0.123);
    LabelMessage copy(withGPS);
    BOOST_TEST_MESSAGE("gps :" << withGPS); 
    BOOST_TEST_MESSAGE("cpy :" << copy); 
    BOOST_CHECK_EQUAL(withGPS, copy); 
    
}

BOOST_AUTO_TEST_CASE( archive_test )
{
    BOOST_TEST_MESSAGE("------------------------------------------Testing LabelMessage archiving..."); 

    LabelMessagePtr lmOut(new LabelMessage); 

    lmOut->label="Hello world";
    lmOut->fontSize=60;
    lmOut->fromNodeID=asio::ip::address::from_string("127.0.0.1"); 
    lmOut->foreground=black;
    lmOut->background=white;
    lmOut->expiration=10;
    lmOut->addLabel=false;       // Normally you would not mix address, and (lat,lng, alt), but 
    lmOut->lat=1.23456789;       // for testing it's ok. 
    lmOut->lng=1.23456789;
    lmOut->alt=1.23456789;

    BOOST_TEST_MESSAGE("Serializing: " << lmOut); 
    ostringstream os1;
    lmOut->pack(os1); 

    BOOST_TEST_MESSAGE("Serialized lmOut: " << os1.str()); 

    LabelMessage lmIn;
    istringstream is1(os1.str());
    lmIn=*(boost::dynamic_pointer_cast<LabelMessage>(Message::unpack(is1))); 

    BOOST_TEST_MESSAGE( "Checking text archiving..." ); 
    BOOST_TEST_MESSAGE("Comparing these two messages:"); 
    BOOST_TEST_MESSAGE("lmOut :" << lmOut); 
    BOOST_TEST_MESSAGE("lmIn :" << lmIn); 

    BOOST_CHECK_EQUAL( *lmOut, lmIn );
}

BOOST_AUTO_TEST_CASE( shared_ptr_archive_test )
{
    BOOST_TEST_MESSAGE("----------------Testing LabelMessagePtr archiving..."); 
    {
        LabelMessagePtr archivedDataPtr(new LabelMessage("THis is a shared_ptr labelMessage"));
        ostringstream os; 
        archivedDataPtr->pack(os); 

        LabelMessagePtr unArchivedDataPtr;
        istringstream is(os.str());
        unArchivedDataPtr=boost::dynamic_pointer_cast<LabelMessage>(Message::unpack(is)); 

        BOOST_TEST_MESSAGE("Checking equality of archived data via shared_ptrs..."); 
        BOOST_TEST_MESSAGE("*archivedDataPtr: " << *archivedDataPtr); 
        BOOST_TEST_MESSAGE("*unArchivedDataPtr: " << *unArchivedDataPtr); 
        BOOST_CHECK_EQUAL( *archivedDataPtr, *unArchivedDataPtr );
    }

    BOOST_TEST_MESSAGE("------------Testing unarchiving via a base class shared_ptr"); 
    {
        LabelMessagePtr archivedDataPtr(new LabelMessage("THis is a shared_ptr labelMessage"));
        ostringstream os; 
        archivedDataPtr->pack(os); 

        MessagePtr basePtrToUnarchived;
        istringstream is(os.str());
        basePtrToUnarchived=Message::unpack(is); 

        BOOST_TEST_MESSAGE("*basePtrToUnarchived after unarchiving: " << *basePtrToUnarchived); 

        LabelMessagePtr derivedPtrToUnarchived = boost::dynamic_pointer_cast<LabelMessage>(basePtrToUnarchived);
        BOOST_REQUIRE( derivedPtrToUnarchived.get() != NULL);  // <-------- GTL FAIL!

        BOOST_TEST_MESSAGE("Checking equality of archived data via shared_ptrs..."); 
        BOOST_TEST_MESSAGE("*archivedDataPtr: " << *archivedDataPtr); 
        BOOST_TEST_MESSAGE("*basePtrToUnarchived: " << *basePtrToUnarchived); 
        BOOST_TEST_MESSAGE("*derivedPtrToUnarchived: " << *derivedPtrToUnarchived); 
        
        BOOST_CHECK_EQUAL( *archivedDataPtr, *derivedPtrToUnarchived );     // GTL FAIL!
    }
}

BOOST_AUTO_TEST_CASE( output_test )
{
    BOOST_TEST_MESSAGE("-----------------------------------Testing LabelMessage output"); 

    LabelMessagePtr lmp1 = LabelMessagePtr(new LabelMessage);

    lmp1->label="Hello world";
    lmp1->fontSize=60;
    lmp1->fromNodeID=boost::asio::ip::address::from_string("127.0.0.1");  
    lmp1->foreground=black;
    lmp1->background=white;
    lmp1->expiration=10;

    BOOST_TEST_MESSAGE("*lmp1 :" << *lmp1); 

    LabelMessage lm1 = *lmp1;

    BOOST_TEST_MESSAGE("To do: finish writing output unit test"); 
    // Althought testing against the timestamp string will be tricky.
}

//
// This is really more a test of message::unpack(). Can it handle mulitple messages in the same stream?
//
// YAML-CPP does not support this. Until it does (issue #148 @ yaml-cpp dev site), 
// this test has been commented out. 
//
// Note that dataMarshaller still supports multiple Messages in a packet, but the 
// Message::un/pack() API does not. 
//
// BOOST_AUTO_TEST_CASE( multi_pack_test )
// {
//     unsigned int loopNum=5;
//     ostringstream os;
//     LabelMessagePtr m;
//     for(unsigned int i=0; i<loopNum; i++)
//     {
//         m=LabelMessagePtr(new LabelMessage("Expiration will vary, look there")); 
//         m->expiration=i;
//         m->pack(os);
//     }
// 
//     BOOST_TEST_MESSAGE("Multiple messages in a stream: " << os.str()); 
// 
// 	// GTL Each message needs be in its own stream now. 
//     istringstream is(os.str()); 
//     for(unsigned int i=0; i<loopNum; i++)
//     {
//         m=boost::dynamic_pointer_cast<LabelMessage>(Message::unpack(is)); 
//         BOOST_REQUIRE(m.get()!=0); 
//         BOOST_CHECK_EQUAL(m->expiration, i);
//         BOOST_TEST_MESSAGE("Unpacked: " << *m); 
//     }
// }
// 
