//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by Geoff Lawler @ SPARTA inc.
//

#include "serverConnection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include "testMessage.h"
#include "messageStatus.h"
#include "message.h"

#include "dataMarshaller.hpp"

using namespace watcher;

INIT_LOGGER(watcher::serverConnection, "serverConnection");

serverConnection::serverConnection(boost::asio::io_service& io_service, boost::shared_ptr<request_handler> handler) :
    strand_(io_service),
    socket_(io_service),
    request_handler_(handler)
{
    TRACE_ENTER(); 

    request_=boost::shared_ptr<Message>(new Message);
    reply_=boost::shared_ptr<Message>(new Message);

    TRACE_EXIT();
}

boost::asio::ip::tcp::socket& serverConnection::socket()
{
    TRACE_ENTER(); 
    TRACE_EXIT();
    return socket_;
}

void serverConnection::start()
{
    TRACE_ENTER(); 
    socket_.async_read_some(boost::asio::buffer(buffer_),
            strand_.wrap(
                boost::bind(
                    &serverConnection::handle_read, 
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    TRACE_EXIT();
}

void serverConnection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    TRACE_ENTER(); 
    if (!e)
    {
        boost::logic::tribool result;
        result = dataMarshaller.unmarshal(request_, buffer_.begin(), bytes_transferred);

        if (result)
        {
            LOG_DEBUG("Recvd message: " << *request_); 

            request_handler_->handle_request(request_, reply_);
            LOG_DEBUG("Got reply from handler: " << *reply_);

            outboundDataBuffers.clear(); 
            dataMarshaller.marshal(reply_, outboundDataBuffers);
            LOG_DEBUG("Sending reply message: " << *reply_);

            boost::asio::async_write(socket_, outboundDataBuffers, 
                    strand_.wrap(
                        boost::bind(
                            &serverConnection::handle_write, 
                            shared_from_this(),
                            boost::asio::placeholders::error)));
        }
        else if (!result)
        {
            LOG_WARN("Did not understand incoming message. Sending back a nack");
            reply_=boost::shared_ptr<MessageStatus>(new MessageStatus(MessageStatus::status_nack));
            outboundDataBuffers.clear(); 
            dataMarshaller.marshal(reply_, outboundDataBuffers);
            LOG_DEBUG("Sending message: " << reply_);

            boost::asio::async_write(socket_, outboundDataBuffers,
                    strand_.wrap(
                        boost::bind(&serverConnection::handle_write, shared_from_this(),
                            boost::asio::placeholders::error)));
        }
        else
        {
            // read more data - not enough sent.
            socket_.async_read_some(boost::asio::buffer(buffer_),
                    strand_.wrap(
                        boost::bind(&serverConnection::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the serverConnection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The serverConnection class's destructor closes the socket.

    TRACE_EXIT();
}

void serverConnection::handle_write(const boost::system::error_code& e)
{
    TRACE_ENTER(); 

    if (!e)
    {
        // Initiate graceful connection closure.
        LOG_DEBUG("Connection with client completed; Doing graceful shutdown of serverConnection"); 
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

