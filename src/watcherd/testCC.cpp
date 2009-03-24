#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "clientConnection.h"
#include <libwatcher/testMessage.h>

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;

int main(int argc, char **argv)
{
    TRACE_ENTER();

    try
    {
        log4cxx::PropertyConfigurator::configure("log.properties");                                                                           

        int loopCount=argc > 1 ? boost::lexical_cast<int>(argv[1]) : 2;

        asio::io_service ioserv;

        ClientConnection c(ioserv, "glory", "watcherd"); 

        vector<int> ints;
        string strVal = "from testCC"; 
        for (int i = 0; i < loopCount; i++)
        {
            ints.push_back(i);
            ints.push_back(i*2);
            c.sendMessage(shared_ptr<TestMessage>(new TestMessage(strVal, ints))); 
        }

        ioserv.run(); 

        // sleep(60); 

        c.close();
        // t.join();
    }
    catch (std::exception &e)
    {
        cerr << "Caught exception: " << e.what() << endl;
    }

    return 0;
}
