#include "clientConnection.h"

#include <vector>
#include <string>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "messageHandler.h"
#include "messageHandlerFactory.h"

using namespace std;
using namespace watcher;
using namespace watcher::event;
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
    theStrand(io_service),
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
    theSocket.close();
    TRACE_EXIT();
}

void ClientConnection::close()
{
    TRACE_ENTER();
    ioService.post(boost::bind(&ClientConnection::doClose, this));
    TRACE_EXIT();
}

void ClientConnection::doConnect()
{
    TRACE_ENTER();
    // Don't exit this function until we're connected. doConnect() is synchronus
    while(false==tryConnect())
    {
        LOG_WARN("Unable to connect to server, trying again in 5 seconds.");
        sleep(5);
    }
    TRACE_EXIT();
}

bool ClientConnection::tryConnect()
{
    TRACE_ENTER();

    LOG_DEBUG("Starting connection sequence to " << server << " running service " << service); 

    try
    {
        boost::system::error_code error;
        tcp::resolver resolver(ioService); 
        tcp::resolver::query query(server, service);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);
        tcp::resolver::iterator end;

        if (error)
        {
            connected=false;
            LOG_DEBUG("Error resolving query: " << error.message()); 
        }
        else
        {
            LOG_DEBUG("Resolved connection query to " << server);
        }

        // Try each endpoint until we successfully establish a connection.
        do 
        {
            theSocket.close();
            LOG_DEBUG("Attempting connect."); 
            theSocket.connect(*endpoint_iterator++, error);

            if (!error)
            {
                connected=true;
                break;
            }
            else
            {
                connected=false;
                LOG_DEBUG("Connection error: " << error.message());
            }
        } while (endpoint_iterator != end);
    }
    catch (const boost::system::system_error &e)
    {
        connected=false;
        if (e.code()==boost::asio::error::service_not_found)
             LOG_INFO("watcherd service not found. Please add \"watcherd    8095/tcp\" to your /etc/services file.")
        else
            LOG_ERROR("Caught connection error: " << e.what() << " : " << e.code());
    }

    TRACE_EXIT_RET((connected==true?"true":"false"));
    return connected;
}

tcp::socket& ClientConnection::getSocket()
{
    TRACE_ENTER(); 
    TRACE_EXIT();
    return theSocket;
}

bool ClientConnection::sendMessage(const MessagePtr message)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    ioService.post(bind(&ClientConnection::doWrite, this, message));

    TRACE_EXIT_RET("true");
    return true;
}

//
// Protocol sequence: 
//      aysnc write -> handle write ->
//      (if more data to send: GOTO 10)
//      async read header -> handle read header ->
//      async read payload -> handle read payload
//
void ClientConnection::doWrite(const MessagePtr &message)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    // GTL - Should there be a mutex access in here somewhere?

    bool writeInProgress=!transferData.empty();
    TransferDataPtr dataPtr=TransferDataPtr(new TransferData);
    dataPtr->theRequest=message;
    dataPtr->theReply=MessagePtr(new Message);
    transferData.push_back(dataPtr);
    if(!writeInProgress)
    {
        LOG_DEBUG("Marshaling outbound message"); 
        outBuffers.clear(); 
        if (!dataMarshaller.marshal(dataPtr->theRequest, outBuffers))
        {
            LOG_WARN("Error marshaling message, not sending"); 
            TRACE_EXIT(); 
            return;
        }
        LOG_INFO("Sending message: " << *dataPtr->theRequest << " (" << dataPtr->theRequest<< ")");
        int numBytes=0;
        for(OutBuffers::const_iterator i = outBuffers.begin(); i !=  outBuffers.end(); ++i)
            numBytes+=boost::asio::buffer_size(*i);
        LOG_DEBUG("Sending " << numBytes << " bytes"); 
        asio::async_write(theSocket, outBuffers, 
                theStrand.wrap(
                    bind(&ClientConnection::handle_write_message, this, 
                        asio::placeholders::error, dataPtr)));
    }

    TRACE_EXIT(); 
}

void ClientConnection::handle_write_message(const boost::system::error_code &e, const TransferDataPtr &dataPtr)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    if (!e)
    {
        // now read a response
        LOG_DEBUG("Sucessfully sent message " << dataPtr << ". Now will async read response"); 
        boost::asio::async_read(theSocket,
                asio::buffer(dataPtr->incomingBuffer, DataMarshaller::header_length),
                theStrand.wrap(
                    bind(&ClientConnection::handle_read_header, this, 
                        asio::placeholders::error, asio::placeholders::bytes_transferred, dataPtr)));

        // No errors, remove the message from the outbound message list.
        transferData.remove(dataPtr); 

        // start a new write if we need to
        if (!transferData.empty())
        {
            outBuffers.clear(); 
            dataMarshaller.marshal(dataPtr->theRequest, outBuffers);
            LOG_DEBUG("Sending message: " << *dataPtr->theRequest << " (" << dataPtr->theRequest << ")");
            int numBytes=0;
            for(OutBuffers::const_iterator i = outBuffers.begin(); i !=  outBuffers.end(); ++i)
                numBytes+=boost::asio::buffer_size(*i);
            LOG_DEBUG("Sending " << numBytes << " bytes"); 
            asio::async_write(theSocket, outBuffers, 
                    theStrand.wrap(bind(&ClientConnection::handle_write_message, 
                            this, asio::placeholders::error, transferData.front())));
        }
    }
    else
    {
        LOG_WARN("Error while writing message: " << e.message());
        doClose(); 
    }

    TRACE_EXIT();
}

