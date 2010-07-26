/* Copyright 2009, 2010 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clientConnection.h"

#include <vector>
#include <string>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "libwatcher/message.h"
#include "messageHandler.h"
#include "logger.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
using namespace boost;
using namespace boost::asio::ip;

INIT_LOGGER(ClientConnection, "Connection.ClientConnection");

ClientConnection::ClientConnection(
        boost::asio::io_service& io_service, 
        const std::string &server_, 
        const std::string &service_) :
    Connection(io_service),
    connected(false),
    ioService(io_service),
    theStrand(io_service),
    writeStrand(io_service),
    server(server_),
    service(service_)
{
    TRACE_ENTER(); 
    TRACE_EXIT();
}

//virtual 
ClientConnection::~ClientConnection()
{
    TRACE_ENTER();
    TRACE_EXIT();
}

void ClientConnection::doClose()
{
    TRACE_ENTER();
    LOG_DEBUG("Closing the socket"); 
    getSocket().close();
    connected = false;
    TRACE_EXIT();
}

void ClientConnection::close()
{
    TRACE_ENTER();
    if (connected) {
	LOG_DEBUG("posting call to doClose()");
	ioService.post(boost::bind(&ClientConnection::doClose, this));
    }
    TRACE_EXIT();
}

bool ClientConnection::connect(bool async)
{
    TRACE_ENTER();

    if (!connected) {
	if (async) {
	    bool rv = tryConnect();
	    TRACE_EXIT_RET(rv);
	    return rv;
	}

	// Don't exit this function until we're connected. connect(false) is synchronous
	while (! tryConnect()) {
	    LOG_WARN("Unable to connect to server, trying again in 2 seconds.");
	    sleep(2);
	}
    }
    TRACE_EXIT_RET(true);
    return true;
}

bool ClientConnection::tryConnect()
{
    TRACE_ENTER();

    LOG_DEBUG("Starting connection sequence to " << server); 

    boost::system::error_code error;
    tcp::resolver resolver(ioService); 
    tcp::resolver::query query(server, service);
    LOG_DEBUG("Connecting to service/port " << service);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);
    tcp::resolver::iterator end;

    if (error)
    {
        connected=false;
        if (error==boost::asio::error::service_not_found)
        {
            LOG_FATAL("watcherd service not found. Please add \"watcherd    8095/tcp\" to your /etc/services file.");
        }
        else if(error==boost::asio::error::host_not_found)
        {
            LOG_FATAL("Unable to resolve hostname of server \"" << server << "\". Please resolve connectivity issues and try again.");
        }
        else
            LOG_FATAL("Resolution error: " << error);

        throw(error);
    }
    else
    {
        LOG_DEBUG("Resolved connection query to " << server);

        // Try each endpoint until we successfully establish a connection.
        do 
        {
            theSocket.close();
            LOG_DEBUG("Attempting connect."); 
            theSocket.connect(*endpoint_iterator++, error);

            if (!error)
            {
                /* Store connection information for use by Connection::getPeerAddr() */
                boost::asio::ip::tcp::endpoint ep = theSocket.remote_endpoint();
                endpoint_addr_ = ep.address().to_string();
                endpoint_port_ = ep.port();

                connected=true;
                break;
            }
            else
            {
                connected=false;
                LOG_ERROR("Connection error: " << error);
            }
        } while (endpoint_iterator != end);
    }

    if (connected)
        run(); // start message reader strand

    TRACE_EXIT_RET((connected==true?"true":"false"));
    return connected;
}

bool ClientConnection::sendMessage(const MessagePtr message)
{
    TRACE_ENTER();

    vector<MessagePtr> tmp;
    tmp.push_back(message); 

    bool retVal=sendMessages(tmp); 

    TRACE_EXIT_RET((retVal?"true":"false"));
    return retVal;
}

