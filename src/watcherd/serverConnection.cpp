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

#include "sharedStream.h"
#include "serverConnection.h"
#include <vector>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <libwatcher/message.h>
#include <libwatcher/messageStatus.h>
#include <libwatcher/seekWatcherMessage.h>
#include <libwatcher/speedWatcherMessage.h>
#include <libwatcher/nodeStatusMessage.h>
#include <libwatcher/playbackTimeRange.h>
#include <libwatcher/messageStreamFilterMessage.h>
#include <libwatcher/subscribeStreamMessage.h>
#include <libwatcher/listStreamsMessage.h>
#include <libwatcher/streamDescriptionMessage.h>

#include "watcherd.h"
#include "writeDBMessageHandler.h"
#include "watcherdConfig.h"
#include "replayState.h"
#include "database.h"
#include "logger.h"
#include "singletonConfig.h"

using namespace std; 
using namespace boost::asio;

namespace {
    using namespace watcher::event;

    /* Grr, because there is no std::copy_if have to use the negation with
     * remove_copy_if() */
    bool not_feeder_message(const MessagePtr& m)
    {
        return !isFeederEvent(m->type);
    }
}

namespace watcher {
    using namespace event;

    INIT_LOGGER(ServerConnection, "Connection.ServerConnection");

    ServerConnection::ServerConnection(Watcherd& w, boost::asio::io_service& io_service) :
        Connection(io_service),
        watcher(w),
        io_service_(io_service),
        strand_(io_service),
        write_strand_(io_service),
	incomingBuffer(DataMarshaller::header_length), // ensure enough space to read the payload header
        conn_type(unknown),
        dataNetwork(0),
        messageStreamFilterEnabled(false)
    {
        TRACE_ENTER();
        libconfig::Config &cfg=SingletonConfig::instance();
        libconfig::Setting &root=cfg.getRoot();
        try {
            string value;
            if (root.lookupValue("dataNetwork", value)) {
                boost::system::error_code ec;
                dataNetwork=ip::address_v4::from_string(value, ec);
                if (ec) {
                    LOG_ERROR("Error parsing dataNetwork value from configuration file: " << ec);
                    exit (1); // GTL too harsh?
                }
            }
        }
        catch (const libconfig::SettingException &e) {
            LOG_ERROR("Error reading \"dataNetwork\" from configuration at " << e.getPath() << ": " << e.what());
        }

        TRACE_EXIT();
    }

    ServerConnection::~ServerConnection()
    {
        TRACE_ENTER();
        //shared_from_this() not allowed in destructor
        //watcher.unsubscribe(shared_from_this());
        TRACE_EXIT();
    }

