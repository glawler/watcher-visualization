#include <boost/asio.hpp>

#include "../clientConnection.hpp"
#include "../response_handler.hpp"

using namespace std;
using namespace watcher;
using namespace boost;

int main(int argc, char **argv)
{
    TRACE_ENTER();

    log4cxx::PropertyConfigurator::configure("log.properties");                                                                           
    boost::asio::io_service ioserv;
    //boost::shared_ptr<response_handler> handler = new response_handler;
    ClientConnection cc(ioserv);
    ioserv.run();

    return 0;
}
