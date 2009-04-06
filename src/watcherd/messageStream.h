#ifndef WATCHER_MESSAGE_STREAM_H
#define WATCHER_MESSAGE_STREAM_H

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include "libwatcher/watcherTypes.h"    // for Timestamp
#include "libwatcher/message.h"         // for MessagePtr

#include "client.h"
#include "clientMessageHandler.h"
#include "messageStreamFilter.h"

namespace watcher
{
    using namespace event;  // for libwatcher and messages. 

    class MessageStream;
    typedef boost::shared_ptr<MessageStream> MessageStreamPtr;

    /** 
     * @class MessageStream
     * @author Geoff Lawler <geoff.lawler@sparta.com>
     * @date 2009-04-03
     */
    class MessageStream : 
        public ClientMessageHandler, 
        public boost::enable_shared_from_this<MessageStream>
    {
            /** 
             * MessageStream()
             * Create a message stream. 
             * @param serverName - the machine name running the message server. 
             * @param (optional) startTime - the time to start the stream at. 
             *      0 == give current messages - "real time" stream
             *      non-zero == start message playback at the Timestamp given
             *      default value is 0
             * @param (optional) streamRate - If positive, stream goes forward in time. If negative, then backwards. 
             *      1.0 -- "realtime"
             *      < 1.0 -- slower than "real time" by factor given
             *      > 1.0 -- faster than "real time" by factor given
             *      default value is 1.0
             */
             MessageStream(const std::string &serverName, const Timestamp &startTime=0, const float streamRate=1.0); 

        public:

            static MessageStreamPtr createNewMessageStream(
                    const std::string &serverName, 
                    const Timestamp &startTime=0, 
                    const float streamRate=1.0);

            void initConnection();

            /**
             * Death to all humans
             */
            virtual ~MessageStream();

            /**
             * startStream()
             * Start the stream of messages from the message server.
             * @return bool: true if started, false if not (connection error, stream already started).
             */
            bool startStream();

            /**
             * stopStream()
             * Stop the stream of messages from the message server.
             * @return bool: true if stopped, false if not (connection error, stream already stopped).
             */
            bool stopStream();

            /**
             * setStreamTimeStart()
             * Set or reset the stream's start time. 
             * @param startTime - the time to start the stream at. 
             *      0 == give current messages - "real time" stream
             *      non-zero == start message playback at the Timestamp given
             * @return - always returns true
             */
             bool setStreamTimeStart(const Timestamp &startTime); 

            /**
             * setStreamRate()
             * Set or reset the message streams playback rate.
             * @param streamRate - If positive, stream goes forward in time. If negative, then backwards. 
             *                     1.0 -- "realtime"
             *                     < 1.0 -- slower than "real time" by factor given
             *                     > 1.0 -- faster than "real time" by factor given
             * @return - always returns true
             */
            bool setStreamRate(const float &messageStreamRate);

            /**
             * getNextMessage(MessagePtr newMessage)
             * This function blocks until the next message arrives from the watcherd instance connected to. 
             * isStreamReadable() can be used to see if the call to getNextMessage() would block or not.
             * @param MessagePtr - the next message in the message stream
             * @return - returns false on read message error - watcherd disconnect
             */
            bool getNextMessage(MessagePtr newMessage);

            /**
             * isStreamReadable()
             * Returns true if a call to getNextMessage() would return immediately.
             * @return a boolean: true if getNextMessage() would not block, false otherwise
             */
            bool isStreamReadable() const;

            /**
             * bool filterMessageStream(const MessageStreamFilter &filter)
             * This method filters the existing message stream by the filter given.
             * Once set, the stream will only contist of messages which pass the filter. More than
             * one filter can be added and the filters are additive. 
             * @param - MessageStreamFilter - the filter to apply to the stream. 
             * @return - always returns true
             */
             bool addMessageFilter(const MessageStreamFilter &filter);

            /**
             * getTimeRange()
             * This function queries the server and returns the earliest and latest (most recent)  
             * timestamps for all messages known by the server.
             * @param Timestamp earliest - earliest (least recent) message time known.
             * @param Timestamp latest - latest (most recent)  message time known.
             */
            bool getMessageTimeRange(Timestamp &startTime, Timestamp endTime); 

            /**
             * Write an instance of this class as a human readable stream to the otream given
             */
            virtual std::ostream &toStream(std::ostream &out) const;

            /**
             * Write an instance of this class as a human readable stream to the otream given.
             * Just calls MessageStream::toStream().
             */
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            /**
             * Handle the arrival of a message. Overridden from ClientMessageHandler base class.
             */
            virtual bool handleMessageArrive(const MessagePtr message, MessagePtr &response);

        protected:

        private:
            DECLARE_LOGGER();

            typedef std::vector<MessageStreamFilterPtr> MessageStreamFilterList;
            typedef MessageStreamFilterList::iterator MessageStreamFilterListIterator;
            typedef MessageStreamFilterList::const_iterator MessageStreamFilterListConstIterator;
            MessageStreamFilterList messageStreamFilters;

            float streamRate;
            Timestamp streamStartTime;

            MessageStream(const MessageStream &ms); /// No copies allowed though I don't know why.
            MessageStream &operator=(const MessageStream &ms); /// No operator = allowed.

            /** This is the connection to the message server (watcherd instance) used to send/recv messages */
            ClientPtr connection;

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
