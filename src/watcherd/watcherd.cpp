#include <algorithm>

#include "watcherd.h"
#include "singletonConfig.h"

using namespace watcher;
using namespace std;
using namespace boost;

INIT_LOGGER(Watcherd, "Watcherd"); 

Watcherd::Watcherd() : config(SingletonConfig::instance())
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
    serverConnection.reset(new Server(address, port, (size_t)threadNum, shared_from_this()));
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

MessageHandler::ConnectionCommand Watcherd::produceReply(const MessagePtr &request, MessagePtr &reply)
{
    TRACE_ENTER();
    MessageHandler::ConnectionCommand retVal = MessageHandler::produceReply(request, reply);
    TRACE_EXIT_RET(retVal);
    return retVal;
}

MessageHandler::ConnectionCommand Watcherd::handleReply(const MessagePtr &request, const MessagePtr &reply)
{
    TRACE_ENTER();
    MessageHandler::ConnectionCommand retVal = MessageHandler::handleReply(request, reply);
    TRACE_EXIT_RET(retVal);
    return retVal;
}

MessageHandler::ConnectionCommand Watcherd::produceRequest(MessagePtr &request)
{
    TRACE_ENTER();
    MessageHandler::ConnectionCommand retVal = MessageHandler::writeMessage;
    TRACE_EXIT_RET(retVal);
    return retVal;
}

void Watcherd::handleMessageArrive(const MessagePtr message)
{
    TRACE_ENTER();
    LOG_INFO("Recv'd message: " << message); 

    // If this is a dataRequestMessage, add the message to the list of requestors.
    if (message->type==DATA_REQUEST_MESSAGE_TYPE) 
    {
        LOG_INFO("Adding dataRequestor to dataRequester list..."); 
        DataRequestMessagePtr drMess=boost::dynamic_pointer_cast<DataRequestMessage>(message);
        messageRequesters.push_back(drMess);
    }
    else
    {
        // If we have any requestors for this type of message, send it to them
        if (!messageRequesters.empty())
        {
            for (MessageRequesters::const_iterator mr = messageRequesters.begin(); mr != messageRequesters.end(); mr++)
            {
                // First check time.
                if (((*mr)->startingAt <= message->timestamp && (*mr)->timeFactor > 0) || 
                        ((*mr)->startingAt >= message->timestamp && (*mr)->timeFactor < 0))   
                {
                    DataRequestMessage::MessageTypeList::const_iterator theEnd=(*mr)->dataTypesRequested.end();
                    DataRequestMessage::MessageTypeList::const_iterator i=(*mr)->dataTypesRequested.begin();
                    for ( ; i != theEnd; i++)
                    {
                        if( (*i) == message->type)
                        {
                            LOG_INFO("Would've sent the incoming message to requestor here."); 
                        }
                    }
                }
            }
        }
    }
    TRACE_EXIT();
}
