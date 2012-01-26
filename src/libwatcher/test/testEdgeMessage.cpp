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
 * @file testEdgeMessage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include "../edgeMessage.h"
#include "../watcherTypes.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace watcher::colors;
using namespace boost::unit_test_framework;

EdgeMessagePtr createEdgeMessage()
{

    LabelMessagePtr lmm(new LabelMessage); 
    lmm->label="I am the middle label"; 
    lmm->fontSize=15;
    lmm->foreground=black;
    lmm->background=white;
    lmm->expiration=5000;

    LabelMessagePtr lm1(new LabelMessage); 
    lm1->label="I am node1 label"; 
    lm1->fontSize=30;
    lm1->foreground=blue;
    lm1->background=red;
    lm1->expiration=10000;

    // Test with a NULL MessageLabelPtr for the node2 label.
    // LabelMessagePtr lm2(new LabelMessage); 
    // lm2->label="I am node2 label"; 
    // lm2->fontSize=10;
    // lm2->foreground=red;
    // lm2->background=blue;
    // lm2->expiration=7500;

    EdgeMessagePtr em(new EdgeMessage); 
    em->node1=NodeIdentifier::from_string("192.168.1.1");
    em->node2=NodeIdentifier::from_string("192.168.1.2");
    em->edgeColor=red;
    em->expiration=20000;
    em->width=15;
    em->layer=UNDEFINED_LAYER;
    em->addEdge=true;
    em->setMiddleLabel(lmm);
    em->setNode1Label(lm1);
    // em->setNode2Label(lm2);

    return em;
}

BOOST_AUTO_TEST_CASE( ctor_test )
{
    EdgeMessagePtr emp=createEdgeMessage(); 

    BOOST_TEST_MESSAGE("Testing operator=()..."); 
    EdgeMessage em2(*emp);
    em2=*emp;
    BOOST_TEST_MESSAGE(" em :" << *emp); 
    BOOST_TEST_MESSAGE("em2 :" << em2); 
    BOOST_CHECK_EQUAL( *emp, em2 ); 

    BOOST_TEST_MESSAGE("Testing copy constructor..."); 
    EdgeMessage em3(*emp);
    BOOST_TEST_MESSAGE(" em :" << *emp);
    BOOST_TEST_MESSAGE("em3 :" << em3); 
    BOOST_CHECK_EQUAL( *emp, em3 );
}

BOOST_AUTO_TEST_CASE( derived_pointer_archive_test )
{
    BOOST_TEST_MESSAGE("Testing EdgeMessage archiving..."); 

    EdgeMessagePtr emp=createEdgeMessage();

    BOOST_TEST_MESSAGE(" em: " << *emp); 
    ostringstream os1;
    emp->pack(os1);

    BOOST_TEST_MESSAGE("Serialized em: " << os1.str()); 

    EdgeMessagePtr emp2;
    istringstream is1(os1.str());
    emp2=dynamic_pointer_cast<EdgeMessage>(Message::unpack(is1)); 

    BOOST_TEST_MESSAGE( "Checking shared_ptr archiving..." ); 
    BOOST_TEST_MESSAGE("Comparing these two messages:"); 
    BOOST_TEST_MESSAGE(" em :" << *emp); 
    BOOST_TEST_MESSAGE("em2 :" << *emp2); 
    BOOST_CHECK_EQUAL( *emp, *emp2 );
}

BOOST_AUTO_TEST_CASE( base_pointer_archive_test )
{
    MessagePtr emp=createEdgeMessage();

    BOOST_TEST_MESSAGE(" em: " << *emp); 
    ostringstream os1;
    emp->pack(os1);

    BOOST_TEST_MESSAGE("Serialized em: " << os1.str()); 

    MessagePtr emp2;
    istringstream is1(os1.str());
    emp2=Message::unpack(is1); 

    BOOST_TEST_MESSAGE( "Checking shared_ptr archiving..." ); 
    BOOST_TEST_MESSAGE("Comparing these two messages:"); 
    BOOST_TEST_MESSAGE(" em :" << *emp); 
    BOOST_TEST_MESSAGE("em2 :" << *emp2); 
    BOOST_CHECK_EQUAL( *emp, *emp2 );
}
