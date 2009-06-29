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

#include "watcherd.h"
#include "writeDBMessageHandler.h"
#include "watcherdConfig.h"
#include "replayState.h"
#include "database.h"

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
        conn_type(unknown),
        isPlaying_(false), isLive_(true)
    {
        TRACE_ENTER(); 
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

        if (e == boost::asio::error::eof)
        {
            LOG_DEBUG("Received empty message from client or client closed connection.");
            LOG_INFO("Connection to client closed."); 

        }
        else
        {
            LOG_ERROR("Error reading socket: " << e.message());
        }

        // unsubscribe to event stream, otherwise it will hold a
        // shared_ptr open
        if (conn_type == gui)
            watcher.unsubscribe(shared_from_this());

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
            if (!DataMarshaller::unmarshalHeader(incomingBuffer.begin(), bytes_transferred, payloadSize, numOfMessages))
            {
                LOG_ERROR("Error parsing incoming message header.");

                if (conn_type == gui)
                    watcher.unsubscribe(shared_from_this());
            }
            else
            {
                LOG_DEBUG("Reading packet payload of " << payloadSize << " bytes.");

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
        if (p) {
            if (p->offset == SeekMessage::eof) {
                if (isLive_) {
                    // nothing to do
                } else if (replay->speed() >= 0) {
                    // switch to live stream
                    isLive_ = true;
                    replay->pause();
                    if (isPlaying_)
                        watcher.subscribe(shared_from_this());
                } else {
                    replay->seek( SeekMessage::eof );
                }
            } else {
                replay->seek(p->offset);

                // when switching from live to replay while playing, kick off the replay strand
                if (isLive_ && isPlaying_)
                    replay->run();
                isLive_ = false;
            }
        } else {
            LOG_WARN("unable to dynamic_pointer_cast to SeekMessage!");
        }
        TRACE_EXIT();
    }

    void ServerConnection::start(event::MessagePtr&)
    {
        TRACE_ENTER();
        LOG_DEBUG("in: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
        if (!isPlaying_) {
            isPlaying_ = true;
            if (isLive_)
                watcher.subscribe(shared_from_this());
            else
                replay->run();
        }
        LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
        TRACE_EXIT();
    }

    void ServerConnection::stop(event::MessagePtr&)
    {
        TRACE_ENTER();
        LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
        if (isPlaying_) {
            LOG_DEBUG("stopping playback");
            if (isLive_) {
                watcher.unsubscribe(shared_from_this());
                /* save current Timestamp so we can resume from the database */
                isLive_ = false;
                replay->seek( getCurrentTime() );
            } else
                replay->pause();
            isPlaying_ = false;
        } else
            LOG_DEBUG("stop message received, but playback is stopped");
        LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
        TRACE_EXIT();
    }

    void ServerConnection::speed(event::MessagePtr& m)
    {
        TRACE_ENTER();
        LOG_DEBUG("isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
        SpeedMessagePtr p = boost::dynamic_pointer_cast<SpeedMessage>(m);
        if (p) {
            if (p->speed == 0) {
                /* special case, speed==0 means StopMessage.  This is to avoid
                 * a Bad_arg exception from ReplayState. */
                LOG_INFO("got speed==0, emulating StopMessage");
                stop(m);
            } else if (isLive_ && p->speed >= 1.0f) {
                // ignore, can't predict the future
            } else {
                replay->speed(p->speed);
                if (isLive_) {
                    isLive_ = false;
                    /* when transitioning from live playback, automatically
                     * seek to the end of the database.
                     */
                    replay->seek( SeekMessage::eof );
                    if (isPlaying_)
                        replay->run();
                }
            }
        } else {
            LOG_WARN("unable to dynamic_pointer_cast to SpeedMessage!");
        }
        LOG_DEBUG("out: isPlaying_=" << isPlaying_ << ", isLive_=" << isLive_);
        TRACE_EXIT();
    }

    /** Returns a PlaybackTimeRange event to the sender with the timestamps of
     * the first and last event in the database.
     */
    void ServerConnection::range(event::MessagePtr& m)
    {
        PlaybackTimeRangePtr p (boost::dynamic_pointer_cast<PlaybackTimeRange>(m));
        if (p) {
            TimeRange r(event_range());
            p->min_ = r.first;
            p->max_ = r.second;
            sendMessage(p);
        } else
            LOG_WARN("unable to cast to PlaybackTimeRange");
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
            { UNKNOWN_MESSAGE_TYPE, 0 }
        };

        TRACE_ENTER();

        for (size_t i = 0; dispatch[i].type != UNKNOWN_MESSAGE_TYPE; ++i) {
            if (m->type == dispatch[i].type) {
                if (conn_type == unknown) {
                    conn_type = gui;
                    replay.reset(new ReplayState(shared_from_this()));
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
            if (DataMarshaller::unmarshalPayload(arrivedMessages, numOfMessages, incomingBuffer.begin(), bytes_transferred))
            {
                LOG_INFO("Recvd " << arrivedMessages.size() << " message" <<
                         (arrivedMessages.size()>1?"s":"") << " from " <<
                         remoteEndpoint().address()); 

                // Add the incoming address to the Message so everyone
                // knows who the message came from.
                boost::asio::ip::tcp::endpoint ep = getSocket().remote_endpoint();
                BOOST_FOREACH(MessagePtr m, arrivedMessages) {
                    if(m->fromNodeID==NodeIdentifier())  // is empty
                        m->fromNodeID=ep.address();
                }

                /*
                 * If this connection type is unknown or gui, then traverse the
                 * list of arrived messages.  For GUI clients, look for the
                 * STOP_MESSAGE to unsubscribe from the event stream.
                 *
                 * For unknown clients, infer the type from the message
                 * received:
                 * START_MESSAGE => gui
                 * isFeederMessage => feeder
                 */
                if (conn_type == unknown || conn_type == gui) {
                    BOOST_FOREACH(MessagePtr& i, arrivedMessages) {
                        if (dispatch_gui_event(i)) {
                            //empty
                       } else if (isFeederEvent(i->type)) {
                            conn_type = feeder;

                            /*
                             * This connection is a watcher test daemon.
                             * Add a message handler to write its event
                             * stream to the database.
                             */
                            addMessageHandler(MessageHandlerPtr(new WriteDBMessageHandler()));
                        }
                    }
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

                if (conn_type == feeder) {
                    /* relay feeder message to any client requesting the live stream.
                     * Warning: currently there is no check to make sure that a client doesn't
                     * receive a message it just sent.  This should be OK since we are just
                     * relaying feeder messages only, and the GUIs should not be sending
                     * them. */
                    vector<MessagePtr> feeder;
                    remove_copy_if(arrivedMessages.begin(), arrivedMessages.end(), back_inserter(feeder), not_feeder_message);
                    if (! feeder.empty()) {
                        LOG_DEBUG("Sending " << feeder.size() << " feeder messages to clients.");
                        watcher.sendMessage(feeder);
                    }
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

    void ServerConnection::handle_write(const boost::system::error_code& e, MessagePtr message)
    {
        TRACE_ENTER(); 

        if (!e)
        {
            LOG_DEBUG("Successfully sent message to client: " << message); 

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
                watcher.unsubscribe(shared_from_this());
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
        DataMarshaller::NetworkMarshalBuffers outBuffers;
        DataMarshaller::marshalPayload(msg, outBuffers);

        /// FIXME melkins 2004-04-19
        // is it safe to call async_write and async_read from different
        // threads at the same time?  asio::tcp::socket() is listed at not
        // shared thread safe
        async_write(theSocket,
                    outBuffers,
                    write_strand_.wrap( boost::bind( &ServerConnection::handle_write,
                                               shared_from_this(),
                                               placeholders::error,
                                               msg)));
        TRACE_EXIT();
    }

    /** Send a set of messages to this connected client. */
    void ServerConnection::sendMessage(const std::vector<MessagePtr>& msgs)
    {
        TRACE_ENTER();
        DataMarshaller::NetworkMarshalBuffers outBuffers;
        DataMarshaller::marshalPayload(msgs, outBuffers);

        /// FIXME melkins 2004-04-19
        // is it safe to call async_write and async_read from different
        // threads at the same time?
        async_write(theSocket,
                    outBuffers,
                    write_strand_.wrap( boost::bind( &ServerConnection::handle_write,
                                               shared_from_this(),
                                               placeholders::error,
                                               msgs.front())));
        TRACE_EXIT();
    }

}
