#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>  // Need this to serialize shared_ptrs. 

#include <libwatcher/edgeMessage.h>
#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;

EdgeMessagePtr createEdgeMessage()
{

    LabelMessagePtr lmm(new LabelMessage); 
    lmm->label="I am the middle label"; 
    lmm->fontSize=15;
    lmm->foreground=Color::black;
    lmm->background=Color::white;
    lmm->expiration=5000;

    LabelMessagePtr lm1(new LabelMessage); 
    lm1->label="I am node1 label"; 
    lm1->fontSize=30;
    lm1->foreground=Color::blue;
    lm1->background=Color::red;
    lm1->expiration=10000;

    // Test with a NULL MessageLabelPtr for the node2 label.
    // LabelMessagePtr lm2(new LabelMessage); 
    // lm2->label="I am node2 label"; 
    // lm2->fontSize=10;
    // lm2->foreground=Color::red;
    // lm2->background=Color::blue;
    // lm2->expiration=7500;

    EdgeMessagePtr em(new EdgeMessage); 
    em->node1=asio::ip::address::from_string("192.168.1.1");
    em->node2=asio::ip::address::from_string("192.168.1.2");
    em->edgeColor=Color::red;
    em->expiration=20000;
    em->width=15;
    em->layer=NODE_LAYER;
    em->addEdge=true;
    em->setMiddleLabel(lmm);
    em->setNode1Label(lm1);
    // em->setNode2Label(lm2);

    return em;
}

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

    EdgeMessagePtr emp=createEdgeMessage(); 

    LOG_INFO("Testing operator=()..."); 
    EdgeMessage em2(*emp);
    em2=*emp;
    LOG_DEBUG(" em :" << *emp); 
    LOG_DEBUG("em2 :" << em2); 
    BOOST_CHECK_EQUAL( *emp, em2 ); 

    LOG_INFO("Testing copy constructor..."); 
    EdgeMessage em3(*emp);
    LOG_DEBUG(" em :" << *emp);
    LOG_DEBUG("em3 :" << em3); 
    BOOST_CHECK_EQUAL( *emp, em3 );
}

BOOST_AUTO_TEST_CASE( archive_test )
{
    LOG_INFO("Testing EdgeMessage archiving..."); 

    EdgeMessagePtr emp=createEdgeMessage();

    LOG_INFO(" em: " << *emp); 
    ostringstream os1;
    archive::polymorphic_text_oarchive oa1(os1);
    oa1 << emp;

    LOG_INFO("Serialized em: " << os1.str()); 

    EdgeMessagePtr emp2;
    istringstream is1(os1.str());
    archive::polymorphic_text_iarchive ia1(is1);
    ia1 >> emp2;

    LOG_INFO( "Checking shared_ptr text archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    LOG_DEBUG(" em :" << *emp); 
    LOG_DEBUG("em2 :" << *emp2); 
    BOOST_CHECK_EQUAL( *emp, *emp2 );

    ostringstream os2;
    archive::polymorphic_binary_oarchive oa2(os2);
    oa2 << emp;

    istringstream is2(os2.str());
    archive::polymorphic_binary_iarchive ia2(is2);
    ia2 >> emp2;

    LOG_INFO( "Checking shared_ptr binary archiving..." ); 
    BOOST_CHECK_EQUAL( *emp, *emp2 );

    LOG_INFO( "Checking text archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    EdgeMessage em4(*emp);
    EdgeMessage em5(em4);
    LOG_DEBUG(" em4 :" << em4);
    LOG_DEBUG(" em5 :" << em5);

    ostringstream os3;
    archive::polymorphic_text_oarchive oa3(os3);
    oa3 << em4;

    istringstream is3(os3.str());
    archive::polymorphic_text_iarchive ia3(is3);
    ia3 >> em5;

    BOOST_CHECK_EQUAL( em4, em5 );

    ostringstream os4;
    archive::polymorphic_binary_oarchive oa4(os4);
    oa4 << em4;

    istringstream is5(os4.str());
    archive::polymorphic_binary_iarchive ia5(is5);
    ia5 >> em5;

    LOG_INFO( "Checking binary archiving..." ); 
    BOOST_CHECK_EQUAL( em4, em5 );
}

// BOOST_AUTO_TEST_CASE( output_test )
// {
//     LOG_INFO("Testing EdgeMessage output operator..."); 
// 
//     EdgeMessage em;
//     em.label="Hello World";
//     em.fontSize=15;
//     em.node1=asio::ip::address::from_string("192.168.1.1");
//     em.node2=asio::ip::address::from_string("192.168.1.2");
//     em.edgeColor=Color::red;
//     em.labelColorForeground=Color::blue;
//     em.labelColorBackground=Color::orange;
//     em.expiration=10000;
//     em.width=15;
//     em.layer=NODE_LAYER;
// 
//     LOG_FATAL("To do: finish writing output unit test"); 
//     // Althought testing against the timestamp string will be tricky.
// 
// }

