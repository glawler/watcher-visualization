#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::Color test
#include <boost/test/unit_test.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include "../watcherColors.h"
#include "logger.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace watcher::event;
using namespace boost::unit_test_framework;


BOOST_AUTO_TEST_CASE( ctors_test )
{
    LOAD_LOG_PROPS("log.properties"); 

    LOG_INFO("Checking Color constructors..."); 
    Color c1, c2; 
    BOOST_CHECK_EQUAL( c1, c2 );

    c1=Color(0xface1234);
    c2.fromString("0xface1234");
    BOOST_CHECK_EQUAL( c1, c2 );

    c1=Color::blue;
    c2=Color(0x0000ff00); 
    BOOST_CHECK_EQUAL( c1, c2 );

    c1=Color::gold;
    c2=c1;
    BOOST_CHECK_EQUAL( c1, c2 );

    c1=Color(0x00, 0x80, 0x00, 0x00);
    c2=Color::green;
    BOOST_CHECK_EQUAL( c1, c2 );

    bool val = c2.fromString("this is not properly formatted"); 
    BOOST_CHECK_EQUAL( val, false );

    val = c2.fromString("green");
    BOOST_CHECK_EQUAL( val, true ); 
    BOOST_CHECK_EQUAL( c2, Color::green );
}

BOOST_AUTO_TEST_CASE( archive_test )
{
    Color green(Color::green);
    ostringstream os1;
    archive::polymorphic_text_oarchive oa1(os1);
    oa1 << green;

    LOG_DEBUG("Serialized Color::green: (" << os1.str() << ")"); 

    Color c;
    istringstream is1(os1.str());
    archive::polymorphic_text_iarchive ia1(is1);
    ia1 >> c;

    LOG_INFO( "Checking text archiving..." ); 
    BOOST_CHECK_EQUAL( c, green );
    BOOST_CHECK_EQUAL( c, Color::green );
    BOOST_CHECK_EQUAL( green, Color::green );

    ostringstream os2;
    archive::polymorphic_binary_oarchive oa2(os2);
    oa2 << green;

    istringstream is2(os2.str());
    archive::polymorphic_binary_iarchive ia2(is2);
    ia2 >> c;

    LOG_INFO( "Checking binary archiving..." ); 
    BOOST_CHECK_EQUAL( c, green );
    BOOST_CHECK_EQUAL( c, Color::green );
    BOOST_CHECK_EQUAL( green, Color::green );
}

BOOST_AUTO_TEST_CASE( output_test )
{
    LOG_INFO("Checking Color output operators..."); 

    Color c(0xface1234);
    string o1("0xface1234");
    stringstream o2;
    o2 << c;
    BOOST_CHECK_EQUAL( o1, o2.str() );

    o1 = "blue";
    c=Color::blue;
    stringstream o3;
    o3 << c;
    BOOST_CHECK_EQUAL( o1, o3.str() );
}


