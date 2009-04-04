#include <boost/pointer_cast.hpp>
#include "messageStream.h"

using namespace watcher;
using namespace watcher::event;
using namespace std;

INIT_LOGGER(MessageStream, "MessageStream");

MessageStream::MessageStream(const string &serverName_, const Timestamp &startTime_, const float streamRate_) : 
    messageStreamFilters(),
    streamRate(streamRate_),
    streamStartTime(startTime_),
    connection(new Client(serverName_))
{
    TRACE_ENTER();
    ClientMessageHandlerPtr tmp(shared_from_this());
    LOG_DEBUG("Setting message handler to this class. this pointer: " << tmp); 
    // connection->setMessageHandler(tmp); 
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
    TRACE_EXIT_RET("true");
    return true;
}
bool MessageStream::isStreamReadable() const
{
    TRACE_ENTER();
    TRACE_EXIT_RET("true");
    return true;
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
    // startWatcherMessagePtr mess(new startWatcherMessage); 
    // bool retVal=connection.sendMessage(mess); 
    // TRACE_EXIT_RET((retVal==true?"true":"false")); 
    // return retVal;
    TRACE_EXIT_RET("true"); 
    return true;
}
bool MessageStream::stopStream()
{
    TRACE_ENTER();
    // stopWatcherMessagePtr mess(new stopWatcherMessage); 
    // bool retVal=connection.sendMessage(mess); 
    // TRACE_EXIT_RET((retVal==true?"true":"false")); 
    // return retVal;
    TRACE_EXIT_RET("true"); 
    return true;
}

//virtual 
bool MessageStream::handleMessageArrive(const MessagePtr message, MessagePtr &response)
{
    TRACE_ENTER();

    bool retVal=ClientMessageHandler::handleMessageArrive(message, response);

    TRACE_EXIT_RET((retVal==true?"true":"false"));
    return retVal;
}

std::ostream &operator<<(std::ostream &out, const MessageStream &messStream)
{
    TRACE_ENTER();
    TRACE_EXIT();
    return out;
}
