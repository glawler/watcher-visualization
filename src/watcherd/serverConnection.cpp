#include "serverConnection.h"
#include <vector>
#include <boost/bind.hpp>

#include <libwatcher/message.h>
#include <libwatcher/messageStatus.h>

#include "messageFactory.h"
#include "dataMarshaller.h"

using namespace std; 

namespace watcher {
    using namespace event;

    INIT_LOGGER(ServerConnection, "ServerConnection");

    ServerConnection::ServerConnection(boost::asio::io_service& io_service, MessageHandlerPtr messageHandler_) :
        strand_(io_service),
        socket_(io_service),
        messageHandler(messageHandler_)
    {
        TRACE_ENTER(); 
        TRACE_EXIT();
    }

    boost::asio::ip::tcp::socket& ServerConnection::socket()
    {
        TRACE_ENTER(); 
        TRACE_EXIT();
        return socket_;
    }

    void ServerConnection::start()
    {
        TRACE_ENTER(); 
        boost::asio::async_read(
                                socket_, 
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
                        socket_, 
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
                boost::asio::ip::address nodeAddr(socket_.remote_endpoint().address()); 

                LOG_INFO("Recvd " << arrivedMessages.size() << " message" << (arrivedMessages.size()>1?"s":"") << " from " << nodeAddr); 

                MessagePtr reply;
                if(messageHandler->handleMessagesArrive(arrivedMessages, reply))
                {
                    LOG_DEBUG("Sending back a message (at " << reply << "): " << *reply); 

                    // Keep track of this reply in case of write error
                    // and so we know when to stop sending replays and we can 
                    // close the socket to the client.
                    // GTL Should put a lock around this push_back()
                    replies.push_back(reply); 

                    LOG_DEBUG("Marshalling outbound message"); 

                    DataMarshaller::NetworkMarshalBuffers outBuffers;
                    DataMarshaller::marshalPayload(reply, outBuffers);
                    LOG_INFO("Sending reply: " << *reply);
                    boost::asio::async_write(
                            socket_, 
                            outBuffers,   
                            strand_.wrap(
                                boost::bind(
                                    &ServerConnection::handle_write, 
                                    shared_from_this(),
                                    boost::asio::placeholders::error, 
                                    reply))); 
                }
            }
            else
            {
                LOG_WARN("Did not understand incoming message. Sending back a nack");
                MessagePtr reply=MessagePtr(new MessageStatus(MessageStatus::status_nack));

                DataMarshaller::NetworkMarshalBuffers outBuffers;
                DataMarshaller::marshalPayload(reply, outBuffers);
                LOG_INFO("Sending NACK as reply: " << *reply);

                // GTL Should put a lock around this push_back()
                replies.push_back(reply);
                boost::asio::async_write(
                        socket_, 
                        outBuffers,        
                        strand_.wrap(
                            boost::bind(
                                &ServerConnection::handle_write, 
                                shared_from_this(), 
                                boost::asio::placeholders::error, 
                                reply)));
            }
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
            replies.remove(message);

            if (replies.empty())
            {
                // Initiate graceful connection closure.
                // LOG_DEBUG("Connection with client completed; Doing graceful shutdown of ServerConnection"); 
                // boost::system::error_code ignored_ec;
                // socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
                LOG_DEBUG("Restarting connection to client"); 
                start();
            }
            else
            {
                LOG_DEBUG("Still more replies to send, sending next one."); 
                LOG_DEBUG("Marshalling outbound message"); 

                DataMarshaller::NetworkMarshalBuffers outBuffers;

                // Once we get the symantics of multiple messages per packet down, we can send all these
                // replies in a single packet.
                DataMarshaller::marshalPayload(replies.front(), outBuffers);
                LOG_DEBUG("Sending reply message: " << *replies.front());
                boost::asio::async_write(
                        socket_, 
                        outBuffers,             
                        strand_.wrap(
                            boost::bind(
                                &ServerConnection::handle_write,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                replies.front()))); 
            }
        }
        else
        {
            LOG_WARN("Error while sending response to client: " << e);
        }

        // No new asynchronous operations are started. This means that all shared_ptr
        // references to the connection object will disappear and the object will be
        // destroyed automatically after this handler returns. The connection class's
        // destructor closes the socket.

        TRACE_EXIT();
    }

}
