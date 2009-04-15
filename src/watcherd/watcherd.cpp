#include <algorithm>

#include "watcherd.h"
#include "singletonConfig.h"

using namespace watcher;
using namespace watcher::event;
using namespace std;
using namespace boost;

INIT_LOGGER(Watcherd, "Watcherd"); 

Watcherd::Watcherd() : 
    config(SingletonConfig::instance()),
    serverMessageHandlerPtr(new ServerMessageHandler)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

Watcherd::~Watcherd()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void Watcherd::run(const std::string &address, const std::string &port, const int &threadNum)
{
    TRACE_ENTER(); 

    // Block all signals for background thread.
    sigset_t new_mask;
    sigfillset(&new_mask);
    sigset_t old_mask;
    pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

    // Run server in background thread.
    serverConnection.reset(new Server(address, port, (size_t)threadNum, serverMessageHandlerPtr));
    connectionThread = boost::thread(boost::bind(&watcher::Server::run, serverConnection));

    // Restore previous signals.
    pthread_sigmask(SIG_SETMASK, &old_mask, 0);

    // Wait for signal indicating time to shut down.
    sigset_t wait_mask;
    sigemptyset(&wait_mask);
    sigaddset(&wait_mask, SIGINT);
    sigaddset(&wait_mask, SIGQUIT);
    sigaddset(&wait_mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
    int sig = 0;
    sigwait(&wait_mask, &sig);

    // Stop the server.
    serverConnection->stop();
    connectionThread.join();
    TRACE_EXIT();
}

