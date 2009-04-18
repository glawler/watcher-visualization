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
    serverConnection.reset(new Server(*this, address, port, (size_t)threadNum, serverMessageHandlerPtr));
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

void Watcherd::subscribe(ServerConnectionPtr client)
{
    TRACE_ENTER();

    pthread_mutex_lock(&messageRequestorsLock);
    shared_ptr<pthread_mutex_t> lock(&messageRequestorsLock, pthread_mutex_unlock);

    messageRequestors.push_back(client);

    TRACE_EXIT();
}

void Watcherd::unsubscribe(ServerConnectionPtr client)
{
    TRACE_ENTER();
    pthread_mutex_lock(&messageRequestorsLock);
    shared_ptr<pthread_mutex_t> lock(&messageRequestorsLock, pthread_mutex_unlock);

    messageRequestors.remove(client);
    TRACE_EXIT();
}

/** Send a single message to all clients subscribed to the live stream */
void Watcherd::sendMessage(MessagePtr msg)
{
    TRACE_ENTER();
    pthread_mutex_lock(&messageRequestorsLock);
    shared_ptr<pthread_mutex_t> lock(&messageRequestorsLock, pthread_mutex_unlock);

    // bind can't handle overloaded functions.  use member function pointer to help
    void (ServerConnection::*ptr)(MessagePtr) = &ServerConnection::sendMessage;
    for_each(messageRequestors.begin(), messageRequestors.end(), bind(ptr, _1, msg));
    TRACE_EXIT();
}

/** Send a set of messages to all clients subscribed to the live stream */
void Watcherd::sendMessage(const std::vector<MessagePtr>& msg)
{
    TRACE_ENTER();
    pthread_mutex_lock(&messageRequestorsLock);
    shared_ptr<pthread_mutex_t> lock(&messageRequestorsLock, pthread_mutex_unlock);

    // bind can't handle overloaded functions.  use member function pointer to help
    void (ServerConnection::*ptr)(const std::vector<MessagePtr>&) = &ServerConnection::sendMessage;
    for_each(messageRequestors.begin(), messageRequestors.end(), bind(ptr, _1, msg));
    TRACE_EXIT();
}
