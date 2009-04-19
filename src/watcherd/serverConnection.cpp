#include "serverConnection.h"
#include <vector>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

// GTL REMOVE THIS
#include "libwatcher/labelMessage.h"

#include <libwatcher/message.h>
#include <libwatcher/messageStatus.h>

#include "messageFactory.h"
#include "dataMarshaller.h"
#include "watcherd.h"

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
        strand_(io_service),
        write_strand_(io_service)
    {
        TRACE_ENTER(); 
        TRACE_EXIT();
    }

    ServerConnection::~ServerConnection()
    {
        watcher.unsubscribe(shared_from_this());
    }

    void ServerConnection::start()
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
        {
            if (e==boost::asio::error::eof)
            {
                LOG_DEBUG("Received empty message from clienti or client closed connection.");
                LOG_INFO("Connection to client closed."); 

            }
            else
            {
                LOG_ERROR("Error reading socket: " << e.message());
            }

            // unsubscribe to event stream, otherwise it will hold a
            // shared_ptr open
            watcher.unsubscribe(shared_from_this());
        }
        TRACE_EXIT();
    }

    void ServerConnection::handle_read_payload(const boost::system::error_code& e, size_t bytes_transferred, unsigned short numOfMessages)
    {
        TRACE_ENTER();

        if (!e)
        {
            vector<MessagePtr> arrivedMessages; 
            if (DataMarshaller::unmarshalPayload(arrivedMessages, numOfMessages, incomingBuffer.begin(), bytes_transferred))
            {
                boost::asio::ip::address nodeAddr(theSocket.remote_endpoint().address()); 

                LOG_INFO("Recvd " << arrivedMessages.size() << " message" << (arrivedMessages.size()>1?"s":"") << " from " << nodeAddr); 

                /* Flag indicating whether to continue reading from this
                 * connection. */
                bool restart = false;

                BOOST_FOREACH(MessagePtr i, arrivedMessages)
                {
                    if (i->type == START_MESSAGE_TYPE)
                    {
                        /* Client is requesting the live stream of events. */
                        watcher.subscribe(shared_from_this());
                        restart = true; // keep client connection open

                        // GTL THIS NEEDS TO GO ELSEWHERE - JUST TESTING MESSAGE STREAM ---------START----------------
                        vector<MessagePtr> bogusMessages;
                        bogusMessages.push_back(LabelMessagePtr(new LabelMessage("This is a test message 1")));
                        bogusMessages.push_back(LabelMessagePtr(new LabelMessage("This is a test message 2")));
                        bogusMessages.push_back(LabelMessagePtr(new LabelMessage("This is a test message 3")));

                        LOG_INFO("Sending bogus data back to startMessage sender."); 
                        sendMessage(bogusMessages);
                        /*
                           DataMarshaller::NetworkMarshalBuffers outBuffers;
                           DataMarshaller::marshalPayload(bogusMessages, outBuffers);
                           boost::asio::async_write(theSocket, outBuffers,   strand_.wrap( boost::bind( &ServerConnection::handle_write, shared_from_this(), boost::asio::placeholders::error, bogusMessages.front())));
                           */
                        // GTL THIS NEEDS TO GO ELSEWHERE - JUST TESTING MESSAGE STREAM ---------END----------------
                    }
                    else if (i->type == STOP_MESSAGE_TYPE)
                    {
                        watcher.unsubscribe(shared_from_this());
                    }
                }

                BOOST_FOREACH(MessageHandlerPtr mh, messageHandlers)
                {
                    if (mh->handleMessagesArrive(arrivedMessages))
                    {
                        restart = true;
                        LOG_DEBUG("Message handler told us to keep this connection open."); 
                    }
                }

                if (restart)
                    start();

                /* relay feeder message to any client requesting the live stream.
                 * Warning: currently there is no check to make sure that a client doesn't
                 * receive a message it just sent.  This should be OK since we are just
                 * relaying feeder messages only, and the GUIs should not be sending
                 * them. */
                vector<MessagePtr> feeder;
                remove_copy_if(arrivedMessages.begin(), arrivedMessages.end(), back_inserter(feeder), not_feeder_message);
                if (! feeder.empty())
                    watcher.sendMessage(feeder);
            }
        }
        else
        {
            LOG_WARN("Did not understand incoming message."); 

            // unsubscribe to event stream, otherwise it will hold a
            // shared_ptr open
            watcher.unsubscribe(shared_from_this());
        }

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

            bool waitForResponse=false;
            BOOST_FOREACH(MessageHandlerPtr mh, messageHandlers)
            {
                if(waitForResponse) // someone already said they wanted a response, so ignore ret val for others
                    mh->handleMessageSent(message);
                else
                    waitForResponse=mh->handleMessageSent(message);
            }
            if(waitForResponse)
                start(); 
        }
        else
        {
            LOG_WARN("Error while sending response to client: " << e);
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
