#include "serverConnection.h"
#include <vector>
#include <boost/bind.hpp>
#include "messageHandlerFactory.h"

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include "message.h"
#include "messageStatus.h"

#include "dataMarshaller.h"

using namespace watcher;

INIT_LOGGER(ServerConnection, "ServerConnection");

ServerConnection::ServerConnection(boost::asio::io_service& io_service) :
    strand_(io_service),
    socket_(io_service)
{
    TRACE_ENTER(); 

    request=MessagePtr(new Message);

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
    socket_.async_read_some(boost::asio::buffer(incomingBuffer),
            strand_.wrap(
                boost::bind(
                    &ServerConnection::handle_read, 
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    TRACE_EXIT();
}

void ServerConnection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    TRACE_ENTER(); 
    if (!e)
    {
        LOG_DEBUG("Read " << bytes_transferred << " bytes."); 

        boost::logic::tribool result;
        size_t bytesUsed;
        result = dataMarshaller.unmarshal(request, incomingBuffer.begin(), bytes_transferred, bytesUsed);

        if (result)
        {
            LOG_DEBUG("Recvd message: " << *request); 

            boost::shared_ptr<MessageHandler> handler = MessageHandlerFactory::getMessageHandler(request->type);

            if (!handler)
            {
                LOG_WARN("Received unknown message type - ignoring.")
            }
            else 
            {
                MessagePtr reply;
                if (handler->produceReply(request, reply))
                {
                    // Keep track of this reply in case of write error
                    // and so we know when to stop sending replays and we can 
                    // close the socket to the client.
                    replies.push_back(reply); 

                    LOG_DEBUG("Marshalling outbound message"); 
                    outboundDataBuffers.clear(); 
                    dataMarshaller.marshal(reply, outboundDataBuffers);
                    LOG_DEBUG("Sending reply message: " << *reply);
                    boost::asio::async_write(socket_, outboundDataBuffers, 
                            strand_.wrap(
                                boost::bind(
                                    &ServerConnection::handle_write, 
                                    shared_from_this(),
                                    boost::asio::placeholders::error, 
                                    reply))); 
                }
                else
                {
                    LOG_DEBUG("Not sending reply - doesn't need one"); 
                    // This execution branch causes this connection to disapear.
                }
            }
        }
        else if (!result)
        {
            LOG_WARN("Did not understand incoming message. Sending back a nack");
            MessagePtr reply=MessagePtr(new MessageStatus(MessageStatus::status_nack));
            dataMarshaller.marshal(reply, outboundDataBuffers);
            LOG_DEBUG("Sending NACK as reply: " << *reply);

            replies.push_back(reply);
            boost::asio::async_write(socket_, outboundDataBuffers,
                    strand_.wrap(
                        boost::bind(&ServerConnection::handle_write, shared_from_this(),
                            boost::asio::placeholders::error, reply)));
        }
        else
        {
            LOG_WARN("Not enough data received for this message, reading more - hopfully it's there."); 
            // read more data - not enough sent.
            socket_.async_read_some(boost::asio::buffer(incomingBuffer),
                    strand_.wrap(
                        boost::bind(&ServerConnection::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
        }

        if (bytesUsed != bytes_transferred)
        {
            LOG_DEBUG("Looks like we got more than one message in a packet, re-reading with the rest of the buffer"); 
            memcpy(incomingBuffer.c_array(), &incomingBuffer[bytesUsed], bytes_transferred-bytesUsed);
            handle_read(e, bytes_transferred-bytesUsed);
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
            outboundDataBuffers.clear(); 
            dataMarshaller.marshal(replies.front(), outboundDataBuffers);
            LOG_DEBUG("Sending reply message: " << *replies.front());
            boost::asio::async_write(socket_, outboundDataBuffers, 
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

