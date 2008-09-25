#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::Message.LabelMessage test
#include <boost/test/unit_test.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>  // Need this to serialize shared_ptrs. 

#include "../edgeMessage.h"
#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_CASE( ctor_test )
{
    LOAD_LOG_PROPS("log.properties"); 

    EdgeMessage em;
    em.label="Hello World";
    em.fontSize=15;
    em.node1=asio::ip::address::from_string("192.168.1.1");
    em.node2=asio::ip::address::from_string("192.168.1.2");
    em.edgeColor=Color::red;
    em.labelColorForeground=Color::blue;
    em.labelColorBackground=Color::orange;
    em.expiration=10000;
    em.width=15;
    em.layer=NODE_LAYER;

    LOG_INFO("Testing operator=()..."); 
    EdgeMessage em2;
    em2=em;
    LOG_DEBUG(" em :" << em); 
    LOG_DEBUG("em2 :" << em2); 
    BOOST_CHECK_EQUAL(em,em2);

    LOG_INFO("Testing copy constructor..."); 
    EdgeMessage em3(em);
    LOG_DEBUG(" em :" << em); 
    LOG_DEBUG("em3 :" << em3); 
    BOOST_CHECK_EQUAL(em,em3); 
}

BOOST_AUTO_TEST_CASE( archive_test )
{
    LOG_INFO("Testing EdgeMessage archiving..."); 

    EdgeMessage em;
    em.label="Hello World";
    em.fontSize=15;
    em.node1=asio::ip::address::from_string("192.168.1.1");
    em.node2=asio::ip::address::from_string("192.168.1.2");
    em.edgeColor=Color::red;
    em.labelColorForeground=Color::blue;
    em.labelColorBackground=Color::orange;
    em.expiration=10000;
    em.width=15;
    em.layer=NODE_LAYER;

    LOG_INFO("Serializing: " << em); 
    ostringstream os1;
    archive::polymorphic_text_oarchive oa1(os1);
    oa1 << em;

    LOG_INFO("Serialized em: " << os1.str()); 

    EdgeMessage em2;
    istringstream is1(os1.str());
    archive::polymorphic_text_iarchive ia1(is1);
    ia1 >> em2;

    LOG_INFO( "Checking text archiving..." ); 
    LOG_DEBUG("Comparing these two messages:"); 
    LOG_DEBUG(" em :" << em); 
    LOG_DEBUG("em2 :" << em2); 

    BOOST_CHECK_EQUAL( em, em2 );

    ostringstream os2;
    archive::polymorphic_binary_oarchive oa2(os2);
    oa2 << em;

    istringstream is2(os2.str());
    archive::polymorphic_binary_iarchive ia2(is2);
    ia2 >> em2;

    LOG_INFO( "Checking binary archiving..." ); 
    BOOST_CHECK_EQUAL( em, em2 );
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

