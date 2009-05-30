#include <assert.h>

#include "messageStream.h"
#include "libwatcher/startWatcherMessage.h"
#include "libwatcher/stopWatcherMessage.h"
#include "libwatcher/seekWatcherMessage.h"
#include "libwatcher/speedWatcherMessage.h"

using namespace watcher;
using namespace watcher::event;
using namespace std;
using namespace boost;

INIT_LOGGER(MessageStream, "MessageStream");

MessageStream::MessageStream(
        const string &serverName_, 
        const string &serviceName_, 
        const Timestamp &startTime_, 
        const float streamRate_) : 
    messagesSent(0),
    messagesArrived(0),
    messageStreamFilters(),
    streamRate(streamRate_),
    streamStartTime(startTime_),
    serverName(serverName_),
    serviceName(serviceName_),
    connection(),
    messageCache(),
    messageCacheMutex(),
    readReady(false)
{
    TRACE_ENTER();
    TRACE_EXIT();
}

// static 
MessageStreamPtr MessageStream::createNewMessageStream(const string &serverName_, const Timestamp &startTime_, const float streamRate_)
{
    TRACE_ENTER();
    MessageStreamPtr retVal(new MessageStream(serverName_,"watcherd",startTime_,streamRate_));
    retVal->initConnection(); 
    TRACE_EXIT();
    return retVal;
}

// static 
MessageStreamPtr MessageStream::createNewMessageStream(
                    const std::string &serverName, 
                    const std::string &portNum,  // Connect on a non-standard port (different port than watcherd service)
                    const Timestamp &startTime, 
                    const float streamRate)
{
    TRACE_ENTER();
    MessageStreamPtr retVal(new MessageStream(serverName, portNum, startTime, streamRate));
    retVal->initConnection(); 
    TRACE_EXIT();
    return retVal;
}

void MessageStream::initConnection() 
{
    TRACE_ENTER();
    connection=ClientPtr(new Client(serverName, serviceName)); 
    connection->addMessageHandler(shared_from_this()); 
    setStreamTimeStart(streamStartTime);
    setStreamRate(streamRate);
    TRACE_EXIT();
}

// virtual
MessageStream::~MessageStream()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

bool MessageStream::setStreamTimeStart(const Timestamp &startTime)
{
    TRACE_ENTER();
    LOG_DEBUG("Setting stream start time to " << startTime); 
    SeekMessagePtr mess(new SeekMessage(startTime)); 
    bool retVal=connection->sendMessage(mess); 
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool MessageStream::setStreamRate(const float &messageStreamRate)
{
    TRACE_ENTER();
    LOG_DEBUG("Setting message stream rate to " << messageStreamRate); 
    SpeedMessagePtr mess(new SpeedMessage(messageStreamRate)); 
    bool retVal=connection->sendMessage(mess); 
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool MessageStream::getNextMessage(MessagePtr &newMessage)
{
    TRACE_ENTER();
    if(!connection) 
    {
        TRACE_EXIT_RET(false);
        return false;
    }

    unique_lock<mutex> lock(messageCacheMutex); 
    while(false==readReady)
    {
        messageCacheCond.wait(lock); 
    }

    assert(messageCache.size() > 0); 
    newMessage=messageCache.front();
    messageCache.pop_front();
    readReady=messageCache.size()>0; 
    LOG_DEBUG("Setting readReady to " << (readReady?"true":"false")); 
    LOG_DEBUG("MessageCache size: " << messageCache.size()); 

    TRACE_EXIT_RET(true);
    return true;
}

bool MessageStream::isStreamReadable() const
{
    TRACE_ENTER();
    bool retVal=messageCache.size() > 0; 
    TRACE_EXIT_RET(retVal); 
    return retVal;
}

bool MessageStream::addMessageFilter(const MessageStreamFilter & /*filter*/)
{
    TRACE_ENTER();
    
    assert(0);  // filters are not supported yet. 

    TRACE_EXIT_RET("true");
    return true;
}

bool MessageStream::getMessageTimeRange(Timestamp &startTime, Timestamp endTime)
{
    TRACE_ENTER();

    startTime=0;
    endTime=0;

    // query watcherd for the least and greatest time here.
    assert(0); // not yet supported - goodbye.

    LOG_DEBUG("Got message time range from server. Start:" << startTime << " End:" << endTime); 

    TRACE_EXIT_RET("true");
    return true;
}

//virtual 
std::ostream &MessageStream::toStream(std::ostream &out) const
{
    TRACE_ENTER();
    out << "MessageStream output operator not yet implemented - this space intensionally left blank"; 
    TRACE_EXIT();
    return out; 
}

bool MessageStream::startStream()
{
    TRACE_ENTER();
    StartMessagePtr mess(new StartMessage); 
    bool retVal=connection->sendMessage(mess); 
    TRACE_EXIT_RET((retVal==true?"true":"false")); 
    return retVal;
}

bool MessageStream::stopStream()
{
    TRACE_ENTER();
    StopMessagePtr mess(new StopMessage); 
    bool retVal=connection->sendMessage(mess); 
    TRACE_EXIT_RET((retVal==true?"true":"false")); 
    return retVal;
}

//virtual 
bool MessageStream::handleMessageArrive(ConnectionPtr conn, const MessagePtr &message)
{
    TRACE_ENTER();

    messagesArrived++; 

    // We don't really add anything yet to a generic watcherdAPI client.
    bool retVal=WatcherdAPIMessageHandler::handleMessageArrive(conn, message); 

    {
        lock_guard<mutex> lock(messageCacheMutex);
        messageCache.push_back(message); 
        readReady=true;
        // let lock go out of scope
    }
    LOG_DEBUG("MessageCache size=" << messageCache.size());
    LOG_DEBUG("Notifing all waiting threads that there is data to be read"); 
    messageCacheCond.notify_all();


    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

//virtual 
bool MessageStream::handleMessagesArrive(ConnectionPtr conn, const vector<MessagePtr> &messages)
{
    TRACE_ENTER();

    bool retVal=false;
    for(vector<MessagePtr>::const_iterator m=messages.begin(); m!=messages.end(); ++m)
        retVal |= handleMessageArrive(conn, *m);

    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

//virtual 
bool MessageStream::handleMessageSent(const MessagePtr &message)
{
    TRACE_ENTER();

    messagesSent++; 

    // We don't really add anything yet to a generic watcherdAPI client.
    bool retVal=WatcherdAPIMessageHandler::handleMessageSent(message); 

    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

//virtual 
bool MessageStream::handleMessagesSent(const vector<MessagePtr> &messages)
{
    TRACE_ENTER();

    bool retVal=false;
    for(vector<MessagePtr>::const_iterator m=messages.begin(); m!=messages.end(); ++m)
        retVal |= handleMessageSent(*m);

    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

std::ostream &watcher::operator<<(std::ostream &out, const MessageStream & /*messStream*/)
{
    TRACE_ENTER();
    out << "MessageStream()";
    TRACE_EXIT();
    return out;
}
