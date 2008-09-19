#ifndef WATCHERD_CLIENT_CONECTION_HPP
#define WATCHERD_CLIENT_CONECTION_HPP

#include <list>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>

#include "message.h"
#include "dataMarshaller.hpp"

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

            // void handle_resolve(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
            // void handle_connect(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

            void handle_write_message(const boost::system::error_code& e, const MessagePtr messPtr);
            void handle_read_response(const boost::system::error_code& e, std::size_t bytes_transferred, const MessagePtr messPtr);

            boost::asio::ip::tcp::socket theSocket;
            boost::asio::io_service &ioService;

            typedef boost::array<char, 8192> IncomingBuffer;
            IncomingBuffer incomingBuffer;

            typedef boost::asio::const_buffer OutBuffer;
            typedef std::vector<OutBuffer> OutBuffers;
            OutBuffers outBuffers;

            // MessagePtr theRequest;
            MessagePtr theReply;

            std::list<MessagePtr> writeMessages;
            DataMarshaller dataMarshaller;

            std::string server;
            std::string service;
    };

    typedef boost::shared_ptr<ClientConnection> ClientConnectionPtr;

} // namespace watcher

#endif // WATCHERD_CLIENT_CONECTION_HPP
