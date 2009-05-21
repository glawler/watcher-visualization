#include <stdlib.h>

#define BOOST_TEST_MODULE watcher::messageStream test
#include <boost/test/unit_test.hpp>

#include "logger.h"
#include "messageStream.h"

using namespace std;
using namespace boost;
using namespace watcher;
using namespace boost::unit_test_framework;


BOOST_AUTO_TEST_CASE( ctors_test )
{
    LOAD_LOG_PROPS("test.log.properties"); 
    MessageStreamPtr ms=MessageStream::createNewMessageStream("glory"); 
}



