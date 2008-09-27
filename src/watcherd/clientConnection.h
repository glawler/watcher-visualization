#ifndef WATCHERD_CLIENT_CONECTION_HPP
#define WATCHERD_CLIENT_CONECTION_HPP

#include <list>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#include "message.h"
#include "dataMarshaller.h"

namespace watcher 
{
    /// Represents a single connection from a client.
    class ClientConnection : private boost::noncopyable
    {
        public:
            // Connect to the service service on server server using the boost::io_service given.
            explicit ClientConnection(
                    boost::asio::io_service& io_service, 
                    const std::string &server, 
                    const std::string &service);

            bool sendMessage(const MessagePtr message);

            void close(); 
            
            boost::asio::ip::tcp::socket& getSocket();

        private:

            DECLARE_LOGGER();

            void doClose();
            void doConnect(); 
            void doWrite(const MessagePtr &message); 

            bool connected; 

            boost::asio::ip::tcp::socket theSocket;
            boost::asio::io_service &ioService;
            boost::asio::io_service::strand theStrand;

            typedef boost::asio::const_buffer OutBuffer;
            typedef std::vector<OutBuffer> OutBuffers;
            OutBuffers outBuffers;

            typedef boost::array<char, 8192> IncomingBuffer;
            typedef struct 
            {
                IncomingBuffer incomingBuffer;
                MessagePtr theReply;
                MessagePtr theRequest;
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

            // std::list<MessagePtr> writeMessages;
            DataMarshaller dataMarshaller;

            std::string server;
            std::string service;
    };

    typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr;

} // namespace watcher

#endif // WATCHERD_CLIENT_CONECTION_HPP
