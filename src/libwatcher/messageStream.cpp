/* Copyright 2009,2010 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>

#include "messageStream.h"
#include "libwatcher/startWatcherMessage.h"
#include "libwatcher/stopWatcherMessage.h"
#include "libwatcher/seekWatcherMessage.h"
#include "libwatcher/speedWatcherMessage.h"
#include "libwatcher/playbackTimeRange.h"
#include "libwatcher/messageStreamFilterMessage.h"
#include "libwatcher/listStreamsMessage.h"
#include "libwatcher/subscribeStreamMessage.h"
#include "libwatcher/streamDescriptionMessage.h"
#include "logger.h"

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
    messagesDropped(0),
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
MessageStreamPtr MessageStream::createNewMessageStream(const string &serverName_,
                                                       const Timestamp &startTime_,
                                                       const float streamRate_,
                                                       bool reconnect_)
{
    TRACE_ENTER();
    MessageStreamPtr retVal(new MessageStream(serverName_,"watcherd",startTime_,streamRate_));
    retVal->initConnection(reconnect_); 
    TRACE_EXIT();
    return retVal;
}

// static 
MessageStreamPtr MessageStream::createNewMessageStream(
                    const std::string &serverName, 
                    const std::string &portNum,  // Connect on a non-standard port (different port than watcherd service)
                    const Timestamp &startTime, 
                    const float streamRate,
                    bool reconnect_)
{
    TRACE_ENTER();
    MessageStreamPtr retVal(new MessageStream(serverName, portNum, startTime, streamRate));
    retVal->initConnection(reconnect_); 
    TRACE_EXIT();
    return retVal;
}

void MessageStream::initConnection(bool reconnect_) 
{
    TRACE_ENTER();
    connection=ClientPtr(new Client(serverName, serviceName, reconnect_)); 
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
        messageCacheCond.wait(lock); 
    // assert(messageCache.size() > 0); 
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

bool MessageStream::addMessageFilter(const MessageStreamFilterPtr filter)
{
    TRACE_ENTER();
    MessageStreamFilterMessagePtr mess(new MessageStreamFilterMessage(*filter));
    mess->applyFilter=true;
    mess->enableAllFiltering=messageFilteringEnabled;
    bool retVal=connection->sendMessage(mess);
    TRACE_EXIT_RET_BOOL(retVal); 
    return retVal;
}

bool MessageStream::removeMessageFilter(const MessageStreamFilterPtr filter)
{
    TRACE_ENTER();
    MessageStreamFilterMessagePtr mess(new MessageStreamFilterMessage(*filter));
    mess->applyFilter=false;
    mess->enableAllFiltering=messageFilteringEnabled;
    bool retVal=connection->sendMessage(mess);
    TRACE_EXIT_RET_BOOL(retVal); 
    return retVal;
}
bool MessageStream::enableFiltering(bool enable) {
    bool retVal=messageFilteringEnabled;
    messageFilteringEnabled=enable;
    return retVal;
}

bool MessageStream::getMessageTimeRange() const
{
    TRACE_ENTER();
    PlaybackTimeRangeMessagePtr mess(new PlaybackTimeRangeMessage);
    bool retVal=connection->sendMessage(mess);
    TRACE_EXIT_RET(retVal);
    return retVal;
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

    // if (messageCache.size()>750)  { // Whoa, start dropping messages - the GUI cannot keep up.
    //     // messagesDropped++;
    //     // TRACE_EXIT_RET_BOOL(false);
    //     // return false;
    //     // usleep(100000);
    // }

    // We don't really add anything yet to a generic watcherdAPI client.
    bool retVal=WatcherdAPIMessageHandler::handleMessageArrive(conn, message); 
    {
        lock_guard<mutex> lock(messageCacheMutex);
        messageCache.push_back(message); 
        LOG_DEBUG("MessageCache size=" << messageCache.size());
        readReady=true;
    }
    messageCacheCond.notify_all();
    this_thread::interruption_point();
    LOG_DEBUG("Notified all waiting threads that there is data to be read"); 

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

void MessageStream::clearMessageCache()
{
    TRACE_ENTER();
    {
        lock_guard<mutex> lock(messageCacheMutex);
        messagesDropped+=messageCache.size();
        messageCache.clear();
        readReady=false;
        // let lock go out of scope
    }
    TRACE_EXIT();
}

std::ostream &watcher::operator<<(std::ostream &out, const MessageStream & /*messStream*/)
{
    out << "MessageStream()";
    return out;
}

bool MessageStream::connect()
{
    TRACE_ENTER();
    bool rv = connection->connect();
    TRACE_EXIT_RET_BOOL(rv);
    return rv;
}

bool MessageStream::subscribeToStream(uint32_t uid)
{
    TRACE_ENTER();
    MessagePtr mess (new SubscribeStreamMessage(uid));
    bool retVal=connection->sendMessage(mess);
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool MessageStream::listStreams()
{
    TRACE_ENTER();
    MessagePtr mess (new ListStreamsMessage());
    bool retVal=connection->sendMessage(mess);
    TRACE_EXIT_RET(retVal);
    return retVal;
}

bool MessageStream::setDescription(const std::string& desc)
{
    TRACE_ENTER();
    MessagePtr mess (new StreamDescriptionMessage(desc));
    bool retVal=connection->sendMessage(mess);
    TRACE_EXIT_RET(retVal);
    return retVal;
}
