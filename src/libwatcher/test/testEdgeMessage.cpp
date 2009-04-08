#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include <libwatcher/edgeMessage.h>
#include "logger.h"

using namespace std;
using namespace boost;
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

BOOST_AUTO_TEST_CASE( derived_pointer_archive_test )
{
    LOG_INFO("Testing EdgeMessage archiving..."); 

    EdgeMessagePtr emp=createEdgeMessage();

    LOG_INFO(" em: " << *emp); 
    ostringstream os1;
    emp->pack(os1);

    LOG_INFO("Serialized em: " << os1.str()); 

    EdgeMessagePtr emp2;
    istringstream is1(os1.str());
    emp2=dynamic_pointer_cast<EdgeMessage>(Message::unpack(is1)); 

    LOG_INFO( "Checking shared_ptr archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    LOG_DEBUG(" em :" << *emp); 
    LOG_DEBUG("em2 :" << *emp2); 
    BOOST_CHECK_EQUAL( *emp, *emp2 );
}

BOOST_AUTO_TEST_CASE( base_pointer_archive_test )
{
    MessagePtr emp=createEdgeMessage();

    LOG_INFO(" em: " << *emp); 
    ostringstream os1;
    emp->pack(os1);

    LOG_INFO("Serialized em: " << os1.str()); 

    MessagePtr emp2;
    istringstream is1(os1.str());
    emp2=Message::unpack(is1); 

    LOG_INFO( "Checking shared_ptr archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    LOG_DEBUG(" em :" << *emp); 
    LOG_DEBUG("em2 :" << *emp2); 
    BOOST_CHECK_EQUAL( *emp, *emp2 );
}