bool ClientConnection::sendMessages(const vector<event::MessagePtr> &messages)
{
    TRACE_ENTER();

    if (!connected)
        connect();

    LOG_DEBUG("Marshaling outbound message"); 
    DataMarshaller::NetworkMarshalBuffers outBuffers;
    if (!DataMarshaller::marshalPayload(messages, outBuffers)) {
        LOG_WARN("Error marshaling message, not sending"); 
        TRACE_EXIT_RET("false"); 
        return false;
    }

    LOG_INFO("Sending message: " << *(messages.front()) << " (" << messages.front() << ")");
    async_write(theSocket, 
                outBuffers, 
                writeStrand.wrap(bind(&ClientConnection::handle_write_message, 
                                      this, 
                                      asio::placeholders::error, 
                                      messages)));

    TRACE_EXIT_RET("true"); 

    return true;
}

void ClientConnection::handle_write_message(const boost::system::error_code &e, vector<MessagePtr> messages)
{
    TRACE_ENTER();

    if (!e) {
        LOG_DEBUG("Sucessfully sent message " << messages.front()); 

        bool rv = false;
        BOOST_FOREACH(MessageHandlerPtr& mh, messageHandlers) {
            rv |= mh->handleMessagesSent(messages);
        }
        if (rv) {
            LOG_DEBUG("Handler requested shutdown of connection");
            getSocket().shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        }

    }
    else
    {
        LOG_WARN("Error '" << e.message() << "' while writing message: " << messages.front());
        doClose(); 
    }

    TRACE_EXIT();
}

void ClientConnection::run()
{
    async_read(theSocket,
               asio::buffer(incomingBuffer, DataMarshaller::header_length),
               theStrand.wrap(bind(&ClientConnection::handle_read_header,
                                   this,
                                   asio::placeholders::error,
                                   asio::placeholders::bytes_transferred)));
}

void ClientConnection::handle_read_header(const boost::system::error_code &e, std::size_t bytes_transferred)
{
    TRACE_ENTER();

    if (!e) {
        LOG_DEBUG("Recv'd header"); 
        size_t payloadSize;
        unsigned short messageNum;
        if (!DataMarshaller::unmarshalHeader(&incomingBuffer[0], bytes_transferred, payloadSize, messageNum)) {
            LOG_ERROR("Unable to parse incoming message header"); 
        } else {
            LOG_DEBUG("Parsed header - now reading " << messageNum << " message" << (messageNum>1?"s":"") 
                    << " from a buffer of " << payloadSize << " bytes."); 

            bool closeConnection = false;
            size_t bytesRead=0;
            if (payloadSize != (bytesRead=asio::read(theSocket, asio::buffer(incomingBuffer, payloadSize)))) {
                LOG_ERROR("Read " << bytesRead << " bytes when we wanted to read " << payloadSize << " bytes from server.");
                if (bytesRead==incomingBuffer.size() && bytesRead < payloadSize) { 
                    // just read the rest of the messages into the buffer and do nothing. We can't parse messages larger than the buffer size
                    // GTL -- or can we? Look into dynamic sized arrays for this. 
                    // asio::read() can take a basic_stream_buf instance - maybe use that instead of a statically 
                    // sized array?
                    size_t bytesRemaining=payloadSize-bytesRead; 
                    while (bytesRead=asio::read(theSocket, asio::buffer(incomingBuffer, bytesRemaining))) { 
                        LOG_ERROR("Read " << bytesRead << " more bytes, " << payloadSize << " remaining to read"); 
                        bytesRemaining-=bytesRead; 
                    }
                }
                else 
                    closeConnection=true;
            } else {
                vector<MessagePtr> arrivedMessages; 
                if(!DataMarshaller::unmarshalPayload(arrivedMessages, messageNum, &incomingBuffer[0], payloadSize)) {
                    LOG_WARN("Unable to parse incoming server message ");
                    closeConnection = true;
                } else if (messageHandlers.empty()) {
                    LOG_WARN("Ignoring server response - we don't have a message handler set. (This may be intentional)"); 
                    closeConnection = true;
                } else {
                    BOOST_FOREACH(MessageHandlerPtr mh, messageHandlers) {
                        closeConnection |= mh->handleMessagesArrive(shared_from_this(), arrivedMessages);
                    }
                }
            }

            if (!closeConnection) {
                run(); // start another read
            } else {
                LOG_DEBUG("error occurred, or handler requested connection shut down");
            }
        }
    }
    else
    {
        LOG_DEBUG("Error reading inbound message header: " << e.message()); 
        doClose();
    }

    TRACE_EXIT(); 
}

// vim:sw=4
