#include <assert.h>

#include "messageStream.h"
#include "libwatcher/startWatcherMessage.h"
#include "libwatcher/stopWatcherMessage.h"

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
    TRACE_ENTER()
    TRACE_EXIT_RET("true");
    return true;
}
bool MessageStream::setStreamRate(const float &messageStreamRate)
{
    TRACE_ENTER();
    TRACE_EXIT_RET("true");
    return true;
}
bool MessageStream::getNextMessage(MessagePtr newMessage)
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
    messageCache.erase(messageCache.begin());
    readReady=messageCache.size()>0; 
    LOG_DEBUG("Setting readReady to " << (readReady?"true":"false")); 

    TRACE_EXIT_RET(true);
    return true;
}
bool MessageStream::isStreamReadable() const
{
    TRACE_ENTER();
    bool retVal=messageCache.size() > 1; 
    TRACE_EXIT_RET(retVal); 
    return retVal;
}
bool MessageStream::addMessageFilter(const MessageStreamFilter &filter)
{
    TRACE_ENTER();
    
    assert(0);  // filters are not supported yet. 

    TRACE_EXIT_RET("true");
    return true;
}
bool MessageStream::getMessageTimeRange(Timestamp &startTime, Timestamp endTime)
{
    TRACE_ENTER();
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
bool MessageStream::handleMessageArrive(const MessagePtr &message, MessagePtr &response)
{
    TRACE_ENTER();
    bool retVal=ClientMessageHandler::handleMessageArrive(message, response);

    if(retVal==false)  // 
    {
        // put new message on message queue if it's not a status message. 
        switch (message->type)
        {
            case MESSAGE_STATUS_TYPE: 
                // If we've gotten here, the message status is not ack or ok.
                // Close the connection. 
                connection.reset();  // should delete the connection. 

            case TEST_MESSAGE_TYPE:
            case GPS_MESSAGE_TYPE:
            case LABEL_MESSAGE_TYPE:
            case EDGE_MESSAGE_TYPE:
            case COLOR_MESSAGE_TYPE:
            case DATA_REQUEST_MESSAGE_TYPE:
            case NODE_STATUS_MESSAGE_TYPE:
            case USER_DEFINED_MESSAGE_TYPE:
                {
                    lock_guard<mutex> lock(messageCacheMutex); 
                    messageCache.push_back(message); 
                    readReady=true;
                    LOG_DEBUG("Added message to queue, notifing all waiting threads that there is data to be read, readReady=true"); 
                    messageCacheCond.notify_all();
                }
                break;

            case SEEK_MESSAGE_TYPE:
            case START_MESSAGE_TYPE:
            case STOP_MESSAGE_TYPE:
            case SPEED_MESSAGE_TYPE:
                LOG_ERROR("Recv'd watcherAPI message at client. This shouldn't happen. Ignoring message.");
                break;

            case UNKNOWN_MESSAGE_TYPE:
                LOG_ERROR("Recv'd unknown message type (encoded as such!). Should not happen"); 
                break;

                // GTL - do not put default, we want the compiler to complain if we miss a message type.
        }
        retVal=true;
    }

    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

//virtual 
bool MessageStream::handleMessagesArrive(const vector<MessagePtr> &messages, MessagePtr &response)
{
    TRACE_ENTER();

    bool retVal=true;
    for(vector<MessagePtr>::const_iterator m=messages.begin(); m!=messages.end(); ++m)
        if(!handleMessageArrive(*m, response))
            retVal=false;

    TRACE_EXIT_RET(retVal);
    return retVal;
}

std::ostream &operator<<(std::ostream &out, const MessageStream &messStream)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return out;
}
