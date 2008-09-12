//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"

#include "message.h"
#include "dataMarshaller.hpp"
#include <boost/serialization/vector.hpp> 
#include <boost/serialization/shared_ptr.hpp> 

using namespace watcher;
using namespace watcher::server;

INIT_LOGGER(watcher::server::connection, "connection");

connection::connection(boost::asio::io_service& io_service, request_handler& handler) :
    strand_(io_service),
    socket_(io_service),
    request_handler_(handler)
{
    TRACE_ENTER(); 

    request_=boost::shared_ptr<Message>(new Message);
    reply_=boost::shared_ptr<Message>(new Message);

    TRACE_EXIT();
}

boost::asio::ip::tcp::socket& connection::socket()
{
    TRACE_ENTER(); 
    TRACE_EXIT();
    return socket_;
}

void connection::start()
{
    TRACE_ENTER(); 
    socket_.async_read_some(boost::asio::buffer(buffer_),
            strand_.wrap(
                boost::bind(
                    &connection::handle_read, 
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    TRACE_EXIT();
}

void connection::handle_read(const boost::system::error_code& e, std::size_t bytes_transferred)
{
    TRACE_ENTER(); 
    if (!e)
    {
        boost::logic::tribool result;
        result = dataMarshaller.unmarshal(request_, buffer_.begin(), bytes_transferred);

        if (result)
        {
            request_handler_.handle_request(request_, reply_);

            outboundDataBuffers.clear(); 
            dataMarshaller.marshal(reply_, outboundDataBuffers);

            boost::asio::async_write(socket_, outboundDataBuffers, 
                    strand_.wrap(
                        boost::bind(
                            &connection::handle_write, 
                            shared_from_this(),
                            boost::asio::placeholders::error)));
        }
        else if (!result)
        {
            // GTL TODO: come up with a generic reply message
            // that should have an ReplyMessage::badRequest
            // entry. Put that in reply_ here and send it off
            // normally.
            // GTL - LOG error.
            // reply_ = reply::stock_reply(reply::bad_request);
            // boost::asio::async_write(socket_, reply_.to_buffers(),
            //         strand_.wrap(
            //             boost::bind(&connection::handle_write, shared_from_this(),
            //                 boost::asio::placeholders::error)));
        
            LOG_DEBUG("------SEND NACK HERE-------------"); 
        }
        else
        {
            // read more data - not enough sent.
            socket_.async_read_some(boost::asio::buffer(buffer_),
                    strand_.wrap(
                        boost::bind(&connection::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred)));
        }
    }

    // If an error occurs then no new asynchronous operations are started. This
    // means that all shared_ptr references to the connection object will
    // disappear and the object will be destroyed automatically after this
    // handler returns. The connection class's destructor closes the socket.

    TRACE_EXIT();
}

void connection::handle_write(const boost::system::error_code& e)
{
    TRACE_ENTER(); 
    if (!e)
    {
        // Initiate graceful connection closure.
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    }

    // No new asynchronous operations are started. This means that all shared_ptr
    // references to the connection object will disappear and the object will be
    // destroyed automatically after this handler returns. The connection class's
    // destructor closes the socket.
    
    TRACE_EXIT();
}

