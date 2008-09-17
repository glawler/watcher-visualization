#include "serverConnection.hpp"
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

#include "dataMarshaller.hpp"

using namespace watcher;

INIT_LOGGER(ServerConnection, "ServerConnection");

ServerConnection::ServerConnection(boost::asio::io_service& io_service) :
    strand_(io_service),
    socket_(io_service)
{
    TRACE_ENTER(); 

    request=boost::shared_ptr<Message>(new Message);
    reply=boost::shared_ptr<Message>(new Message);

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
        boost::logic::tribool result;
        result = dataMarshaller.unmarshal(request, incomingBuffer.begin(), bytes_transferred);

        if (result)
        {
            LOG_DEBUG("Recvd message: " << *request); 

            boost::shared_ptr<MessageHandler> handler = MessageHandlerFactory::getMessageHandler(request->type);

            if (handler->produceReply(request, reply))
            {
                LOG_DEBUG("Sending reply: " << *reply);

                outboundDataBuffers.clear(); 
                dataMarshaller.marshal(reply, outboundDataBuffers);
                LOG_DEBUG("Sending reply message: " << *reply);

                boost::asio::async_write(socket_, outboundDataBuffers, 
                        strand_.wrap(
                            boost::bind(
                                &ServerConnection::handle_write, 
                                shared_from_this(),
                                boost::asio::placeholders::error)));
            }
            else
            {
                LOG_DEBUG("Not sending reply - doesn't need one"); 
                // This execution branch causes this connection to disapear.
            }
        }
        else if (!result)
        {
            LOG_WARN("Did not understand incoming message. Sending back a nack");
            reply=boost::shared_ptr<MessageStatus>(new MessageStatus(MessageStatus::status_nack));
            dataMarshaller.marshal(reply, outboundDataBuffers);
            LOG_DEBUG("Sending NACK as reply: " << *reply);

            boost::asio::async_write(socket_, outboundDataBuffers,
                    strand_.wrap(
                        boost::bind(&ServerConnection::handle_write, shared_from_this(),
                            boost::asio::placeholders::error)));
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
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the ServerConnection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The ServerConnection class's destructor closes the socket.

    TRACE_EXIT();
}

void ServerConnection::handle_write(const boost::system::error_code& e)
{
    TRACE_ENTER(); 

    if (!e)
    {
        // Initiate graceful connection closure.
        LOG_DEBUG("Connection with client completed; Doing graceful shutdown of ServerConnection"); 
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
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

