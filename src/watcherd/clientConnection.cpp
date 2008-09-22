#include "clientConnection.h"

#include <vector>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "dataMarshaller.h"
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
    connected(false),
    theSocket(io_service),
    ioService(io_service),
    server(server_),
    service(service_)
{
    TRACE_ENTER(); 
    theReply=MessagePtr(new Message);
    TRACE_EXIT();
}

void ClientConnection::doConnect()
{
    TRACE_ENTER();

    LOG_DEBUG("Starting connection sequence to " << server << " running service " << service); 

    while(true)
    {
        tcp::resolver resolver(ioService); 
        tcp::resolver::query query(server, service);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        // Try each endpoint until we successfully establish a connection.
        boost::system::error_code error;
        do 
        {
            theSocket.close();
            LOG_DEBUG("Attempting connect."); 
            theSocket.connect(*endpoint_iterator++, error);
        } while (error && endpoint_iterator != end);

        if (error)
        {
            LOG_WARN("Unable to connect to server, trying again in 5 seconds.");
            boost::asio::deadline_timer t(ioService, boost::posix_time::seconds(5));
            t.wait(); 
        }
        else
        {
            connected=true;
            break;
        }
    }

    TRACE_EXIT();
}

bool ClientConnection::sendMessage(const boost::shared_ptr<Message> message)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    ioService.post(bind(&ClientConnection::doWrite, this, message));

    TRACE_EXIT_RET("true");
    return true;
}

void ClientConnection::doWrite(const MessagePtr &message)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    // GTL - Should there be a mutex access in here somewhere?

    bool writeInProgress=!writeMessages.empty();
    writeMessages.push_back(message);
    if(!writeInProgress)
    {
        outBuffers.clear(); 
        dataMarshaller.marshal(writeMessages.front(), outBuffers);
        LOG_DEBUG("Sending message: " << *writeMessages.front() << " (" << writeMessages.front() << ")");
        int numBytes=0;
        for(OutBuffers::const_iterator i = outBuffers.begin(); i !=  outBuffers.end(); ++i)
            numBytes+=boost::asio::buffer_size(*i);
        LOG_DEBUG("Sending " << numBytes << " bytes"); 
        asio::async_write(theSocket, outBuffers, bind(&ClientConnection::handle_write_message, this, asio::placeholders::error, writeMessages.front()));
    }

    TRACE_EXIT(); 
}

void ClientConnection::doClose()
{
    TRACE_ENTER();
    LOG_DEBUG("Closing the socket"); 
    theSocket.close();
    TRACE_EXIT();
}

void ClientConnection::close()
{
    ioService.post(boost::bind(&ClientConnection::doClose, this));
}

tcp::socket& ClientConnection::getSocket()
{
    TRACE_ENTER(); 
    TRACE_EXIT();
    return theSocket;
}

void ClientConnection::handle_write_message(const boost::system::error_code &e, const MessagePtr messPtr)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    if (!e)
    {
        // No errors, revmoe the message from the outbound message list.
        writeMessages.remove(messPtr); 

        // now read a response
        LOG_DEBUG("Sucessfully sent message " << messPtr << ". Now will async read response"); 
        theSocket.async_read_some(boost::asio::buffer(incomingBuffer),
                boost::bind(&ClientConnection::handle_read_response, this, 
                    asio::placeholders::error, asio::placeholders::bytes_transferred,
                    messPtr));

        // start a new write if we need to
        if (!writeMessages.empty())
        {
            outBuffers.clear(); 
            dataMarshaller.marshal(writeMessages.front(), outBuffers);
            LOG_DEBUG("Sending message: " << *writeMessages.front() << " (" << writeMessages.front() << ")");
            int numBytes=0;
            for(OutBuffers::const_iterator i = outBuffers.begin(); i !=  outBuffers.end(); ++i)
                numBytes+=boost::asio::buffer_size(*i);
            LOG_DEBUG("Sending " << numBytes << " bytes"); 
            asio::async_write(theSocket, outBuffers, bind(&ClientConnection::handle_write_message, this, asio::placeholders::error, writeMessages.front()));
        }
    }
    else
    {
        LOG_WARN("Error while writing message: " << e.message());
        doClose(); 
    }

    TRACE_EXIT();
}

void ClientConnection::handle_read_response(const boost::system::error_code &e, std::size_t bytes_transferred, const MessagePtr messPtr)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    if (!e)
    {
        LOG_DEBUG("Received response from server for message " << messPtr << ", parsing it. Read " << bytes_transferred << " bytes.");

        boost::logic::tribool result;
        size_t bytesUsed;
        result = dataMarshaller.unmarshal(theReply, &incomingBuffer[0], bytes_transferred, bytesUsed); 

        if (result)
        {
            LOG_DEBUG("Successfully parsed response from server for message " << messPtr << ": " << *theReply);
        }
        else if (!result)
        {
            LOG_WARN("Unable to parse incoming server response for message " << messPtr);
        }
        else
        {
            LOG_WARN("Did not get all the bytes for reponse to message " << messPtr << " giving up on it even though I should try to get the rest of the message"); 
        }
        if (bytesUsed != bytes_transferred)
        {
            LOG_DEBUG("Looks like we got more than one message in this read, rescanning the remainder of the buffer"); 
            memcpy(incomingBuffer.c_array(), &incomingBuffer[bytesUsed], bytes_transferred-bytesUsed);
            handle_read_response(e, bytes_transferred-bytesUsed, MessagePtr());
        }
    }
    else
    {
        LOG_WARN("Error reading response from server for message " << messPtr << ": " << e.message());
        doClose(); 
    }

    TRACE_EXIT();
}


