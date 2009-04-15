#include "serverConnection.h"
#include <vector>
#include <boost/bind.hpp>

// GTL REMOVE THIS
#include "libwatcher/labelMessage.h"

#include <libwatcher/message.h>
#include <libwatcher/messageStatus.h>

#include "messageFactory.h"
#include "dataMarshaller.h"

using namespace std; 

namespace watcher {
    using namespace event;

    INIT_LOGGER(ServerConnection, "Connection.ServerConnection");

    ServerConnection::ServerConnection(boost::asio::io_service& io_service) :
        Connection(io_service),
        strand_(io_service)
    {
        TRACE_ENTER(); 
        TRACE_EXIT();
    }

    void ServerConnection::start()
    {
        TRACE_ENTER(); 
        boost::asio::async_read(
                theSocket, 
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
                        theSocket, 
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
                boost::asio::ip::address nodeAddr(theSocket.remote_endpoint().address()); 

                LOG_INFO("Recvd " << arrivedMessages.size() << " message" << (arrivedMessages.size()>1?"s":"") << " from " << nodeAddr); 

                for(MessageHandlerList::iterator mh=messageHandlers.begin(); mh!=messageHandlers.end(); ++mh)
                {
                    // GTL THIS NEEDS TO GO ELSEWHERE - JUST TESTING MESSAGE STREAM ---------START----------------
                    for(vector<MessagePtr>::const_iterator i=arrivedMessages.begin(); i!=arrivedMessages.end(); ++i)
                    {
                        if((*i)->type==START_MESSAGE_TYPE)
                        {
                            vector<MessagePtr> bogusMessages;
                            bogusMessages.push_back(LabelMessagePtr(new LabelMessage("This is a test message 1")));
                            bogusMessages.push_back(LabelMessagePtr(new LabelMessage("This is a test message 2")));
                            bogusMessages.push_back(LabelMessagePtr(new LabelMessage("This is a test message 3")));

                            DataMarshaller::NetworkMarshalBuffers outBuffers;
                            DataMarshaller::marshalPayload(bogusMessages, outBuffers);
                            LOG_INFO("Sending bogus data back to startMessage sender."); 
                            boost::asio::async_write(theSocket, outBuffers,   strand_.wrap( boost::bind( &ServerConnection::handle_write, shared_from_this(), boost::asio::placeholders::error, MessagePtr())));
                        }
                    }
                    // GTL THIS NEEDS TO GO ELSEWHERE - JUST TESTING MESSAGE STREAM ---------END----------------
                    

                    if((*mh)->handleMessagesArrive(arrivedMessages))
                    {
                        LOG_DEBUG("Message handler told us to keep this connection open."); 
                        start();
                    }
                }
            }
        }
        else
        {
            LOG_WARN("Did not understand incoming message."); 
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
            LOG_DEBUG("Successfully sent message to client: " << message); 

            bool waitForResponse=false;
            for(MessageHandlerList::iterator mh=messageHandlers.begin(); mh!=messageHandlers.end(); ++mh)
            {
                if(waitForResponse) // someone already said they wanted a response, so ignore ret val for others
                    (*mh)->handleMessageSent(message);
                else
                    waitForResponse=(*mh)->handleMessageSent(message);
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
