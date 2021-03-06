//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Modified by Geoff Lawler, SPARTA inc.
//

#include "server.h"
#include "logger.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

using namespace watcher;

INIT_LOGGER(watcher::Server, "Server");

Server::Server(
               Watcherd& w,
        const std::string& hostsvc, 
        const std::string& service, 
        std::size_t thread_pool_size,
        MessageHandlerPtr messageHandler_) :
    watcher(w),
    thread_pool_size_(thread_pool_size),
    acceptor_(io_service_),
    messageHandler(messageHandler_)
{
    TRACE_ENTER();

    new_connection_=ServerConnectionPtr(new ServerConnection(watcher, io_service_));
    new_connection_->addMessageHandler(messageHandler);

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(Connection::getServerHost(hostsvc),
						Connection::getServerService(hostsvc, service));
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    acceptor_.open(endpoint.protocol());

    // GSM - fix ZeroWindow errors
    boost::asio::ip::tcp::no_delay option(true);
    acceptor_.set_option(option);

    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    acceptor_.async_accept(
            new_connection_->getSocket(),
            boost::bind(
                &Server::handle_accept, 
                this, 
                boost::asio::placeholders::error));

    TRACE_EXIT();
}

void Server::run()
{
    TRACE_ENTER();
    // Create a pool of threads to run all of the io_services.
    std::vector<boost::shared_ptr<boost::thread> > threads;
    for (std::size_t i = 0; i < thread_pool_size_; ++i)
    {
        boost::shared_ptr<boost::thread> thread(new boost::thread(boost::bind( &boost::asio::io_service::run, &io_service_)));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads.size(); ++i)
        threads[i]->join();

    TRACE_EXIT();
}

void Server::stop()
{
    TRACE_ENTER();
    io_service_.stop();
    TRACE_EXIT();
}

void Server::handle_accept(const boost::system::error_code& e)
{
    TRACE_ENTER();
    if (!e)
    {
       new_connection_->run();
        new_connection_.reset(new ServerConnection(watcher, io_service_));
        acceptor_.async_accept(
                new_connection_->getSocket(),
                boost::bind(
                    &Server::handle_accept, 
                    this,
                    boost::asio::placeholders::error));
    }
    TRACE_EXIT();
}

