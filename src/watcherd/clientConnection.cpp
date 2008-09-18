#include "clientConnection.hpp"

#include <vector>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "dataMarshaller.hpp"
#include "gpsMessage.h"
#include "message.h"

using namespace watcher;
using namespace boost;
using namespace boost::asio::ip;

INIT_LOGGER(ClientConnection, "ClientConnection");

ClientConnection::ClientConnection(
        boost::asio::io_service& io_service, 
        const std::string &server_, 
        const std::string &service_) : 
    theResolver(io_service),
    theSocket(io_service),
    connectionTimeoutTimer(io_service),
    server(server_),
    service(service_)
{
    TRACE_ENTER(); 
    TRACE_EXIT();
}

bool ClientConnection::sendMessage(const boost::shared_ptr<Message> message)
{
    TRACE_ENTER();
    
    if (!connected)
    {
        LOG_DEBUG("Attempted to send message while disconnected - connecting..."); 
        startQuery();
    }
        theRequest = message; 
        dataMarshaller.marshal(theRequest, outBuffers);
        LOG_DEBUG("Sending message: " << *theRequest);
        asio::async_write(theSocket, outBuffers, 
                bind(&ClientConnection::handle_write_message, this, asio::placeholders::error));

    TRACE_EXIT_RET("true");
    return true;
}

tcp::socket& ClientConnection::getSocket()
{
    TRACE_ENTER(); 
    TRACE_EXIT();
    return theSocket;
}

void ClientConnection::startQuery()
{
    TRACE_ENTER();

    tcp::resolver::query query(server, service);
    theResolver.async_resolve(query, 
            bind(&ClientConnection::handle_resolve, this, asio::placeholders::error, asio::placeholders::iterator));

    TRACE_EXIT();
}

void ClientConnection::handle_resolve(const boost::system::error_code &e, tcp::resolver::iterator endpoint_iterator)
{
    TRACE_ENTER();
    if (!e)
    {
        tcp::endpoint endpoint = *endpoint_iterator;
        theSocket.async_connect(endpoint,
                boost::bind(&ClientConnection::handle_connect, this,
                    boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
        LOG_WARN("Error resolving server: " << e.message());
    }
    TRACE_EXIT();
}

void ClientConnection::handle_connect(const boost::system::error_code& e, tcp::resolver::iterator endpoint_iterator)
{
    if (!e)
    {
        LOG_DEBUG("Connected to server. Sending test message");
        connected=true;
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
        LOG_DEBUG("Unable to connect to endpoint, trying the next one.");
        theSocket.close();
        tcp::endpoint endpoint = *endpoint_iterator;
        theSocket.async_connect(endpoint, bind(&ClientConnection::handle_connect, 
                    this, asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
        LOG_DEBUG("Unable to connect to the server at all.");
        LOG_WARN("Unable to connect to server, trying again in 5 seconds.");
        connectionTimeoutTimer.expires_from_now(boost::posix_time::seconds(5));
        connectionTimeoutTimer.async_wait(bind(&ClientConnection::startQuery, this));
    }
    TRACE_EXIT();
}

void ClientConnection::handle_write_message(const boost::system::error_code &e)
{
    TRACE_ENTER();

    if (!e)
    {
        LOG_DEBUG("Sucessfully sent message. Now will async read response"); 
        theSocket.async_read_some(boost::asio::buffer(inBuffer),
                boost::bind(&ClientConnection::handle_read_response, this, 
                    asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    else
    {
        LOG_WARN("Error while writing message: " << e.message());
        connected=false;
    }

    TRACE_EXIT();
}

void ClientConnection::handle_read_response(const boost::system::error_code &e, std::size_t bytes_transferred)
{
    TRACE_ENTER();

    if (!e)
    {
        LOG_DEBUG("Received response from server, parsing it. Read " << bytes_transferred << " bytes.");

        boost::logic::tribool result;
        result = dataMarshaller.unmarshal(theResponse, &inBuffer[0], bytes_transferred);

        if (result)
        {
            LOG_DEBUG("Successfully parsed response from server: " << *theResponse);
        }
        else if (!result)
        {
            LOG_WARN("Unable to parse incoming server response");
        }
        else
        {
            LOG_WARN("Unable to parse incoming message - maybe didn't get entire message"); 
        }
    }
    else
    {
        LOG_WARN("Error reading response from server: " << e.message());
        connected=false;
    }

    TRACE_EXIT();
}


