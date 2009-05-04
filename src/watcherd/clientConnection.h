#ifndef WATCHERD_CLIENT_CONECTION_HPP
#define WATCHERD_CLIENT_CONECTION_HPP

#include <list>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "dataMarshaller.h"
#include "connection.h"

#include "libwatcher/message.h"

namespace watcher 
{
    /// Represents a single connection from a client.
    class ClientConnection : 
        public Connection,
        public boost::enable_shared_from_this<ClientConnection>
    {
        public:
            // Connect to the service service on server server using the boost::io_service given.
            ClientConnection(
                    boost::asio::io_service& io_service, 
                    const std::string &server, 
                    const std::string &service);

            virtual ~ClientConnection(); 

            /**
             * sendMessage() will send a packet to the server which contains the message. 
             *
             * @param message - The message to send.
             * @return a bool - currently ignored. 
             */
            bool sendMessage(const event::MessagePtr message);
            bool sendMessages(const std::vector<event::MessagePtr> &message);

            /**
             * close()
             * close the connection to the server.
             */
            void close(); 
           
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

            void doWrite(const std::vector<event::MessagePtr> &message); 

            bool connected; 

            boost::asio::io_service &ioService;
            boost::asio::io_service::strand theStrand;

            typedef boost::array<char, 8192> IncomingBuffer;
            typedef struct 
            {
                IncomingBuffer incomingBuffer;
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
    };

    typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr;

} // namespace watcher

#endif // WATCHERD_CLIENT_CONECTION_HPP
