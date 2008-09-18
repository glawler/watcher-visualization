#ifndef WATCHERD_CLIENT_CONECTION_HPP
#define WATCHERD_CLIENT_CONECTION_HPP

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

            bool sendMessage(const boost::shared_ptr<Message> message);
            
            boost::asio::ip::tcp::socket& getSocket();

        private:

            DECLARE_LOGGER();

            void startQuery();

            void handle_resolve(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
            void handle_connect(const boost::system::error_code& e, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
            void handle_write_message(const boost::system::error_code& e);
            void handle_read_response(const boost::system::error_code& e, std::size_t bytes_transferred);

            boost::asio::ip::tcp::resolver theResolver;
            boost::asio::ip::tcp::socket theSocket;

            boost::array<char, 8192> inBuffer;
            std::vector<boost::asio::const_buffer> outBuffers;

            boost::shared_ptr<Message> theRequest;
            boost::shared_ptr<Message> theResponse;
            DataMarshaller dataMarshaller;

            boost::asio::deadline_timer connectionTimeoutTimer;

            std::string server;
            std::string service;

            bool connected;
    };

    typedef boost::shared_ptr<ClientConnection> clientConnectionPtr;

} // namespace watcher

#endif // WATCHERD_CLIENT_CONECTION_HPP