void ClientConnection::handle_read_header(const boost::system::error_code &e, std::size_t bytes_transferred, const TransferDataPtr &dataPtr)
{
    TRACE_ENTER();

    if (!connected)
        doConnect();

    if (!e)
    {
        LOG_DEBUG("Recv'd header"); 
        size_t payloadSize;
        if (!dataMarshaller.unmarshalHeader(&dataPtr->incomingBuffer[0], bytes_transferred, payloadSize))
        {
            LOG_ERROR("Unable to parse incoming message header"); 
        }
        else
        {
            LOG_DEBUG("Parsed header - now reading payload of size " << payloadSize); 

            // GTL - Wanted to do an async read for the payload, but kept getting handle_read_header called before handle_read_payload.
            // So now the payload is read in sync, which should still be fast as the payload always directly follows the header. 
            //
            // boost::asio::async_read(theSocket, 
            //         asio::buffer(dataPtr->incomingBuffer, payloadSize), 
            //         theStrand.wrap(boost::bind(&ClientConnection::handle_read_payload, 
            //                 this, asio::placeholders::error, asio::placeholders::bytes_transferred, dataPtr)));

            if (payloadSize != asio::read(theSocket, asio::buffer(dataPtr->incomingBuffer, payloadSize)))
            {
                LOG_ERROR("Unable to read " << payloadSize << " bytes from server. Giving up on message.")
            }
            else
            {
                LOG_DEBUG("Received reply from server for message " << dataPtr->theRequest << ", parsing it. Payload size: " << payloadSize); 

                bool result = dataMarshaller.unmarshalPayload(dataPtr->theReply, &dataPtr->incomingBuffer[0], payloadSize); 

                if (result)
                {
                    LOG_INFO("Successfully parsed response from server for message " << dataPtr->theRequest 
                            << ": " << *dataPtr->theReply);

                    MessageHandlerPtr handler = MessageHandlerFactory::getMessageHandler(dataPtr->theReply->type);

                    if (!handler)
                    {
                        LOG_WARN("Received unknown message type - ignoring.")
                    }
                    else 
                    {
                        MessagePtr reply;
                        MessageHandler::ConnectionCommand cmd = handler->handleReply(dataPtr->theRequest, dataPtr->theReply);

                        switch(cmd)
                        {
                            case MessageHandler::writeMessage:
                                {
                                    LOG_DEBUG("Handler told us to write the message"); 
                                    break;
                                }
                            case MessageHandler::readMessage:
                                {
                                    LOG_DEBUG("Handler told us to read a message"); 
                                    break;
                                }
                            case MessageHandler::closeConnection:
                                {
                                    LOG_DEBUG("Handler told us to close the connection."); 
                                    close();
                                    break;
                                }
                            case MessageHandler::stayConnected:
                                {
                                    LOG_DEBUG("Handler told us to just stay connected."); 
                                    break;
                                }
                        }

                    }
                }
                else if (!result)
                {
                    LOG_WARN("Unable to parse incoming server response for message " << dataPtr);
                }
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

// void ClientConnection::handle_read_payload(const boost::system::error_code &e, std::size_t bytes_transferred, const TransferDataPtr &dataPtr)
// {
//     TRACE_ENTER();
// 
//     if (!e)
//     {
//         LOG_DEBUG("Received reply from server for message " << dataPtr->theRequest << ", parsing it."); 
// 
//         bool result = dataMarshaller.unmarshalPayload(dataPtr->theReply, &dataPtr->incomingBuffer[0], bytes_transferred); 
// 
//         if (result)
//         {
//             LOG_DEBUG("Successfully parsed response from server for message " << dataPtr->theRequest 
//                     << ": " << *dataPtr->theReply);
//         }
//         else if (!result)
//         {
//             LOG_WARN("Unable to parse incoming server response for message " << dataPtr);
//         }
//         else
//         {
//             LOG_WARN("Did not get all the bytes for reponse to message " << dataPtr->theRequest 
//                     << " giving up on it even though I should try to get the rest of the message"); 
//         }
// 
//     }
//     else
//     {
//         LOG_WARN("Error reading response from server for message " << dataPtr->theRequest << ": " << e.message());
//         doClose(); 
//     }
// 
//     TRACE_EXIT();
// }


