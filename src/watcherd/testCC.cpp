#include <boost/asio.hpp>

#include "clientConnection.hpp"
#include "testMessage.h"
// #include "gpsMessage.h"

using namespace std;
using namespace watcher;
using namespace boost;

int main(int argc, char **argv)
{
    TRACE_ENTER();

    log4cxx::PropertyConfigurator::configure("log.properties");                                                                           
    boost::asio::io_service ioserv;
    ClientConnection cc(ioserv, "glory", "watcherd"); 
    cc.sendMessage(boost::shared_ptr<TestMessage>(new TestMessage));
    cc.sendMessage(boost::shared_ptr<TestMessage>(new TestMessage));
    cc.sendMessage(boost::shared_ptr<TestMessage>(new TestMessage));
    // cc.sendMessage(gpsMessagePtr(new GPSMessage(1.234, 5.678, 90.0)));
    ioserv.run();

    return 0;
}
