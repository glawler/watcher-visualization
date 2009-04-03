/** 
 * @file watcherClientConnection.h
 * @author Geoff Lawler <geoff.lawler@sparta.com>
 * @date 2009-03-25
 */
#ifndef WATCHERD_CLIENT_CONECTION_WITH_CONNECTION_SPELD_RONG_HPP
#define WATCHERD_CLIENT_CONECTION_WITH_CONNECTION_SPELD_RONG_HPP

#include "clientConnection.h"

namespace watcher 
{
    /**
     * @class WatcherdClientMessageHandler
     * This is a small one abstract method class. The single 
     * method is messageArrived() which is invoked by the 
     * WatcherdClientConnection instance that this class is a 
     * part of. The top level class that wants to handle its 
     * own messages should derive from this class and pass a pointer
     * to itself into the WatcherdClientConnection instance that it
     * uses to connect to the server. 
     */
    class WatcherdClientMessageHandler
    {
        public:
            WatcherdClientMessageHandler() {}; 
            virtual ~WatcherdClientMessageHandler() {} ;
            virtual bool messageArrived(event::MessagePtr message)=0;
    };

    typedef boost::shared_ptr<WatcherdClientMessageHandler> WatcherdClientMessageHandlerPtr;

    /**
     * @class WatcherdClientConnection
     *
     * @brief This class encapsulates a network connection to a watcherd instance. It 
     * derives from the ABC clientConnection and implements the messageArrive function.
     */
    class WatcherdClientConnection : public ClientConnection
    {
        public:
            // Connect to the service service on server server using the boost::io_service given.
            explicit WatcherdClientConnection(
                    WatcherdClientMessageHandlerPtr messageHandler,
                    boost::asio::io_service& io_service, 
                    const std::string &server, 
                    const std::string &service);

            virtual ~WatcherdClientConnection(); 

            /** 
             * messageArrive() is call back invoked by the underlying network
             * code when a message arrives from the watcherd instance.
             *
             * @param message - the newly arrived message.
             * @return a bool - currently ignored. 
             */
            virtual bool messageArrive(const event::Message &message);

        private:

            DECLARE_LOGGER();

            WatcherdClientMessageHandlerPtr messageArrivalHandler;
    };

    typedef boost::shared_ptr<WatcherdClientConnection> WatcherdClientConnectionPtr;

} // namespace watcher

#endif //  WATCHERD_CLIENT_CONECTION_WITH_CONNECTION_SPELD_RONG_HPP

