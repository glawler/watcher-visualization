/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
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

/**
 * @file messageStream.h
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-04-03
 * @date 2009-07-15
 */
#ifndef WATCHER_MESSAGE_STREAM_H
#define WATCHER_MESSAGE_STREAM_H

#include <string>
#include <deque>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include "libwatcher/watcherTypes.h"    // for Timestamp
#include "libwatcher/message.h"         // for MessagePtr
#include "declareLogger.h"

#include "client.h"
#include "watcherdAPIMessageHandler.h"
#include "messageStreamFilter.h"

namespace watcher
{
    using namespace event;  // for libwatcher and messages. 

    class MessageStream;
    typedef boost::shared_ptr<MessageStream> MessageStreamPtr;

    /** 
     * The MessageStream class can be tested using the command line based client @ref messageStream2Text. 
     */
    class MessageStream : 
        public WatcherdAPIMessageHandler, 
        public boost::enable_shared_from_this<MessageStream>
    {
        public:
        /** 
         * Create a message stream. 
         * @param serverName the machine name running the message server. 
         * @param (optional) startTime the time to start the stream at. 
         *      0 == give current messages - "real time" stream
         *      non-zero == start message playback at the Timestamp given
         *      default value is 0
         * @param streamRate If positive, stream goes forward in time. If negative, then backwards. 
         *      1.0 -- "realtime"
         *      < 1.0 -- slower than "real time" by factor given
         *      > 1.0 -- faster than "real time" by factor given
         *      default value is 1.0
         */
        MessageStream(
                const std::string &serverName, 
                const std::string &service="",  // can be port num or service name. If "" serverName will be parsed for host:service format
                const Timestamp &startTime=0, 
                const float streamRate=1.0); 

        public:

        static MessageStreamPtr createNewMessageStream(
                const std::string &serverName, 
                const Timestamp &startTime=0, 
                const float streamRate=1.0);

        static MessageStreamPtr createNewMessageStream(
                const std::string &serverName, 
                const std::string &portNumber,  // Connect on a non-standard port (different port than watcherd service)
                const Timestamp &startTime=0, 
                const float streamRate=1.0);

        void initConnection();

        /**
         * Death to all humans
         */
        virtual ~MessageStream();

        /**
         * Start the stream of messages from the message server.
         * @retval bool true if started
         * @retval false if not (connection error, stream already started)
         */
        bool startStream();

        /**
         * Stop the stream of messages from the message server.
         * @retval true if stopped
         * @retval false if not (connection error, stream already stopped)
         */
        bool stopStream();

        /**
         * Set or reset the stream's start time. 
         * @param startTime the time to start the stream at. 
         *      SeekMessage::eof == give current messages - "real time" stream
         *      SeekMesssage::epoch == start at beginning
         *      non-zero == start message playback at the Timestamp given
         * @retval true
         */
        bool setStreamTimeStart(const Timestamp &startTime); 

        /**
         * Set or reset the message streams playback rate.
         * @param messageStreamRate  If positive, stream goes forward in time. If negative, then backwards. 
         *                     1.0 -- "realtime"
         *                     < 1.0 -- slower than "real time" by factor given
         *                     > 1.0 -- faster than "real time" by factor given
         * @retval true
         */
        bool setStreamRate(const float &messageStreamRate);

        /**
         * This function blocks until the next message arrives from the watcherd instance connected to. 
         * isStreamReadable() can be used to see if the call to getNextMessage() would block or not.
         * @param newMessage the next message in the message stream
         * @return false on read message error - watcherd disconnect
         */
        bool getNextMessage(MessagePtr &newMessage);

        /**
         * Returns true if a call to getNextMessage() would return immediately.
         * @retval true if getNextMessage() would not block
         * @retval false otherwise
         */
        bool isStreamReadable() const;

        /**
         * This method filters the existing message stream by the filter given.
         * Once set, the stream will only contist of messages which pass the filter. More than
         * one filter can be added and the filters are additive. 
         * @param filter the filter to apply to the stream. 
         * @retval true if watcherd notifed of new filter
         */
        bool addMessageFilter(const MessageStreamFilterPtr filter);

        /** 
         * This method removes a previously applied filter
         */
        bool removeMessageFilter(const MessageStreamFilterPtr filter);

        /** Turns filtering on or off. Returns previous value */
        bool enableFiltering(bool enable); 

        /**
         * Causes the server to respond with a PlaybackTimeRange message at some point in the 
         * near future. 
         */
        bool getMessageTimeRange() const;

        /**
         * Write an instance of this class as a human readable stream to the otream given
         * @param out the output stream
         * @return reference to the output stream
         */
        virtual std::ostream &toStream(std::ostream &out) const;

        /**
         * Write an instance of this class as a human readable stream to the otream given.
         * Just calls MessageStream::toStream().
         * @param out the output stream
         * @return reference to the output stream
         */
        std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

        /**
         * Connect to the server.
         * @param async, If true, connect() will attempt to connect once, and return true/false on success/failure.
         *          If false, connect() will not return until connected, retrying every 2 seconds. Returns true.
         * @retval true connection was established
         * @retval false connection failed
         */
        bool connect(bool async=false); 

        /**
         * @retval true if connected, false otherwise
         */
        bool connected() const;

        /**
         * Clear the message cache. Can be used to clear the cache if the GUI is overloaded
         * or knowns that it does not need the cached data.
         */
        void clearMessageCache(); 

        // Bookkeeping
        unsigned int messagesSent;
        unsigned int messagesArrived;
        unsigned int messagesDropped;
        unsigned int messageQueueSize() { return messageCache.size(); }

	/** Subscribe to an existing message stream on the watcher server.
	 * @param uid unique identifier of the stream to join
	 * @retval true message was sent.
	 * @retval false message send failed.
	 */
	bool subscribeToStream(uint32_t uid);

	/** Fetch a list of the available streams from the watcher server.
	 * @retval true message was sent.
	 * @retval false message send failed.
	 */
	bool listStreams();

	/** Specify a human readable string used to identify this stream.
	 * Watcher GUI clients can request a list of the shared streams using
	 * the ListStreamsMessage.  This string will be associated with the UID
	 * for this stream.
	 */
	bool setDescription(const std::string& desc);

	/** Closes the connection to the server and reconnects.
	 */
	void reconnect();

        protected:

        /**
         * Handle the arrival of a message. Overridden from base class.
         * It is invoked by a thread in a Client instance. 
         * @param[in] conn the connection from which the message was received
         * @param[in] message the newly arrived message. 
         * @return boolean. if false, keep connection open, true otherwise.
         */
        virtual bool handleMessageArrive(ConnectionPtr conn, const MessagePtr &message);

        /**
         * Handle the arrival of mulitple messages. Overridden from base class.
         * It is invoked by a thread in a Client instance. 
         * @param[in] conn the connection from which the message was received
         * @param[in] messages the newly arrived messages. 
         * @return boolean. if false, keep connection open, true otherwise.
         */
        virtual bool handleMessagesArrive(ConnectionPtr conn, const std::vector<event::MessagePtr> &messages); 

        virtual bool handleMessageSent(const event::MessagePtr &message); 
        virtual bool handleMessagesSent(const std::vector<event::MessagePtr> &messages);

        private:
        DECLARE_LOGGER();

        /** 
         * private data 
         **/
        typedef std::vector<MessageStreamFilterPtr> MessageStreamFilterList;
        typedef MessageStreamFilterList::iterator MessageStreamFilterListIterator;
        typedef MessageStreamFilterList::const_iterator MessageStreamFilterListConstIterator;
        MessageStreamFilterList messageStreamFilters;
        bool messageFilteringEnabled;

        float streamRate;
        Timestamp streamStartTime;
        std::string serverName;
        std::string serviceName;

        /** This is the connection to the message server (watcherd instance) used to send/recv messages */
        ClientPtr connection;

        /** Incoming messages are stored in a cache until getNextMessage() is called. */
        typedef std::deque<watcher::event::MessagePtr> MessageCache;
        MessageCache messageCache; 

        /** 
         * Single-writer, multiple-reader around messageCache access as it will be filled 
         * and emptied by different threads.
         **/
        boost::mutex messageCacheMutex;
        boost::condition_variable messageCacheCond;
        bool readReady;

        /** 
         * private methods 
         **/
        MessageStream(const MessageStream &ms); /// No copies allowed though I don't know why.
        MessageStream &operator=(const MessageStream &ms); /// No operator = allowed.

    }; // like a fired school teacher.

    /**
     * typedef a MessageStream shared pointer type
     */
    typedef boost::shared_ptr<MessageStream> MessageStreamPtr;

    /** write a human readable version of the MessageStream class to the ostream given
    */
    std::ostream &operator<<(std::ostream &out, const MessageStream &messStream);

}

#endif // WATCHER_MESSAGE_STREAM_H