    /** Initialization point for start of new ServerConnection thread. */
    void ServerConnection::run()
    {
        TRACE_ENTER(); 
        boost::asio::async_read(
                theSocket, 
                boost::asio::buffer(incomingBuffer, DataMarshaller::header_length),
                strand_.wrap(
                    boost::bind(
                        &ServerConnection::handle_read_header, 
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
        TRACE_EXIT();
    }

    void ServerConnection::read_error(const boost::system::error_code &e)
    {
        TRACE_ENTER();

        if (e == boost::asio::error::eof) {
            LOG_DEBUG("Received empty message from client or client closed connection.");
            LOG_INFO("Connection to client closed."); 
        } else
            LOG_ERROR("Error reading socket: " << e.message());

        // unsubscribe to event stream, otherwise it will hold a
        // shared_ptr open
        if (conn_type == gui)
	    stream->unsubscribe(shared_from_this());

        TRACE_EXIT();
    }

    void ServerConnection::handle_read_header(const boost::system::error_code& e, size_t bytes_transferred)
    {
        TRACE_ENTER(); 
        if (!e)
        {
            LOG_DEBUG("Read " << bytes_transferred << " bytes."); 

            size_t payloadSize;
            unsigned short numOfMessages;
            if (!DataMarshaller::unmarshalHeader(&incomingBuffer[0], bytes_transferred, payloadSize, numOfMessages))
            {
                LOG_ERROR("Error parsing incoming message header.");

                if (conn_type == gui)
                    stream->unsubscribe(shared_from_this());
            }
            else
            {
                LOG_DEBUG("Reading packet payload of " << payloadSize << " bytes.");

		// ensure there is enough space to read the full payload
		if (incomingBuffer.size() < payloadSize)
		{
		    LOG_INFO("increasing incomingBuffer to " << payloadSize << " bytes.");
		    incomingBuffer.resize(payloadSize);
		}
                boost::asio::async_read(
                        theSocket, 
                        boost::asio::buffer(
                            incomingBuffer, 
                            payloadSize),  // Should incoming buffer be new'd()? 
                        strand_.wrap(
                            boost::bind(
                                &ServerConnection::handle_read_payload,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred, 
                                numOfMessages)));
            }
        }
        else
            read_error(e);
        TRACE_EXIT();
    }

    void ServerConnection::seek(event::MessagePtr& m)
    {
        TRACE_ENTER();
        SeekMessagePtr p = boost::dynamic_pointer_cast<SeekMessage>(m);
        if (p)
	    stream->seek(p);
        else
            LOG_WARN("unable to dynamic_pointer_cast to SeekMessage!");
        TRACE_EXIT();
    }

    void ServerConnection::start(event::MessagePtr&)
    {
        TRACE_ENTER();
	stream->start();
        TRACE_EXIT();
    }

    void ServerConnection::stop(event::MessagePtr&)
    {
        TRACE_ENTER();
	stream->stop();
        TRACE_EXIT();
    }

    void ServerConnection::speed(event::MessagePtr& m)
    {
        TRACE_ENTER();
        SpeedMessagePtr p = boost::dynamic_pointer_cast<SpeedMessage>(m);
        if (p) {
	    stream->speed(p);
        } else {
            LOG_WARN("unable to dynamic_pointer_cast to SpeedMessage!");
        }
        TRACE_EXIT();
    }

    /** Returns a PlaybackTimeRangeMessage event to the sender with the timestamps of
     * the first and last event in the database.
     */
    void ServerConnection::range(event::MessagePtr& m)
    {
        PlaybackTimeRangeMessagePtr p (boost::dynamic_pointer_cast<PlaybackTimeRangeMessage>(m));
        if (p)
	    stream->range(shared_from_this());
        else
            LOG_WARN("unable to cast to PlaybackTimeRangeMessage");
    }

    void ServerConnection::filter(event::MessagePtr& m)
    {
        MessageStreamFilterMessagePtr p (boost::dynamic_pointer_cast<MessageStreamFilterMessage>(m));
        if (p) { 
            messageStreamFilterEnabled=p->enableAllFiltering;
            if (p->applyFilter) { 
                LOG_DEBUG("Adding message filter: " << p->theFilter);
                messageStreamFilters.push_back(p->theFilter);
            }
            else {
                LOG_DEBUG("Removing message filter: " << p->theFilter);
                messageStreamFilters.remove(p->theFilter);
            }
            LOG_DEBUG("There are now " << messageStreamFilters.size() << " filters on this stream:"); 
            BOOST_FOREACH(const MessageStreamFilter &f, messageStreamFilters) 
                LOG_DEBUG("     " << f); 
        } else
            LOG_WARN("unable to cast to MessageStreamFilterMessagePtr");
    }

    void ServerConnection::subscribeToStream(MessagePtr& m)
    {
	TRACE_ENTER();
	SubscribeStreamMessagePtr p = boost::dynamic_pointer_cast<SubscribeStreamMessage>(m);
	if (p) {
	    if (p->uid == stream->getUID())
		LOG_WARN("client resubscribed to same uid " << p->uid);
	    else {
		SharedStreamPtr newstream = watcher.getStream(p->uid);
		if (newstream) {
		    LOG_INFO("client unsubscribed from stream " << stream->getUID());
		    stream->unsubscribe(shared_from_this());
		    LOG_INFO("client subscribed to stream " << p->uid);
		    stream = newstream;
		    stream->subscribe(shared_from_this());
		}
		else
		    LOG_WARN("client attempted to subscribe to non-existant stream uid " << p->uid);
	    }
	} else
	    LOG_WARN("unable to cast MessagePtr to SubscribeStreamMessagePtr");
	TRACE_EXIT();
    }

    void ServerConnection::description(MessagePtr& m)
    {
	TRACE_ENTER();
	StreamDescriptionMessagePtr p = boost::dynamic_pointer_cast<StreamDescriptionMessage>(m);
	if (p) {
	    stream->setDescription(p);
	    LOG_INFO("set description for stream " << stream->getUID() << ": " << p->desc);
	} else
	    LOG_WARN("unable to cast MessagePtr to StreamDescriptionMessagePtr");
	TRACE_EXIT();
    }

    void ServerConnection::listStreams(MessagePtr&)
    {
	TRACE_ENTER();
	watcher.listStreams(shared_from_this());
	TRACE_EXIT();
    }

    bool ServerConnection::dispatch_gui_event(MessagePtr& m)
    {
        static const struct {
            MessageType type;
            void (ServerConnection::*fn)(event::MessagePtr&);
        } dispatch[] = {
            { START_MESSAGE_TYPE, &ServerConnection::start },
            { STOP_MESSAGE_TYPE, &ServerConnection::stop },
            { SEEK_MESSAGE_TYPE, &ServerConnection::seek },
            { SPEED_MESSAGE_TYPE, &ServerConnection::speed },
            { PLAYBACK_TIME_RANGE_MESSAGE_TYPE, &ServerConnection::range },
            { MESSAGE_STREAM_FILTER_MESSAGE_TYPE, &ServerConnection::filter },
	    { SUBSCRIBE_STREAM_MESSAGE_TYPE, &ServerConnection::subscribeToStream },
	    { STREAM_DESCRIPTION_MESSAGE_TYPE, &ServerConnection::description },
	    { LIST_STREAMS_MESSAGE_TYPE, &ServerConnection::listStreams },
            { UNKNOWN_MESSAGE_TYPE, 0 }
        };

        TRACE_ENTER();

        for (size_t i = 0; dispatch[i].type != UNKNOWN_MESSAGE_TYPE; ++i) {
            if (m->type == dispatch[i].type) {
                if (conn_type == unknown) {
                    conn_type = gui;
		    stream.reset(new SharedStream(watcher));
		    stream->subscribe(shared_from_this());
		    watcher.addStream(stream);
                }
                (this->*(dispatch[i].fn)) (m);
                TRACE_EXIT_RET_BOOL(true);
                return true;
            }
        }
        TRACE_EXIT_RET_BOOL(false);
        return false;
    }

    void ServerConnection::handle_read_payload(const boost::system::error_code& e, size_t bytes_transferred, unsigned short numOfMessages)
    {
        TRACE_ENTER();

        if (!e)
        {
            vector<MessagePtr> arrivedMessages; 
            if (DataMarshaller::unmarshalPayload(arrivedMessages, numOfMessages, &incomingBuffer[0], bytes_transferred))
            {
                boost::system::error_code err;
                boost::asio::ip::tcp::endpoint ep = getSocket().remote_endpoint(err);
                if (err) { 
                    LOG_INFO("Lost connection to client, cleaning up connection"); 
                    read_error(err); // not really a read error, but this cleans up the connection.
                    TRACE_EXIT();
                    return;
                }

                LOG_INFO("Recvd " << arrivedMessages.size() << " message" <<
                        (arrivedMessages.size()>1?"s":"") << " from " <<
                        ep.address()); 

                BOOST_FOREACH(MessagePtr m, arrivedMessages) {
                    if (isFeederEvent(m->type)) {
			// sanity check
			if (conn_type == gui) {
			    LOG_ERROR("received feeder event from a detected GUI client!  Closing connection.");
			    TRACE_EXIT();
			    return;
			}

			// Add the incoming address to the Message so everyone
			// knows who the message came from. If there is a dataNetwork, use that 
			// to mask/modify the incoming ip address to be in the correct network.
                        if (m->fromNodeID==NodeIdentifier() && dataNetwork.to_ulong()!=0) { 
                            unsigned long mask=ip::address_v4::netmask(dataNetwork).to_ulong();
                            m->fromNodeID=ip::address_v4((ep.address().to_v4().to_ulong() & ~mask) | (mask & dataNetwork.to_ulong()));
                        }

			if (conn_type == unknown) {
                            conn_type = feeder;

                            if (! watcher.readOnly()) {
                                /*
                                 * This connection is a watcher test daemon.
                                 * Add a message handler to write its event
                                 * stream to the database.
                                 */
				    LOG_DEBUG("adding handle to write to event db");
                                addMessageHandler(MessageHandlerPtr(new WriteDBMessageHandler()));
                            }
			}
                    } else if (conn_type == feeder) { // sanity check, anything else should be a gui control event
			LOG_ERROR("received GUI control event from a detected feeder daemon!  Closing connection.");
			TRACE_EXIT();
			return;
		    } else // otherwise must be a GUI control event
			dispatch_gui_event(m);
                }

                /* Flag indicating whether to continue reading from this
                 * connection. */
                bool fail = false;

                BOOST_FOREACH(MessageHandlerPtr& mh, messageHandlers) {
                    if (mh->handleMessagesArrive(shared_from_this(), arrivedMessages)) {
                        fail = true;
                        LOG_DEBUG("Message handler told us to close this connection."); 
                    }
                }

                if (!fail) {
                    // initiate request to read next message
                    LOG_DEBUG("Waiting for next message.");
                    run();
                }
            }
        }
        else
            read_error(e);

        // If an error occurs then no new asynchronous operations are started. This
        // means that all shared_ptr references to the ServerConnection object will
        // disappear and the object will be destroyed automatically after this
        // handler returns. The ServerConnection class's destructor closes the socket.

        TRACE_EXIT();
    }

    void ServerConnection::handle_write(const boost::system::error_code& e, size_t bytes_transferred, MessagePtr message, DataMarshaller::NetworkMarshalBuffersPtr outBuf)
    {
        TRACE_ENTER(); 

	{
	    boost::mutex::scoped_lock lock(outBuffersLock);
	    outBuffers.remove(outBuf);
	}

        if (!e)
        {
            LOG_DEBUG("Successfully sent message to client(size=" << bytes_transferred << "): " << message); 

            BOOST_FOREACH(MessageHandlerPtr mh, messageHandlers)
            {
#if 0
                /* melkins
                 * The reads and writes to the socket are asynchronous, so
                 * we should never be waiting for something to be read as
                 * a result of a write.
                 */
                if(waitForResponse) // someone already said they wanted a response, so ignore ret val for others
                    mh->handleMessageSent(message);
                else
                    waitForResponse=mh->handleMessageSent(message);
#endif
                    mh->handleMessageSent(message);
            }

            // melkins
            // start() calls async_read(), which is not what we want to do here
            /*
            if(waitForResponse)
                start(); 
                */
        }
        else
        {
            LOG_WARN("Error while sending response to client: " << e);
            if (conn_type == gui)
                stream->unsubscribe(shared_from_this());
        }

        // No new asynchronous operations are started. This means that all shared_ptr
        // references to the connection object will disappear and the object will be
        // destroyed automatically after this handler returns. The connection class's
        // destructor closes the socket.

        /* NOTE: The refcount will not go to zero so long as there is an
         * async_read operation also oustanding. */

        TRACE_EXIT();
    }

    /** Send a single message to this connected client. */
    void ServerConnection::sendMessage(MessagePtr msg)
    {
        TRACE_ENTER();

        if (!messageStreamFilterEnabled) {
            BOOST_FOREACH(const MessageStreamFilter &f, messageStreamFilters) {
                if (!f.passFilter(msg)) {
                    LOG_DEBUG("Not sending message as it did not pass the current set of message filters"); 
                    TRACE_EXIT();
                    return;
                }
                else
                    LOG_DEBUG("Message passes all filters - sending it."); 
            }
        }

	// pointer to memory must persist until async handler is completed
        DataMarshaller::NetworkMarshalBuffersPtr outBuf(new DataMarshaller::NetworkMarshalBuffers);
        DataMarshaller::marshalPayload(msg, *outBuf);
	// the lock must be released prior to async write because it is possible this thread
	// will enter the handler, and we aren't using re-entrant lock
	{
	    boost::mutex::scoped_lock lock(outBuffersLock);
	    outBuffers.push_back(outBuf);
	}


        /// FIXME melkins 2004-04-19
        // is it safe to call async_write and async_read from different
        // threads at the same time?  asio::tcp::socket() is listed at not
        // shared thread safe
        async_write(theSocket,
                    *outBuf,
                    write_strand_.wrap( boost::bind( &ServerConnection::handle_write,
                                               shared_from_this(),
                                               placeholders::error,
					       placeholders::bytes_transferred,
                                               msg,
					       outBuf)));
        TRACE_EXIT();
    }

    /** Send a set of messages to this connected client. */
    void ServerConnection::sendMessage(const std::vector<MessagePtr>& msgs)
    {
        TRACE_ENTER();

        std::vector<MessagePtr> messageList;

        if (!messageStreamFilterEnabled)
            messageList=msgs;
        else {
            BOOST_FOREACH(const MessagePtr m, msgs) { 
                bool passed=false;
                // Need to figure out if filters are ANDed or ORed or something else
                // for now if it passes any - it's in.
                BOOST_FOREACH(const MessageStreamFilter &f, messageStreamFilters) 
                    if (f.passFilter(m)) {
                        passed=true;
                        break;
                    }
                if (passed) {  
                    LOG_DEBUG("Message passed at least one filter - sending it."); 
                    messageList.push_back(m); 
                }
                else 
                    LOG_DEBUG("Not sending message as it did not pass any of the current set of message filters"); 
            }

            if (!messageList.size()) { 
                LOG_DEBUG("No messages passed the filters, sending nothing."); 
                TRACE_EXIT();
                return; 
            }
        }


	// pointer to memory must persist until async handler is completed
        DataMarshaller::NetworkMarshalBuffersPtr outBuf(new DataMarshaller::NetworkMarshalBuffers);
        DataMarshaller::marshalPayload(messageList, *outBuf);
	// the lock must be released prior to async write because it is possible this thread
	// will enter the handler, and we aren't using re-entrant lock
	{
	    boost::mutex::scoped_lock lock(outBuffersLock);
	    outBuffers.push_back(outBuf);
	}

        /// FIXME melkins 2004-04-19
        // is it safe to call async_write and async_read from different
        // threads at the same time?
        async_write(theSocket,
                    *outBuf,
                    write_strand_.wrap( boost::bind( &ServerConnection::handle_write,
                                               shared_from_this(),
                                               placeholders::error,
					       placeholders::bytes_transferred,
                                               msgs.front(),
					       outBuf)));
        TRACE_EXIT();
    }

}

// vim:sw=4 ts=8
