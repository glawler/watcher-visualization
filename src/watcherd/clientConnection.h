#ifndef WATCHERD_CLIENT_CONECTION_HPP
#define WATCHERD_CLIENT_CONECTION_HPP

#include <list>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include "dataMarshaller.h"

#include "libwatcher/message.h"

#include "messageHandler.h"
namespace watcher 
{
    /// Represents a single connection from a client.
    class ClientConnection : private boost::noncopyable
    {
        public:
            // Connect to the service service on server server using the boost::io_service given.
            ClientConnection(
                    boost::asio::io_service& io_service, 
                    const std::string &server, 
                    const std::string &service);

            virtual ~ClientConnection(); 

            /**
             * sendMessage() will send a message to the server.
             *
             * @param message - The message to send.
             * @return a bool - currently ignored. 
             */
            bool sendMessage(const event::MessagePtr message);


            /**
             * setMessageHandler() Set a messageHandler if you want direct access to the 
             * responses sent via sendMessage().
             * @param messageHandler - an instance of a message handler class. 
             */
            void setMessageHandler(MessageHandlerPtr messageHandler); 

            /**
             * close()
             * close the connection to the server.
             */
            void close(); 
           
            /**
             * getSocket()
             * @return socket - the underlying socket for this connection.
             */
            boost::asio::ip::tcp::socket& getSocket();

        private:

            DECLARE_LOGGER();

            void doClose();
            /** 
             * Connect to the server. This function will not return until connected.
             * Will loop until connected, trying every X seconds.
             */
            void doConnect(); 

            /**
             * Attempt connection to server. Will set 'connected' to true, if successful. Will
             * also return true on success.
             * @return true on success, false otherwise.
             */
            bool tryConnect(); 

            void doWrite(const event::MessagePtr &message); 

            bool connected; 

            boost::asio::ip::tcp::socket theSocket;
            boost::asio::io_service &ioService;
            boost::asio::io_service::strand theStrand;

            DataMarshaller::NetworkMarshalBuffers outBuffers;

            typedef boost::array<char, 8192> IncomingBuffer;
            typedef struct 
            {
                IncomingBuffer incomingBuffer;
                event::MessagePtr theReply;
                event::MessagePtr theRequest;
            } TransferData;

            typedef boost::shared_ptr<TransferData> TransferDataPtr;
            std::list<TransferDataPtr> transferData;

            // void handle_resolve(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
            // void handle_connect(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

            void handle_write_message(const boost::system::error_code& e, const TransferDataPtr &dataPtr);
            void handle_read_header(const boost::system::error_code& e, std::size_t bytes_transferred, 
                    const TransferDataPtr &dataPtr);
            void handle_read_payload(const boost::system::error_code& e, std::size_t bytes_transferred, 
                    const TransferDataPtr &dataPtr);

            std::string server;
            std::string service;

            MessageHandlerPtr messageHandler;
    };

    typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr;

} // namespace watcher

#endif // WATCHERD_CLIENT_CONECTION_HPP
